#include "trianglemesh.h"
#include "trianglemesh.h"
#include <iostream>
#include<fstream>
#include<sstream>
#include<string>
#include <vector>
#include <algorithm>
#include <map>
using namespace std;
//using namespace glm;


// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	// -------------------------------------------------------
	// Add your initialization code here.
	vboId = 0;
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	objExtent = glm::vec3(0.0f, 0.0f, 0.0f);
	// -------------------------------------------------------
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	// -------------------------------------------------------
	// Add your release code here.
	vertices.clear();
	subMeshes.clear();
	// -------------------------------------------------------
}

// refind max(x,y,z) and min(x, y, z)
glm::vec3 TriangleMesh::refind_objExtent()
{
	glm::vec3 newObjExtent = glm::vec3(0.0f, 0.0f, 0.0f);
	float max_x = 0.0, max_y = 0.0, max_z = 0.0,
		min_x = 0.0, min_y = 0.0, min_z = 0.0;

	for (int i = 0; i < vertices.size(); i++)
	{
		float x = vertices[i].position[0], y = vertices[i].position[1], z = vertices[i].position[2];
		//cout << "x = " << vertices[i].position[0] << ", y = " << vertices[i].position[1] << ", z = " << vertices[i].position[2] << endl;
		if (x < min_x) min_x = x;
		if (y < min_y) min_y = y;
		if (z < min_z) min_z = z;
		if (x > max_x) max_x = x;
		if (y > max_y) max_y = y;
		if (z > max_z) max_z = z;
	}
	newObjExtent = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
	return newObjExtent;
}

void TriangleMesh::delete_model()
{
	vertices.clear();
	subMeshes[0].vertexIndices.clear();
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	objExtent = glm::vec3(0.0f, 0.0f, 0.0f);
}

void TriangleMesh::LoadBuffer()
{
	// Create vertex buffer.
	glGenBuffers(2, &vboId); // 配合AttribPointer分配2個
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId);
	vector<VertexPTN> _vertices = getVertices();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(VertexPTN) * _vertices.size(), &_vertices[0], GL_STATIC_DRAW);
	// Create index buffer.
	//if (!load) glGenBuffers(1, &ibo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * vertexIndices.size(), &vertexIndices[0], GL_STATIC_DRAW);

	//for (int i = 0; i < vertices.size(); i++)
	//{
	//	cout << "vertices(x, y, z) = (" << vertices[i].position[0] << ", " << vertices[i].position[1] << ", " << vertices[i].position[2] << ")\n";
	//}
	//
	//cout << "vertexindices = ";
	//for (int i = 0; i < vertexIndices.size(); i++)
	//{
	//	cout << vertexIndices[i] << " ";
	//}
	//cout << endl;
	for (int i = 0; i < subMeshes.size(); i++)
	{
		glGenBuffers(1, &subMeshes[i].iboId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMeshes[i].iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * subMeshes[i].vertexIndices.size(), &subMeshes[i].vertexIndices[0], GL_STATIC_DRAW); // sizeof(x) is get memory size
	}
}

void TriangleMesh::DrawTriangles(PhongShadingDemoShaderProg* phongShadingShader)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)12);
	glEnableVertexAttribArray(1);
	//glBindBuffer(GL_ARRAY_BUFFER, ibo);
	//glDrawElements(GL_TRIANGLES, vertexIndices.size(), GL_UNSIGNED_INT, 0);
	for (int i = 0; i < subMeshes.size(); i++)
	{
		//float ns = MeshGetNs(i);
		//glm::vec3 ka = MeshGetKa(i);
		//glm::vec3 kd = MeshGetKd(i);
		//glm::vec3 ks = MeshGetKs(i);
		//cout << subMeshes[i].material->GetName() << ":\n";
		//cout << "Ns = " << ns << endl;
		//cout << "Ka = " << ka[0] << " " << ka[1] << " " << ka[2] << endl;
		//cout << "Kd = " << kd[0] << " " << kd[1] << " " << kd[2] << endl;
		//cout << "Ks = " << ks[0] << " " << ks[1] << " " << ks[2] << endl;

		// Material properties.
		glUniform3fv(phongShadingShader->GetLocKa(), 1, glm::value_ptr(MeshGetKa(i)));
		glUniform3fv(phongShadingShader->GetLocKd(), 1, glm::value_ptr(MeshGetKd(i)));
		glUniform3fv(phongShadingShader->GetLocKs(), 1, glm::value_ptr(MeshGetKs(i)));
		glUniform1f(phongShadingShader->GetLocNs(), MeshGetNs(i));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMeshes[i].iboId);
		glDrawElements(GL_TRIANGLES, subMeshes[i].vertexIndices.size(), GL_UNSIGNED_INT, 0);
	}
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// Apply transform to vertices using GPU.
void TriangleMesh::ApplyTransformCPU(const glm::mat4x4& mvpMatrix)
{
	for (int i = 0; i < numVertices; ++i) {
		glm::vec4 p = mvpMatrix * glm::vec4(vertices[i].position, 1.0f);
		if (p.w != 0.0f) {
			float inv = 1.0f / p.w;
			vertices[i].position.x = p.x * inv;
			vertices[i].position.y = p.y * inv;
			vertices[i].position.z = p.z * inv;
		}
	}
}

