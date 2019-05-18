#include "stdafx.h"
#include "MeshFactory.h"

ModelData MeshFactory::GenerateScreenQuad()
{
	Vertex* v = new Vertex[4];
	uint16* i = new uint16[6];
	v[0].pos = Vector3(-1, -1, 0);
	v[1].pos = Vector3( 1, -1, 0);
	v[2].pos = Vector3( 1,  1, 0);
	v[3].pos = Vector3(-1,  1, 0);
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;
	return ModelData(v, 4, i, 6);
}

ModelData MeshFactory::GenerateTerrain(int sizeX, int sizeY, float texMultiplier)
{
	uint numVertices = (1 + sizeX) * (1 + sizeY);
	uint numIndices = sizeX * sizeY * 6;
	Vertex* vert = new Vertex[numVertices];
	uint16* ind = new uint16[numIndices];
	for (int i = 0; i <= sizeY; i++)
	{
		for (int j = 0; j <= sizeX; j++)
		{
			Vertex& v = vert[i * (sizeX + 1) + j];
			v.pos = Vector3(j, 0, i);
			v.uv = Vector3((float)j / sizeX, (float)i / sizeY, 0);
		}
	}
	int k = 0;
	for (int i = 0; i < sizeY; i++)
	{
		for (int j = 0; j < sizeX; j++)
		{
			ind[k++] = (i) * (sizeX + 1) + j;
			ind[k++] = (i + 1) * (sizeX + 1) + j;
			ind[k++] = (i + 1) * (sizeX + 1) + j + 1;
			ind[k++] = (i) * (sizeX + 1) + j;
			ind[k++] = (i + 1) * (sizeX + 1) + j + 1;
			ind[k++] = (i) * (sizeX + 1) + j + 1;
		}
	}
	ModelData md(vert, numVertices, ind, numIndices);
	GenerateNormals(md);
	GenerateTangentsAndBitangents(md);
	return md;
}

void MeshFactory::GenerateNormals(ModelData md)
{
	for (uint32 i = 0; i < md.numVertices; i++)
		md.vertices[i].norm = Vector3(0, 0, 0);
	for (uint32 i = 0; i < md.numIndices; i += 3)
	{
		Vector3 ab = md.vertices[md.indices[i + 1]].pos - md.vertices[md.indices[i + 0]].pos;
		Vector3 ac = md.vertices[md.indices[i + 2]].pos - md.vertices[md.indices[i + 0]].pos;
		Vector3 p = ab.Cross(ac);
		md.vertices[md.indices[i + 0]].norm += p;
		md.vertices[md.indices[i + 1]].norm += p;
		md.vertices[md.indices[i + 2]].norm += p;
	}
	for (uint32 i = 0; i < md.numVertices; i++)
		md.vertices[i].norm.Normalize();
}

void MeshFactory::GenerateTangentsAndBitangents(ModelData md)
{
	for (uint32 i = 0; i < md.numVertices; i++)
		md.vertices[i].tgt = md.vertices[i].binorm = Vector3(0, 0, 0);
	for (uint32 i = 0; i < md.numIndices; i += 3)
	{
		int i1 = md.indices[i];
		int i2 = md.indices[i + 1];
		int i3 = md.indices[i + 2];

		const Vector3& v1 = md.vertices[i1].pos;
		const Vector3& v2 = md.vertices[i2].pos;
		const Vector3& v3 = md.vertices[i3].pos;

		const Vector3& w1 = md.vertices[i1].uv;
		const Vector3& w2 = md.vertices[i2].uv;
		const Vector3& w3 = md.vertices[i3].uv;

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		md.vertices[i1].tgt += sdir;
		md.vertices[i2].tgt += sdir;
		md.vertices[i3].tgt += sdir;

		md.vertices[i1].binorm += tdir;
		md.vertices[i2].binorm += tdir;
		md.vertices[i3].binorm += tdir;
	}

	for (uint32 i = 0; i < md.numVertices; i++)
	{
		const Vector3& n = md.vertices[i].norm;
		const Vector3& t = md.vertices[i].tgt;

		md.vertices[i].tgt = (t - n * n.Dot(t));
		float m = n.Cross(t).Dot(md.vertices[i].binorm) < 0 ? -1 : 1;
		md.vertices[i].binorm = n.Cross(t) * m;
	}
}

ModelData MeshFactory::GenerateSphere(float size, int hquality, int vquality)
{
	Vertex* v = new Vertex[hquality * (vquality * 2 + 1) + 2];
	uint16* ind = new uint16[hquality * (vquality * 2 - 1) * 6];
	ModelData md;
	return md;
}

