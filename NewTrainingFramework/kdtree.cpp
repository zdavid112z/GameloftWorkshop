#include "kdtree.h"

void KDTree::Init(std::vector<triangle>& triangles, float epsilon, float threshold)
{
	this->epsilon = epsilon;
	this->threshold = threshold;
	root = BuildNode(triangles, 0);
}

void KDTree::Delete()
{

}

KDNode* KDTree::BuildNode(std::vector<triangle>& list, uint32 h)
{
	if (list.size() <= KD_TREE_MAX_TRIANGLES)
	{
		aabb area;
		area.min = Vector3(1e20f);
		area.max = Vector3(-1e20f);
		KDNode* node;
		node->isLeaf = true;
		for (uint32 i = 0; i < list.size(); i++)
		{
			node->tris[i] = list[i];
			for (uint32 p = 0; p < 3; p++)
			{
				for (uint32 d = 0; d < 3; d++)
				{
					area.min[d] = std::min(area.min[d], list[i].p[p][d]);
					area.max[d] = std::max(area.max[d], list[i].p[p][d]);
				}
			}
		}
		area.min -= Vector3(threshold);
		area.max += Vector3(threshold);
		for (uint32 i = list.size(); i < KD_TREE_MAX_TRIANGLES; i++)
			node->tris[i] = triangle(Vector3(0.f), Vector3(0.f), Vector3(0.f));
		node->area = area;
		return node;
	}
	aabb area;
	area.min= Vector3(1e20f);
	area.max = Vector3(-1e20f);
	for (const auto& t : list)
	{
		for (int p = 0; p < 3; p++)
		for (int d = 0; d < 3; d++)
		{
			area.min[d] = std::min(area.min[d], t.p[p][d]);
			area.max[d] = std::max(area.max[d], t.p[p][d]);
		}
	}
	area.min -= Vector3(threshold);
	area.max += Vector3(threshold);
	float dx = area.max.x - area.min.x;
	float dy = area.max.y - area.min.y;
	float dz = area.max.z - area.min.z;
	int bestd;
	if (dx >= dy && dx >= dz)
		bestd = 0;
	else if (dy >= dx && dy >= dz)
		bestd = 1;
	else bestd = 2;
	std::sort(list.begin(), list.end(), [=](const triangle& a, const triangle& b)->bool
	{
		Vector3 ga = (a.p1 + a.p2 + a.p3) / 3.f;
		Vector3 gb = (b.p1 + b.p2 + b.p3) / 3.f;
		return ga[bestd] < gb[bestd];
	});
	std::vector<triangle> leftList, rightList;
	leftList.insert(leftList.begin(), list.begin(), list.begin() + list.size() / 2);
	rightList.insert(rightList.begin(), list.begin() + list.size() / 2, list.end());

	KDNode n;
	n.area = area;
	n.isLeaf = false;
	n.nodel = BuildNode(leftList, h + 1);
	n.noder = BuildNode(rightList, h + 1);
	return allocator.Allocate(&n);
}

RaycastHitData KDTree::Raycast(const Vector3& start, const Vector3& direction)
{
	return RaycastNode(root, start, direction);
}