// get the material data for submesh
void TriangleMesh::get_material_data(map<string, int> mtl_table, vector <string>_mtl_names, vector <float> _Ns, vector <glm::vec3> _Ka, vector <glm::vec3> _Kd, vector <glm::vec3> _Ks)
{
	for (int i = 0; i < _mtl_names.size(); i++)
	{
		mtl_table[_mtl_names[i]] = i;
	}


	vector <string>::iterator name;
	for (int i = 0; i < subMeshes.size(); i++)
	{
		name = find(_mtl_names.begin(), _mtl_names.end(), subMeshes[i].material->GetName());
		if (name != _mtl_names.end())
		{
			subMeshes[i].material->SetNs(_Ns[mtl_table[*name]]);
			subMeshes[i].material->SetKa(_Ka[mtl_table[*name]]);
			subMeshes[i].material->SetKd(_Kd[mtl_table[*name]]);
			subMeshes[i].material->SetKs(_Ks[mtl_table[*name]]);
		}

		//float ns = subMeshes[i].material->GetNs();
		//glm::vec3 ka = subMeshes[i].material->GetKa();
		//glm::vec3 kd = subMeshes[i].material->GetKd();
		//glm::vec3 ks = subMeshes[i].material->GetKs();
		//
		//cout << subMeshes[i].material->GetName() << ":\n";
		//cout << "Ns = " << ns << endl;
		//cout << "Ka = " << ka[0] << " " << ka[1] << " " << ka[2] << endl;
		//cout << "Kd = " << kd[0] << " " << kd[1] << " " << kd[2] << endl;
		//cout << "Ks = " << ks[0] << " " << ks[1] << " " << ks[2] << endl;
	}
}