ModelData MeshFactory::GenerateFire(int numBlades, float height)
{
	Vertex* v = new Vertex[numBlades * 4];
	uint16* ind = new uint16[numBlades * 6];
	for (int i = 0; i < numBlades; i++)
	{
		float angle = (float)i / numBlades * PI;
		float x = cosf(angle);
		float z = sinf(angle);
		v[i * 4 + 0].pos = Vector3(x, -height, z);
		v[i * 4 + 0].uv = Vector3(0, 0, 0);
		v[i * 4 + 1].pos = Vector3(-x, -height, -z);
		v[i * 4 + 1].uv = Vector3(1, 0, 0);
		v[i * 4 + 2].pos = Vector3(x, height, z);
		v[i * 4 + 2].uv = Vector3(0, 1, 0);
		v[i * 4 + 3].pos = Vector3(-x, height, -z);
		v[i * 4 + 3].uv = Vector3(1, 1, 0);
		float xr = cosf(angle + PI / 2.f);
		float zr = sinf(angle + PI / 2.f);
		for (int j = 0; j < 4; j++)
		{
			v[i * 4 + j].norm = Vector3(xr, 0, zr);
			v[i * 4 + j].tgt = Vector3(-x, 0, -z);
			v[i * 4 + j].binorm = Vector3(0, 1, 0);
		}
		ind[i * 6 + 0] = i * 4 + 0;
		ind[i * 6 + 1] = i * 4 + 1;
		ind[i * 6 + 2] = i * 4 + 2;
		ind[i * 6 + 3] = i * 4 + 1;
		ind[i * 6 + 4] = i * 4 + 2;
		ind[i * 6 + 5] = i * 4 + 3;
	}
	return ModelData(v, numBlades * 4, ind, numBlades * 6);
}

ModelData MeshFactory::GenerateCube(float size)
{
	ModelData md = GenerateSkyboxCube(size);
	for (int i = 0; i < md.numIndices; i += 3)
		std::swap(md.indices[i], md.indices[i + 1]);
	GenerateNormals(md);
	GenerateTangentsAndBitangents(md);
	return md;
}

ModelData MeshFactory::GenerateSkyboxCube(float size)
{
	Vertex* v = new Vertex[8];
	uint16* ind = new uint16[36];
	
	v[0].pos = Vector3(-size, -size, -size);
	v[1].pos = Vector3(size, -size, -size);
	v[2].pos = Vector3(size, -size, size);
	v[3].pos = Vector3(-size, -size, size);
	v[4].pos = Vector3(-size, size, -size);
	v[5].pos = Vector3(size, size, -size);
	v[6].pos = Vector3(size, size, size);
	v[7].pos = Vector3(-size, size, size);
	v[0].uv = Vector3(0, 0, 0);
	v[1].uv = Vector3(1, 0, 0);
	v[2].uv = Vector3(1, 1, 0);
	v[3].uv = Vector3(0, 1, 0);
	v[4].uv = Vector3(1, 1, 0);
	v[5].uv = Vector3(0, 1, 0);
	v[6].uv = Vector3(0, 0, 0);
	v[7].uv = Vector3(1, 0, 0);
	uint16 il[] = {
		0, 2, 1,
		0, 3, 2,
		0, 7, 3,
		0, 4, 7,
		0, 1, 5,
		0, 5, 4,
		6, 5, 1,
		6, 1, 2,
		6, 4, 5,
		6, 7, 4,
		6, 2, 3,
		6, 3, 7
	};
	memcpy(ind, il, sizeof(il));
	ModelData md(v, 8, ind, 36);
	GenerateNormals(md);
	GenerateTangentsAndBitangents(md);
	return md;
}

ModelData MeshFactory::GenerateWireframeCube(float size)
{
	Vertex* v = new Vertex[8];
	uint16* ind = new uint16[24];
	v[0].pos = Vector3(-1, -1, -1) * size;
	v[1].pos = Vector3(1, -1, -1) * size;
	v[2].pos = Vector3(1, -1, 1) * size;
	v[3].pos = Vector3(-1, -1, 1) * size;
	v[4].pos = Vector3(-1, 1, -1) * size;
	v[5].pos = Vector3(1, 1, -1) * size;
	v[6].pos = Vector3(1, 1, 1) * size;
	v[7].pos = Vector3(-1, 1, 1) * size;
	ind[0] = 0; ind[1] = 1;
	ind[2] = 1; ind[3] = 2;
	ind[4] = 2; ind[5] = 3;
	ind[6] = 3; ind[7] = 0;
	ind[8] = 4; ind[9] = 5;
	ind[10] = 5; ind[11] = 6;
	ind[12] = 6; ind[13] = 7;
	ind[14] = 7; ind[15] = 4;
	ind[16] = 0; ind[17] = 4;
	ind[18] = 1; ind[19] = 5;
	ind[20] = 2; ind[21] = 6;
	ind[22] = 3; ind[23] = 7;
	return ModelData(v, 8, ind, 24);
}

