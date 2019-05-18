#pragma once

#include "../Utilities/Math.h"

struct triangle
{
	triangle() {}
	triangle(const Vector3& p1, const Vector3& p2, const Vector3& p3)
		: p1(p1), p2(p2), p3(p3) {}
	union
	{
		Vector3 p[3];
		struct
		{
			Vector3 p1, p2, p3;
		};
	};
};

struct plane
{
	plane() {}
	plane(const triangle& t);
	plane(const Vector3& m, const Vector3& n);

	Vector3 normal, origin;
	float ec[4];
};

struct aabb
{
	Vector3 min, max;
};

struct sphere
{
	Vector3 center;
	float radius;
};

struct tetrahedron
{
	union
	{
		Vector3 p[4];
		struct
		{
			Vector3 p1, p2, p3, p4;
		};
	};
};

class Shapes
{
public:
	static Vector3 TriangleNormal(const triangle& t);
	static bool Collide(const aabb& a, const sphere& s);
	static bool PointInsideTriangle(const triangle& t, const Vector3& p);
	static bool PointInsideSphere(const sphere& s, const Vector3& p);
	static float DistanceFromPointToPlane(const plane& p, const Vector3& v);
	static bool SharesVertex(const tetrahedron& t, const Vector3& p);
	static bool LineIntersectsAABB(const aabb& a, const Vector3& origin, const Vector3& direction, float& distance);
	static aabb JoinAABBs(const aabb& a, const aabb& b);
	static aabb IntersectAABBs(const aabb& a, const aabb& b);
	static bool AABBsIntersect(const aabb& a, const aabb& b);
};

