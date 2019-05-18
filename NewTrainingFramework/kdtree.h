#pragma once

#include "stdafx.h"
#include "shapes.h"
#include "../Utilities/utilities.h"
#define KD_TREE_MAX_TRIANGLES 6

struct RaycastHitData
{
	bool hit;
	float distance;
	Vector3 point;
	triangle triangle;
};

struct KDNode
{
	KDNode() {}
	bool isLeaf;
	aabb area;
	union
	{
		struct
		{
			KDNode* noder, *nodel;
		};
		triangle tris[KD_TREE_MAX_TRIANGLES];
	};
};

// not actually a kd tree, more like aabb tree
class KDTree
{
public:
	KDTree() {}
	~KDTree() {}
	void Init(std::vector<triangle>& triangles, float epsilon = 1e-6f, float threshold = 0.1f);
	void Delete();
	RaycastHitData Raycast(const Vector3& start, const Vector3& direction);
	std::vector<triangle> GetTrianglesInside(const aabb&);
	std::vector<std::pair<aabb, int>> GetAABBs(int minDepth, int maxDepth);
private:
	KDNode* BuildNode(std::vector<triangle>& list, uint32 h);
	RaycastHitData RaycastNode(uint32 node, const Vector3& start, const Vector3& direction);
	std::vector<triangle> GetTrianglesInside(uint32 node, const aabb& a);
	std::vector<std::pair<aabb, int>> GetAABBs(uint32 node, int depth, int minDepth, int maxDepth);
private:
	float threshold;
	float epsilon;
	uint32 root;
};