RaycastHitData KDTree::RaycastNode(uint32 node, const Vector3& start, const Vector3& direction)
{
	KDNode* n = allocator.Get(node);
	if (n->isLeaf)
	{
		RaycastHitData hit;
		hit.hit = false;
		hit.distance = 1e20f;
		for (uint32 i = 0; i < KD_TREE_MAX_TRIANGLES; i++)
		{
			Vector3 vertex0 = n->tris[i].p1;
			Vector3 vertex1 = n->tris[i].p2;
			Vector3 vertex2 = n->tris[i].p3;
			Vector3 edge1, edge2, h, s, q;
			float a, f, u, v;
			edge1 = vertex1 - vertex0;
			edge2 = vertex2 - vertex0;
			h = direction.Cross(edge2);
			a = edge1.Dot(h);
			if (a > -epsilon && a < epsilon)
				continue;
			f = 1.0f / a;
			s = start - vertex0;
			u = f * (s.Dot(h));
			if (u < 0.0f || u > 1.0f)
				continue;
			q = s.Cross(edge1);
			v = f * direction.Dot(q);
			if (v < 0.0f || u + v > 1.0f)
				continue;
			float t = f * edge2.Dot(q);
			if (t > epsilon)
			{
				if (hit.distance > t)
				{
					hit.point = start + direction * t;
					hit.distance = t;
					hit.hit = true;
					hit.triangle = n->tris[i];
				}
			}
		}
		return hit;
	}
	KDNode* l = allocator.Get(n->nodel);
	KDNode* r = allocator.Get(n->noder);
	float distl;
	bool hitl = Shapes::LineIntersectsAABB(l->area, start, direction, distl);
	float distr;
	bool hitr = Shapes::LineIntersectsAABB(r->area, start, direction, distr);
	if (hitl && hitr)
	{
		if (distl < distr)
		{
			RaycastHitData hitl = RaycastNode(n->nodel, start, direction);
			if (hitl.hit && hitl.distance < distr)
				return hitl;
			else if(!hitl.hit)
				return RaycastNode(n->noder, start, direction);
			else
			{
				RaycastHitData hitr = RaycastNode(n->noder, start, direction);
				if (!hitr.hit || hitr.distance > hitl.distance)
					return hitl;
				return hitr;
			}
		}
		else
		{
			RaycastHitData hitr = RaycastNode(n->noder, start, direction);
			if (hitr.hit && hitr.distance < distl)
				return hitr;
			else if (!hitr.hit)
				return RaycastNode(n->nodel, start, direction);
			else
			{
				RaycastHitData hitl = RaycastNode(n->nodel, start, direction);
				if (!hitl.hit || hitl.distance > hitr.distance)
					return hitr;
				return hitl;
			}
		}
	}
	else if (hitl)
		return RaycastNode(n->nodel, start, direction);
	else if(hitr)
		return RaycastNode(n->noder, start, direction);
	RaycastHitData hit;
	hit.hit = false;
	return hit;
}

std::vector<triangle> KDTree::GetTrianglesInside(const aabb& a)
{
	return GetTrianglesInside(root, a);
}

std::vector<triangle> KDTree::GetTrianglesInside(uint32 node, const aabb& a)
{
	KDNode* n = allocator.Get(node);
	if (n->isLeaf)
	{
		std::vector<triangle> result;
		for (uint32 i = 0; i < KD_TREE_MAX_TRIANGLES; i++)
		{
			aabb ta;
			ta.min = Vector3(1e20f);
			ta.max = Vector3(-1e20f);
			for (uint32 p = 0; p < 3; p++)
			{
				for (uint32 d = 0; d < 3; d++)
				{
					ta.min[d] = glm::min(ta.min[d], n->tris[i].p[p][d]);
					ta.max[d] = glm::max(ta.max[d], n->tris[i].p[p][d]);
				}
			}
			if (Shapes::AABBsIntersect(ta, a))
			{
				result.push_back(n->tris[i]);
			}
		}
		return result;
	}
	KDNode* l = allocator.Get(n->nodel);
	KDNode* r = allocator.Get(n->noder);
	std::vector<triangle> result;
	if (Shapes::AABBsIntersect(l->area, a))
		result = GetTrianglesInside(n->nodel, a);
	if (Shapes::AABBsIntersect(r->area, a))
	{
		std::vector<triangle> listr = GetTrianglesInside(n->noder, a);
		result.insert(result.begin(), listr.begin(), listr.end());
	}
	return result;
}

std::vector<std::pair<aabb, int>> KDTree::GetAABBs(int minDepth, int maxDepth)
{
	return GetAABBs(root, 0, minDepth, maxDepth);
}

std::vector<std::pair<aabb, int>> KDTree::GetAABBs(uint32 node, int depth, int minDepth, int maxDepth)
{
	std::vector<std::pair<aabb, int>> res;
	KDNode* n = allocator.Get(node);
	if (depth >= minDepth && depth <= maxDepth)
		res.push_back(std::make_pair(n->area, depth - minDepth));
	if (depth < maxDepth && !n->isLeaf)
	{
		auto vl = GetAABBs(n->nodel, depth + 1, minDepth, maxDepth);
		auto vr = GetAABBs(n->noder, depth + 1, minDepth, maxDepth);
		res.insert(res.end(), vl.begin(), vl.end());
		res.insert(res.end(), vr.begin(), vr.end());
	}
	return res;
}