#pragma once

#include "../shared/gltools.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

typedef struct Vector3f {
	float v[3];
	Vector3f() {}
	Vector3f(float v1, float v2, float v3) {
		v[0] = v1;
		v[1] = v2;
		v[2] = v3;
	}
} Vector3f;

typedef struct Vector2f {
	float v[2];
	Vector2f() {}
	Vector2f(float v1, float v2) {
		v[0] = v1;
		v[1] = v2;
	}
} Vector2f;

typedef struct Face3f {
	Face3f() {}
	Vector3f vertices[3];
	Vector2f uv[3];
	Vector3f normals[3];
} Face3f;

class Mesh
{
private:
	std::vector<Face3f> faces;

public:
	Mesh();
	~Mesh();

	void Draw();
	void loadMesh(char* path);
};

