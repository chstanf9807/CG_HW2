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
GLuint ibo;

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	// -------------------------------------------------------
	// Add your initialization code here.
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

void TriangleMesh::LoadBuffer(bool load)
{
	// Create vertex buffer.
	if (!load) glGenBuffers(1, &vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(VertexPTN) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	// Create index buffer.
	if (!load) glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * vertexIndices.size(), &vertexIndices[0], GL_STATIC_DRAW); // sizeof(x) is get memory size
}

void TriangleMesh::DrawTriangles()
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)12);
	glBindBuffer(GL_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, GetNumTriangles() * 3, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

// get the face data for three point(三點成一面的資料)
void TriangleMesh::get_face_data(map<int, map<int, map<int, int> > >& v_table, vector<vector<int> > fd, vector <glm::vec3> v, vector <glm::vec2> vt, vector <glm::vec3> vn) {
	VertexPTN tmp;
	int indx = 0;
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
					vertexIndices.push_back(v_table[p][t][n]);
				}
				else
				{
					v_table[p][t][n] = indx;
					tmp.position = v[p];
					tmp.texcoord = vt[t];
					tmp.normal = vn[n];
					vertices.push_back(tmp);
					vertexIndices.push_back(indx);
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
				vertexIndices.push_back(indx);
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
			vertexIndices.push_back(indx);
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
	// Parse the OBJ file.
	ifstream file;
	string line;
	// ---------------------------------------------------------------------------
    // Add your implementation here (HW1 + read *.MTL).
	// Add your implementation.
	float max_x = 0.0, max_y = 0.0, max_z = 0.0,
		min_x = 0.0, min_y = 0.0, min_z = 0.0;
	bool firstPosion = true;

	// load file
	file.open(filePath, ios::in);
	if (!file.is_open()) {
		cout << "file is not open !" << endl;
		exit(0);
		return false;
	}

	vector <glm::vec3> v;
	vector <glm::vec2> vt;
	vector <glm::vec3> vn;
	vector<vector<int> > f;
	map<int, map<int, map<int, int> > > v_table;
	string mtllib_str;
	string mtl_filePath;
	vector <string> mtl_names;
	size_t mtl_group = -1;
	int mtl_count = 0;
	int row = 0, number = 0;
	while (getline(file, line)) {
		if (line.substr(0, 2) == "v ") // 'v' ==> read vertex position (頂點)
		{
			GLfloat x_v, y_v, z_v;
			istringstream s(line.substr(2));
			s >> x_v;
			s >> y_v;
			s >> z_v;
			glm::vec3 p = glm::vec3(x_v, y_v, z_v);
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
				}
				count++;
			}
		}
		else if (line.substr(0, 1) == "mtllib ")
		{
			mtllib_str = line.substr(7);
			mtl_filePath = filePath.substr(0, filePath.find_last_of("/\\") + 1) + mtllib_str;
		}
		else if (line.substr(0, 7) == "usemtl ")
		{
			mtl_count++;
			string mtl_name = line.substr(7);
			vector<string>::iterator it = find(mtl_names.begin(), mtl_names.end(), mtl_name);
			if (it == mtl_names.end())
			{
				subMeshes.emplace_back();
				mtl_group = subMeshes.size() - 1;

				mtl_names.emplace_back(mtl_name);
				PhongMaterial mtl;
				mtl.SetName(mtl_name);
				subMeshes[mtl_group].material = &mtl;
			}
			else
				mtl_group = distance(mtl_names.begin(), it);
		}
	}
	file.close();

	get_face_data(v_table, f, v, vt, vn);
	numVertices = vertices.size();
	int numVertexIndices = vertexIndices.size();
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
		//std::cout << "SubMesh " << i << " with material: " << g.material->GetName() << std::endl;
		std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3 << std::endl;
	}
	std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", " << objCenter.z << std::endl;
	std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x " << objExtent.z << std::endl;
}

