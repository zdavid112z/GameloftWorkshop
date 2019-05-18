#pragma once

#include "stdafx.h"
#include "Vertex.h"

class MeshFactory
{
public:
	static ModelData GenerateScreenQuad();
	static ModelData GenerateTerrain(int sizeX, int sizeY, float texMultiplier);
	static ModelData GenerateFire(int numBlades, float height);
	static ModelData GenerateCube(float size);
	static ModelData GenerateSkyboxCube(float size);
	// Untested
	static ModelData GenerateWireframeCube(float size);
	static ModelData GenerateSphere(float size, int hquality, int vquality);
	static ModelData GenerateArrow(float lineSize, float headOffset, float headSize, float thickness, int quality);
	static ModelData GenerateAxis(float lineSize, float headOffset, float headSize, float thickness, int quality);
	static void GenerateNormals(ModelData md);
	static void GenerateTangentsAndBitangents(ModelData md);
};
