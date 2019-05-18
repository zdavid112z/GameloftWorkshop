#include "stdafx.h"
#include "Model.h"
#include "Shader.h"
#include "ModelParser.h"
#include "ResourceManager.h"
#include "MeshFactory.h"

Model::Model(ModelResource* res)
{
	resource = new ModelResource(*res);
	ModelData md;
	isWireframe = false;
	switch (res->type)
	{
	case ModelType::LOADED:
		if (Utils::EndsWith(res->path, ".obj"))
			md = ParseObj(res->path);
		else md = ParseModel(res->path);
		break;
	case ModelType::CUBE:
		md = MeshFactory::GenerateCube(1);
		break;
	case ModelType::AXIS:
		md = MeshFactory::GenerateAxis(res->lineLength, res->headOffset, res->headLength, res->thickness, res->quality);
		break;
	case ModelType::SCREEN_QUAD:
		md = MeshFactory::GenerateScreenQuad();
		break;
	}
	this->numVert = md.numVertices;
	aabb.min = aabb.max = md.vertices[0].pos;
	for (uint i = 1; i < md.numVertices; i++)
	{
		aabb.min.x = std::min(aabb.min.x, md.vertices[i].pos.x);
		aabb.min.y = std::min(aabb.min.y, md.vertices[i].pos.y);
		aabb.min.z = std::min(aabb.min.z, md.vertices[i].pos.z);
		aabb.max.x = std::max(aabb.max.x, md.vertices[i].pos.x);
		aabb.max.y = std::max(aabb.max.y, md.vertices[i].pos.y);
		aabb.max.z = std::max(aabb.max.z, md.vertices[i].pos.z);
	}
	GenerateWireframe(md);
	GenerateNormals(md);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * md.numVertices, md.vertices, GL_STATIC_DRAW);
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * md.numIndices, md.indices, GL_STATIC_DRAW);
	count = md.numIndices;

	md.Delete();
}

Model::Model(ModelData md)
{
	this->numVert = md.numVertices;
	aabb.min = aabb.max = md.vertices[0].pos;
	for (uint i = 1; i < md.numVertices; i++)
	{
		aabb.min.x = std::min(aabb.min.x, md.vertices[i].pos.x);
		aabb.min.y = std::min(aabb.min.y, md.vertices[i].pos.y);
		aabb.min.z = std::min(aabb.min.z, md.vertices[i].pos.z);
		aabb.max.x = std::max(aabb.max.x, md.vertices[i].pos.x);
		aabb.max.y = std::max(aabb.max.y, md.vertices[i].pos.y);
		aabb.max.z = std::max(aabb.max.z, md.vertices[i].pos.z);
	}

	GenerateWireframe(md);
	GenerateNormals(md);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * md.numVertices, md.vertices, GL_STATIC_DRAW);
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * md.numIndices, md.indices, GL_STATIC_DRAW);
	count = md.numIndices;
}

void Model::GenerateWireframe(ModelData md)
{
	uint16* wireframeIndices = new uint16[md.numIndices * 2];
	for (uint i = 0; i < md.numIndices / 3; i++)
	{
		wireframeIndices[i * 6 + 0] = md.indices[i * 3 + 0];
		wireframeIndices[i * 6 + 1] = md.indices[i * 3 + 1];
		wireframeIndices[i * 6 + 2] = md.indices[i * 3 + 0];
		wireframeIndices[i * 6 + 3] = md.indices[i * 3 + 2];
		wireframeIndices[i * 6 + 4] = md.indices[i * 3 + 1];
		wireframeIndices[i * 6 + 5] = md.indices[i * 3 + 2];
	}
	glGenBuffers(1, &wiredIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wiredIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * md.numIndices * 2, wireframeIndices, GL_STATIC_DRAW);
	delete[] wireframeIndices;
}

Model::~Model()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &wiredIbo);
	glDeleteBuffers(1, &normalsIbo);
	glDeleteBuffers(1, &normalsVbo);
	if(resource)
		delete resource;
}

void Model::Draw(Shader* shader, bool wired)
{
	shader->Bind();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if(wired)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wiredIbo);
	else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	shader->BindCommonAttributes();
	if(wired)
		glDrawElements(GL_LINES, count * 2, GL_UNSIGNED_SHORT, 0);
	else
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);
}

void Model::GenerateNormals(ModelData md)
{
	Vertex* v = new Vertex[md.numVertices * 2];
	uint16* ind = new uint16[md.numVertices * 2];
	for (uint i = 0; i < md.numVertices; i++)
	{
		v[i * 2 + 0].pos = md.vertices[i].pos;
		v[i * 2 + 1].pos = md.vertices[i].pos + md.vertices[i].norm * 10.f;
		ind[i * 2 + 0] = i * 2 + 0;
		ind[i * 2 + 1] = i * 2 + 1;
	}
	glGenBuffers(1, &normalsVbo);
	glBindBuffer(GL_ARRAY_BUFFER, normalsVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * md.numVertices * 2, v, GL_STATIC_DRAW);
	glGenBuffers(1, &normalsIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normalsIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * md.numVertices * 2, ind, GL_STATIC_DRAW);
	delete[] v;
	delete[] ind;
}

void Model::DrawNormals(Shader* shader)
{
	shader->Bind();
	glBindBuffer(GL_ARRAY_BUFFER, normalsVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normalsIbo);
	shader->BindCommonAttributes();
	glDrawElements(GL_LINES, numVert * 2, GL_UNSIGNED_SHORT, 0);
}