#include "stdafx.h"
#include "ModelParser.h"
#include "MeshFactory.h"
#include "utils.h"
#include <unordered_map>
#include <fstream>
#include <sstream>

bool ParseVariable(const std::string& line, const std::string& varName, float* values, int numComponents)
{
	char number[32];
	int pos = line.find(varName);
	pos += varName.size();
	while (line[pos] == ' ') pos++;
	if (line[pos] != ':') return false;
	pos++;
	while (line[pos] == ' ') pos++;
	if (line[pos] != '[') return false;
	pos++;
	for (int i = 0; i < numComponents; i++)
	{
		while (line[pos] == ' ') pos++;
		int end = pos + 1;
		while (isdigit(line[end]) || line[end] == '-' || line[end] == '.') end++;
		memcpy(number, line.c_str() + pos, end - pos);
		number[end - pos] = 0;
		values[i] = atof(number);
		pos = end;
		while (line[pos] == ' ') pos++;
		if (line[pos] != ',') return false;
		pos++;
	}
	return true;
}

ModelData ParseModel(const std::string& path)
{
	ModelData res;
	Vertex* vertices;
	int numVertices;
	uint16* indices;
	int numIndices;
	std::ifstream fin(path);
	if (!fin)
		return res;
	char line[1024];
	fin.getline(line, 1024);
	char* word = strtok(line, ": ");

	if (strcmp(word, "NrVertices") != 0)
		return res;
	word = strtok(0, ": ");
	numVertices = atoi(word);
	vertices = new Vertex[numVertices];
	for(int i = 0; i < numVertices; i++)
	{
		fin.getline(line, 1024);
		std::string sline = line;
		ParseVariable(sline, "pos", (float*)&vertices[i].pos, 3);
		ParseVariable(sline, "uv", (float*)&vertices[i].uv, 2);
		ParseVariable(sline, "norm", (float*)&vertices[i].norm, 3);
		ParseVariable(sline, "binorm", (float*)&vertices[i].binorm, 3);
		ParseVariable(sline, "tgt", (float*)&vertices[i].tgt, 3);
	}

	fin.getline(line, 1024);
	word = strtok(line, ": ");

	if (strcmp(word, "NrIndices") != 0)
		return res;
	word = strtok(0, ": ");
	numIndices = atoi(word);
	indices = new uint16[numIndices];
	for(int i = 0; i < numIndices / 3; i++)
	{
		fin.getline(line, 1024);
		word = strtok(line, ". \t,");
		word = strtok(0, ". \t,");
		indices[i * 3 + 0] = atoi(word);
		word = strtok(0, ". \t,");
		indices[i * 3 + 1] = atoi(word);
		word = strtok(0, ". \t,");
		indices[i * 3 + 2] = atoi(word);
	}
	res.vertices = vertices;
	res.indices = indices;
	res.numIndices = numIndices;
	res.numVertices = numVertices;
	return res;
}

static bool StartsWith(const char* str, const char* prefix)
{
	while (*prefix != NULL)
	{
		if (*str != *prefix)
			return false;
		str++;
		prefix++;
	}
	return true;
}

static float NextFloat(const char*& str)
{
	bool positive = true;
	float nr = 0;
	while (*str != '-' && *str != '.' && !isdigit(*str) && *str != NULL)
		str++;
	if (*str == '-')
	{
		positive = false;
		str++;
	}

	while (isdigit(*str))
	{
		nr = nr * 10 + *str - '0';
		str++;
	}
	if (*str == '.')
	{
		float pw = 0.1f;
		str++;
		while (isdigit(*str))
		{
			nr += pw * (*str - '0');
			pw /= 10.f;
			str++;
		}
	}
	return nr * (positive * 2 - 1);
}

static float NextUInt(const char*& str)
{
	int nr = 0;
	while (!isdigit(*str) && *str != NULL)
		str++;

	while (isdigit(*str))
	{
		nr = nr * 10 + *str - '0';
		str++;
	}
	return nr;
}

struct OBJVertexIds
{
	int v, t, n;

	bool operator==(const OBJVertexIds& o) const
	{
		return v == o.v && t == o.t && n == o.n;
	}
};

namespace std {
	template <> struct hash<OBJVertexIds>
	{
		size_t operator()(const OBJVertexIds& x) const
		{
			return (x.v * 13) ^ (x.t * 31) ^ (x.n * 7115);
		}
	};
}

ModelData ParseObj(const std::string& path)
{
	ModelData res;
	std::ifstream fin(path);
	if (!fin)
		return res;
	char line[4096];
	std::vector<Vector3> v, n;
	std::vector<Vector2> t;
	std::unordered_map<OBJVertexIds, uint16> vertIds;
	std::vector<Vertex> verts;
	std::vector<uint16> indices;
	while (!fin.eof())
	{
		fin.getline(line, 4096);
		const char* s = line;
		if (StartsWith(line, "v "))
		{
			Vector3 vv;
			vv.x = NextFloat(s);
			vv.y = NextFloat(s);
			vv.z = NextFloat(s);
			v.push_back(vv);
		}
		else if (StartsWith(line, "vt "))
		{
			Vector2 vv;
			vv.x = NextFloat(s);
			vv.y = NextFloat(s);
			t.push_back(vv);
		}
		else if (StartsWith(line, "vn "))
		{
			Vector3 vv;
			vv.x = NextFloat(s);
			vv.y = NextFloat(s);
			vv.z = NextFloat(s);
			n.push_back(vv);
		}
		else if (StartsWith(line, "f "))
		{
			for (int i = 0; i < 3; i++)
			{
				int iv, it, in;
				iv = NextUInt(s);
				it = NextUInt(s);
				in = NextUInt(s);
				iv--; it--; in--;
				OBJVertexIds ids;
				ids.v = iv;
				ids.t = it;
				ids.n = in;

				auto iter = vertIds.find(ids);
				if (iter == vertIds.end())
				{
					Vertex vv;
					vv.pos = v[iv];
					vv.uv = Vector3(t[it].x, t[it].y, 0);
					vv.norm = n[in];
					indices.push_back(verts.size());
					vertIds[ids] = verts.size();
					verts.push_back(vv);
				}
				else
				{
					indices.push_back(iter->second);
				}
			}
		}
	}

	res.vertices = new Vertex[verts.size()];
	memcpy(res.vertices, verts.data(), sizeof(Vertex) * verts.size());
	res.numVertices = verts.size();
	
	res.indices = new uint16[indices.size()];
	memcpy(res.indices, indices.data(), sizeof(uint16) * indices.size());
	res.numIndices = indices.size();

	MeshFactory::GenerateTangentsAndBitangents(res);
	return res;
}