ModelData MeshFactory::GenerateArrow(float lineSize, float headOffset, float headSize, float thickness, int quality)
{
	uint numVertices = quality * 3 + 2;
	Vertex* vertices = new Vertex[quality * 3 + 2];
	Vector3 local;
	for (int j = 0; j < quality; j++)
	{
		float angle = j * 2.f * PI / quality;
		local.x = cosf(angle) * thickness;
		local.y = 0;
		local.z = sinf(angle) * thickness;
		vertices[j * 3 + 0].pos = local;
		local.y = lineSize;
		vertices[j * 3 + 1].pos = local;
		local.x *= headOffset;
		local.z *= headOffset;
		vertices[j * 3 + 2].pos = local;
	}
	vertices[quality * 3].pos = Vector3(0, lineSize + headSize, 0);
	vertices[quality * 3 + 1].pos = Vector3(0, 0, 0);
	uint16* indices = new uint16[quality * 18];
	uint numIndices = 0;
	for (int j = 0; j < quality; j++)
	{
		indices[numIndices++] = quality * 3 + 1;
		indices[numIndices++] = ((j + 0) % quality) * 3 + 0;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 0;

		indices[numIndices++] = ((j + 0) % quality) * 3 + 0;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 0;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 1;

		indices[numIndices++] = ((j + 0) % quality) * 3 + 0;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 1;
		indices[numIndices++] = ((j + 0) % quality) * 3 + 1;

		indices[numIndices++] = ((j + 0) % quality) * 3 + 1;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 1;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 2;

		indices[numIndices++] = ((j + 0) % quality) * 3 + 1;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 2;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 2;

		indices[numIndices++] = ((j + 0) % quality) * 3 + 2;
		indices[numIndices++] = ((j + 1) % quality) * 3 + 2;
		indices[numIndices++] = quality * 3;
	}
	return ModelData(vertices, numVertices, indices, numIndices);
}

ModelData MeshFactory::GenerateAxis(float lineSize, float headOffset, float headSize, float thickness, int quality)
{
	ModelData x, y, z;
	x = GenerateArrow(lineSize, headOffset, headSize, thickness, quality);
	y = GenerateArrow(lineSize, headOffset, headSize, thickness, quality);
	z = GenerateArrow(lineSize, headOffset, headSize, thickness, quality);
	Vertex* vertices = new Vertex[3 * x.numVertices];
	uint16* indices = new uint16[3 * x.numIndices];
	uint numVertices = 0;
	uint numIndices = 0;
	Matrix rx, ry, rz;
	rx.SetRotationZ(PI / 2.f);
	rz.SetRotationX(-PI / 2.f);
	for (int i = 0; i < x.numVertices; i++)
	{
		x.vertices[i].pos = Vector3(rx * Vector4(x.vertices[i].pos, 1));
		x.vertices[i].norm = Vector3(1, 0, 0);
		vertices[numVertices++] = x.vertices[i];
	}
	for (int i = 0; i < y.numVertices; i++)
	{
		y.vertices[i].norm = Vector3(0, 1, 0);
		vertices[numVertices++] = y.vertices[i];
	}
	for (int i = 0; i < z.numVertices; i++)
	{
		z.vertices[i].pos = Vector3(rz * Vector4(z.vertices[i].pos, 1));
		z.vertices[i].norm = Vector3(0, 0, 1);
		vertices[numVertices++] = z.vertices[i];
	}
	for (int i = 0; i < x.numIndices; i++)
		indices[numIndices++] = x.indices[i];
	for (int i = 0; i < y.numIndices; i++)
		indices[numIndices++] = y.indices[i] + x.numVertices;
	for (int i = 0; i < z.numIndices; i++)
		indices[numIndices++] = z.indices[i] + x.numVertices + y.numVertices;

	x.Delete();
	y.Delete();
	z.Delete();
	return ModelData(vertices, numVertices, indices, numIndices);
}
