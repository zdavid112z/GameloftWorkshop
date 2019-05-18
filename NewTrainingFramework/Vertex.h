#pragma once

#include "stdafx.h"
#include "../Utilities/Math.h"

struct Vertex
{
	Vector3 pos = Vector3(0, 0, 0);
	Vector3 uv = Vector3(0, 0, 0);
	Vector3 norm = Vector3(0, 0, 0);
	Vector3 binorm = Vector3(0, 0, 0);
	Vector3 tgt = Vector3(0, 0, 0);
};

struct ModelData
{
	ModelData() {}
	ModelData(Vertex* v, uint numv, uint16* i, uint numi) :
		vertices(v), numVertices(numv),
		indices(i), numIndices(numi) {}
	Vertex* vertices = nullptr;
	uint16* indices = nullptr;
	uint numVertices = 0, numIndices = 0;
	void Delete()
	{
		if (vertices)
		{
			delete[] vertices;
			vertices = nullptr;
		}
		if (indices)
		{
			delete[] indices;
			indices = nullptr;
		}
	}
};