// get the face data for vertex buffer and index buffer(三點成一面的資料)
void TriangleMesh::get_face_data(map<int, map<int, map<int, int> > >& v_table, vector<vector<int> > fd, vector <glm::vec3> v, vector <glm::vec2> vt, vector <glm::vec3> vn, vector <int> mtl_group_indx, vector <int> mtl_fd) {
	VertexPTN tmp;
	int indx = 0, j = 0;
	int p = 0, t = 0, n = 0;
	map<int, map<int, map<int, int> > >::iterator it_p;
	map<int, map<int, int> >::iterator it_t;
	map<int, int>::iterator it_n;

	for (int i = 0; i < fd.size(); i++)
	{
		p = fd[i][0];
		t = fd[i][1];
		n = fd[i][2];
		it_p = v_table.find(p);
		if (it_p != v_table.end())
		{
			it_t = v_table[p].find(t);
			if (it_t != v_table[p].end())
			{
				it_n = v_table[p][t].find(n);
				if (it_n != v_table[p][t].end())
				{
					if (i < mtl_fd[j]*3)
						subMeshes[mtl_group_indx[j]].vertexIndices.push_back(v_table[p][t][n]);
					else
					{
						j++;
						subMeshes[mtl_group_indx[j]].vertexIndices.push_back(v_table[p][t][n]);
					}
					//vertexIndices.push_back(v_table[p][t][n]);
				}
				else
				{
					v_table[p][t][n] = indx;
					tmp.position = v[p];
					tmp.texcoord = vt[t];
					tmp.normal = vn[n];
					vertices.push_back(tmp);
					if (i < mtl_fd[j]*3)
						subMeshes[mtl_group_indx[j]].vertexIndices.push_back(indx);
					else
					{
						j++;
						subMeshes[mtl_group_indx[j]].vertexIndices.push_back(indx);
					}
					//vertexIndices.push_back(indx);
					indx++;
				}
			}
			else
			{
				map<int, int>  ptn_t;
				v_table[p][t] = ptn_t; //沒有t，給這個p加一個key為t，value為空map的node
				v_table[p][t][n] = indx; //沒有n，給這個t加一個key為n，value為indx的node
				tmp.position = v[p];
				tmp.texcoord = vt[t];
				tmp.normal = vn[n];
				vertices.push_back(tmp);
				if (i < mtl_fd[j]*3)
					subMeshes[mtl_group_indx[j]].vertexIndices.push_back(indx);
				else
				{
					j++;
					subMeshes[mtl_group_indx[j]].vertexIndices.push_back(indx);
				}
				//vertexIndices.push_back(indx);
				indx++;
			}
		}
		else
		{
			map<int, map<int, int> > ptn_p;
			map<int, int>  ptn_t;
			v_table[p] = ptn_p;
			v_table[p][t] = ptn_t;
			v_table[p][t][n] = indx;
			tmp.position = v[p];
			tmp.texcoord = vt[t];
			tmp.normal = vn[n];
			vertices.push_back(tmp);
			if (i < mtl_fd[j]*3)
				subMeshes[mtl_group_indx[j]].vertexIndices.push_back(indx);
			else
			{
				j++;
				subMeshes[mtl_group_indx[j]].vertexIndices.push_back(indx);
			}
			//vertexIndices.push_back(indx);
			indx++;
		}
	}
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{	
	vertices.clear();
	subMeshes.clear();
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	objExtent = glm::vec3(0.0f, 0.0f, 0.0f);
	
	// ---------------------------------------------------------------------------
    // Add your implementation here (HW1 + read *.MTL).
	// Add your implementation.
	
	// Parse the OBJ file.
	ifstream OBJ_file;
	string line;

	// Parse the MTL file.
	ifstream MTL_file;

	// Calculate objCenter and objExtent value
	float max_x = 0.0, max_y = 0.0, max_z = 0.0,
		min_x = 0.0, min_y = 0.0, min_z = 0.0;
	bool firstPosion = true;

	// load obj file
	OBJ_file.open(filePath, ios::in);
	if (!OBJ_file.is_open()) {
		cout << "file is not open !" << endl;
		exit(0);
		return false;
	}

	// Save face data value
	vector <glm::vec3> v;
	vector <glm::vec2> vt;
	vector <glm::vec3> vn;
	vector<vector<int> > f;
	map<int, map<int, map<int, int> > > v_table;
	int num_triangle = 0;

	// Save mtl value
	bool isnewmtl = false;
	string mtllib_str;
	string mtl_filePath;
	vector <string> mtl_names;
	int mtl_group = -1;
	vector <int> mtl_group_indx;
	vector <int> mtl_fd;

	// read obj file
	while (getline(OBJ_file, line)) {
		if (line.substr(0, 2) == "v ") // 'v' ==> read vertex position (頂點)
		{
			GLfloat x_v, y_v, z_v;
			istringstream s(line.substr(2));
			s >> x_v;
			s >> y_v;
			s >> z_v;
			glm::vec3 p = glm::vec3(x_v, y_v, z_v);
			//cout << "p = (" << p[0] << ", " << p[1] << ", " << p[2] << ")\n";
			v.push_back(p);
			if (firstPosion)
			{
				max_x = x_v, max_y = y_v, max_z = z_v;
				min_x = x_v, min_y = y_v, min_z = z_v;
				firstPosion = false;
			}
			else
			{
				if (x_v < min_x) min_x = x_v;
				if (y_v < min_y) min_y = y_v;
				if (z_v < min_z) min_z = z_v;
				if (x_v > max_x) max_x = x_v;
				if (y_v > max_y) max_y = y_v;
				if (z_v > max_z) max_z = z_v;
			}
		}
		else if (line.substr(0, 2) == "vt")
		{
			GLfloat t1, t2;
			istringstream s(line.substr(2));
			s >> t1;
			s >> t2;
			glm::vec2 t = glm::vec2(t1, t2);
			vt.push_back(t);
		}
		else if (line.substr(0, 2) == "vn")
		{
			GLfloat x_n, y_n, z_n;
			istringstream s(line.substr(2));
			s >> x_n;
			s >> y_n;
			s >> z_n;
			glm::vec3 n = glm::vec3(x_n, y_n, z_n);
			vn.push_back(n);
		}
		else if (line.substr(0, 1) == "f") // 'f' ==> read triangle's 3 vertex (哪幾個頂點構成一個面)
		{
			istringstream s(line.substr(2));
			string fd;
			int count = 0;
			vector <vector<int> > f_indx;
			while (s >> fd) // f <-- "p/t/n"
			{
				replace(fd.begin(), fd.end(), '/', ' ');
				istringstream s_fd(fd);
				string str;
				vector<int> ptn;
				int p = 0, t = 0, n = 0;
				for (int i = 0; i < 3; i++)
				{
					s_fd >> str;
					ptn.push_back(abs(stoi(str)) - 1);
				}
				if (count < 3)
				{
					f.push_back(ptn);
					f_indx.push_back(ptn);
				}
				else
				{
					f.push_back(f_indx[0]);
					f.push_back(f_indx[count - 1]);
					f.push_back(ptn);
					f_indx.push_back(ptn);
					num_triangle++;
				}
				count++;
			}
			num_triangle++;
			isnewmtl = true;
		}
		else if (line.substr(0, 7) == "mtllib ")
		{
			mtllib_str = line.substr(7);
			mtl_filePath = filePath.substr(0, filePath.find_last_of("/\\") + 1) + mtllib_str;
		}
		else if (line.substr(0, 7) == "usemtl ")
		{
			if (isnewmtl)
			{
				mtl_fd.push_back(num_triangle);
				isnewmtl = false;
			}

			string mtl_name = line.substr(7);
			vector<string>::iterator it = find(mtl_names.begin(), mtl_names.end(), mtl_name);
			if (it == mtl_names.end())
			{
				subMeshes.emplace_back(); // names跟submeshes同步
				mtl_group = subMeshes.size() - 1; // 若沒找到新入的name，就新增到最尾端
				mtl_group_indx.push_back(mtl_group); // 紀錄mtl順序
				mtl_names.emplace_back(mtl_name); // names跟submeshes同步
				subMeshes[mtl_group].material = new PhongMaterial(); // 直接在.material指標上配置新的PhongMaterial
				subMeshes[mtl_group].material->SetName(mtl_name);
			}
			else
			{
				mtl_group = distance(mtl_names.begin(), it);
				mtl_group_indx.push_back(mtl_group);
			}
		}
	}
	mtl_fd.push_back(num_triangle);
	OBJ_file.close();
	line.clear();

	//cout << "mtl_names: \n";
	//for (int i = 0; i < mtl_names.size(); i++)
	//{
	//	cout << mtl_names[i] << " ";
	//}
	//cout << endl;

	// Save mtl file data
	vector <string>_mtl_names;
	vector <float> _Ns;
	vector <glm::vec3> _Ka;
	vector <glm::vec3> _Kd;
	vector <glm::vec3> _Ks;
	map<string, int> mtl_table;

	// load mtl file
	MTL_file.open(mtl_filePath, ios::in);
	if (!MTL_file.is_open()) {
		cout << "file is not open !" << endl;
		exit(0);
		return false;
	}
	while (getline(MTL_file, line))
	{
		istringstream s(line);
		string str;
		s >> str;
		if (str == "newmtl")
		{
			s >> str;
			_mtl_names.push_back(str);
		}
		else if (str == "Ns")
		{
			GLfloat ns;
			s >> ns;
			_Ns.push_back(ns);
		}
		else if (str == "Ka")
		{
			GLfloat x, y, z;
			s >> x;
			s >> y;
			s >> z;
			glm::vec3 ka = glm::vec3(x, y, z);
			_Ka.push_back(ka);
		}
		else if (str == "Kd")
		{
			GLfloat x, y, z;
			s >> x;
			s >> y;
			s >> z;
			glm::vec3 kd = glm::vec3(x, y, z);
			_Kd.push_back(kd);
		}
		else if (str == "Ks")
		{
			GLfloat x, y, z;
			s >> x;
			s >> y;
			s >> z;
			glm::vec3 ks = glm::vec3(x, y, z);
			_Ks.push_back(ks);
		}
	}
	MTL_file.close();
	line.clear();
	
	//for (int i = 0; i < _mtl_names.size(); i++)
	//{
	//	cout << _mtl_names[i] << ":\n";
	//	cout << "Ns: " << _Ns[i] << endl;
	//	cout << "Ka: " << _Ka[i][0] << " " << _Ka[i][1] << " " << _Ka[i][2] << endl;
	//	cout << "Kd: " << _Kd[i][0] << " " << _Kd[i][1] << " " << _Kd[i][2] << endl;
	//	cout << "Ks: " << _Ks[i][0] << " " << _Ks[i][1] << " " << _Ks[i][2] << endl;
	//}

	get_material_data(mtl_table, _mtl_names, _Ns, _Ka, _Kd, _Ks);

	//for (int i = 0; i < v.size(); i++)
	//{
	//	cout << "v(x, y, z) = (" << v[i][0] << ", " << v[i][1] << ", " << v[i][2] << ")\n";
	//}
	
	get_face_data(v_table, f, v, vt, vn, mtl_group_indx, mtl_fd);

	//for (int i = 0; i < subMeshes.size(); i++)
	//{
	//	cout << "Material: " << subMeshes[i].material << endl;
	//	cout << "Indx_Buffer(size = " << subMeshes[i].vertexIndices.size() << "): ";
	//
	//	for (int j = 0; j < subMeshes[i].vertexIndices.size(); j++)
	//	{
	//		cout << subMeshes[i].vertexIndices[j] << " ";
	//	}
	//	cout << endl;
	//}
	//cout << "vertexIndices[i] = ";
	//for (int i = 0; i < vertexIndices.size(); i++)
	//{
	//	cout << vertexIndices[i] << " ";
	//}
	//cout << endl;
	//for (int i = 0; i < vertices.size(); i++)
	//{
	//	cout << "vertices(x, y, z) = (" << vertices[i].position[0] << ", " << vertices[i].position[1] << ", " << vertices[i].position[2] << ")\n";
	//	cout << "vertices(t1, t2) = (" << vertices[i].texcoord[0] << ", " << vertices[i].texcoord[1] << ")\n";
	//	cout << "vertices(nx, ny, nz) = (" << vertices[i].normal[0] << ", " << vertices[i].normal[1] << ", " << vertices[i].normal[2] << ")\n";
	//}
	//cout << endl;
	
	//for (int i = 0; i < subMeshes.size(); i++)
	//{
	//	for (int j = 0; j < subMeshes[i].vertexIndices.size(); j++)
	//		cout << subMeshes[i].vertexIndices[j] << " ";
	//	cout << endl;
	//}

	numVertices = vertices.size();
	int numVertexIndices = 0;
	//numVertexIndices = vertexIndices.size();
	for (int i = 0; i < subMeshes.size(); i++)
	{
		numVertexIndices += subMeshes[i].vertexIndices.size();
	}
	numTriangles = numVertexIndices / 3;


	objCenter = glm::vec3((max_x + min_x) / 2.0, (max_y + min_y) / 2.0, (max_z + min_z) / 2.0);
	objExtent = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
    // ---------------------------------------------------------------------------

	// Normalize the geometry data.
	if (normalized) {
		// -----------------------------------------------------------------------
		// Add your normalization code here (HW1).
		float max_edge = 0.0;
		max_edge = objExtent.x;
		if (max_edge < objExtent.y) max_edge = objExtent.y;
		if (max_edge < objExtent.z) max_edge = objExtent.z;

		for (int i = 0; i < vertices.size(); i++)
		{
			glm::vec3 tmp = vertices[i].position;
			// move to center
			vertices[i].position = glm::vec3(tmp.x - objCenter.x, tmp.y - objCenter.y, tmp.z - objCenter.z);

			tmp = vertices[i].position;
			// normalize size
			vertices[i].position = glm::vec3(tmp.x / max_edge, tmp.y / max_edge, tmp.z / max_edge);
		}
		objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
		objExtent = refind_objExtent();
		// -----------------------------------------------------------------------
	}
	return true;
}

// Show model information.
void TriangleMesh::ShowInfo()
{
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Total " << subMeshes.size() << " subMeshes loaded" << std::endl;
	for (unsigned int i = 0; i < subMeshes.size(); ++i) {
		const SubMesh& g = subMeshes[i];
		std::cout << "SubMesh " << i << " with material: " << g.material->GetName() << std::endl;
		std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3 << std::endl;
	}
	std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", " << objCenter.z << std::endl;
	std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x " << objExtent.z << std::endl;
}

