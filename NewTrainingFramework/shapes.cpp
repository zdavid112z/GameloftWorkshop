#include "stdafx.h"
#include "shapes.h"

plane::plane(const triangle& t)
{
	normal = (t.p2 - t.p1).Cross(t.p3 - t.p1).Normalize();
	origin = t.p1;
	ec[0] = normal.x;
	ec[1] = normal.y;
	ec[2] = normal.z;
	ec[3] = -(normal.x * t.p1.x + normal.y * t.p1.y + normal.z * t.p1.z);
}

plane::plane(const Vector3& m, const Vector3& n)
{
	normal = n;
	origin = m;
	ec[0] = n.x;
	ec[1] = n.y;
	ec[2] = n.z;
	ec[3] = -(n.x * m.x + n.y * m.y + n.z * m.z);
}

Vector3 Shapes::TriangleNormal(const triangle& t)
{
	return (t.p2 - t.p1).Cross(t.p3 - t.p1).Normalize();
}

bool Shapes::Collide(const aabb& a, const sphere& s)
{
	float sqDist = 0.0f;
	for (int i = 0; i < 3; i++) {
		// for each axis count any excess distance outside box extents
		float v = s.center[i];
		if (v < a.min[i]) sqDist += (a.min[i] - v) * (a.min[i] - v);
		if (v > a.max[i]) sqDist += (v - a.max[i]) * (v - a.max[i]);
	}
	return sqDist <= s.radius * s.radius;
}

bool Shapes::PointInsideTriangle(const triangle& t, const Vector3& p)
{
	Vector3 e10 = t.p2 - t.p1;
	Vector3 e20 = t.p3 - t.p1;
	float a = e10.Dot(e10);
	float b = e10.Dot(e20);
	float c = e20.Dot(e20);
	float ac_bb = (a * c) - (b * b);
	Vector3 vp(p.x - t.p1.x, p.y - t.p1.y, p.z - t.p1.z);
	float d = vp.Dot(e10);
	float e = vp.Dot(e20);
	float x = (d * c) - (e * b);
	float y = (e * a) - (d * b);
	float z = x + y - ac_bb;
	return (( ((uint32&)z) & ~( ((uint32&)x) | ((uint32&)y) )) & 0x80000000);
}

bool Shapes::PointInsideSphere(const sphere& s, const Vector3& p)
{
	Vector3 d = s.center - p;
	return d.Dot(d) <= s.radius * s.radius;
}

float Shapes::DistanceFromPointToPlane(const plane& p, const Vector3& v)
{
	return std::abs(p.normal.Dot(v) + p.ec[3]);
}

bool Shapes::SharesVertex(const tetrahedron& t, const Vector3& p)
{
	return p == t.p1 || p == t.p2 || p == t.p3 || p == t.p4;
}

bool Shapes::LineIntersectsAABB(const aabb& a, const Vector3& origin, const Vector3& direction, float& distance)
{
	float t1 = (a.min.x - origin.x) / direction.x;
	float t2 = (a.max.x - origin.x) / direction.x;
	float t3 = (a.min.y - origin.y) / direction.y;
	float t4 = (a.max.y - origin.y) / direction.y;
	float t5 = (a.min.z - origin.z) / direction.z;
	float t6 = (a.max.z - origin.z) / direction.z;

	float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
	if (tmax < 0 || tmin > tmax)
		return false;
	distance = tmin;
	return true;
}

aabb Shapes::JoinAABBs(const aabb& a, const aabb& b)
{
	aabb res;
	res.min.x = std::min(a.min.x, b.min.x);
	res.min.y = std::min(a.min.y, b.min.y);
	res.min.z = std::min(a.min.z, b.min.z);
	res.max.x = std::max(a.max.x, b.max.x);
	res.max.y = std::max(a.max.y, b.max.y);
	res.max.z = std::max(a.max.z, b.max.z);
	return res;
}

aabb Shapes::IntersectAABBs(const aabb& a, const aabb& b)
{
	aabb res;
	res.min.x = std::max(a.min.x, b.min.x);
	res.min.y = std::max(a.min.y, b.min.y);
	res.min.z = std::max(a.min.z, b.min.z);
	res.max.x = std::min(a.max.x, b.max.x);
	res.max.y = std::min(a.max.y, b.max.y);
	res.max.z = std::min(a.max.z, b.max.z);
	return res;
}

bool Shapes::AABBsIntersect(const aabb& a, const aabb& b)
{
	return std::max(a.min.x, b.min.x) <= std::min(a.max.x, b.max.x) &&
		std::max(a.min.y, b.min.y) <= std::min(a.max.y, b.max.y) &&
		std::max(a.min.z, b.min.z) <= std::min(a.max.z, b.max.z);
}


