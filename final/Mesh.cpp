#include "Mesh.h"

Mesh::Mesh()
{
}


Mesh::~Mesh()
{
}

/*
Blender Export options: Triangulated faces, Include UVs, Write Normals
*/
void Mesh::loadMesh(char* path) {
	std::vector<Vector3f> tempVertices, tempNormals;
	std::vector<Vector2f> tempUV;
	std::ifstream ifs(path);
	std::string line;

	while (std::getline(ifs, line)) {
		std::stringstream stream(line);

		std::string item;
		std::getline(stream, item, ' ');

		if (item.compare("v") == 0) { // Vertices
			float temp[3];
			for (int i = 0; i < 3; i++) {
				std::getline(stream, item, ' ');
				temp[i] = std::stof(item);
			}
			tempVertices.push_back(Vector3f(temp[0], temp[1], temp[2]));
		}
		else if (item.compare("vn") == 0) { // Normals
			float temp[3];
			for (int i = 0; i < 3; i++) {
				std::getline(stream, item, ' ');
				temp[i] = std::stof(item);
			}
			tempNormals.push_back(Vector3f(temp[0], temp[1], temp[2]));
		}
		else if (item.compare("vt") == 0) { // Texture coords
			float temp[2];
			for (int i = 0; i < 2; i++) {
				std::getline(stream, item, ' ');
				temp[i] = std::stof(item);
			}
			tempUV.push_back(Vector2f(temp[0], temp[1]));
		}
		else if (item.compare("f") == 0) { // Faces
			Face3f tempFace;
			int a = 0;
			while (std::getline(stream, item, ' ')) {
				std::stringstream s(item);

				int b = 0;
				unsigned int temp[3];
				for (std::string i; std::getline(s, i, '/'); b++) {
					temp[b] = std::stoi(i);
				}

				tempFace.vertices[a] = tempVertices.at(temp[0] - 1);
				tempFace.uv[a] = tempUV.at(temp[1] - 1);
				tempFace.normals[a] = tempNormals.at(temp[2] - 1);
				a++;
			}

			faces.push_back(tempFace);
		}
	}
}

void Mesh::Draw() {
	glBegin(GL_TRIANGLES);
		std::vector<Face3f>::iterator it;
		for (it = faces.begin(); it != faces.end(); it++) {
			for (int i = 0; i < 3; i++) {
				glNormal3f((*it).normals[i].v[0], (*it).normals[i].v[1], (*it).normals[i].v[2]);
				glTexCoord2f((*it).uv[i].v[0], (*it).uv[i].v[1]);
				glVertex3f((*it).vertices[i].v[0], (*it).vertices[i].v[1], (*it).vertices[i].v[2]);
			}
		}
	glEnd();
}