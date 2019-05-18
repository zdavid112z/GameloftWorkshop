#pragma once

#include "GLES2/gl2.h"
#include "Vertex.h"
#include "AABB.h"

class ModelResource;
class Shader;

class Model
{
public:
	Model(ModelResource* res);
	Model(ModelData md);
	~Model();

	void Draw(Shader* shader, bool wired = false);
	void DrawNormals(Shader* shader);
	AABB GetAABB() { return aabb; }
private:
	void GenerateWireframe(ModelData md);
	void GenerateNormals(ModelData md);

	AABB aabb;
	ModelResource* resource = nullptr;
	GLuint normalsVbo, normalsIbo;
	GLuint vbo, ibo, wiredIbo;
	int count;
	int numVert;
	bool isWireframe;
};