#pragma once

#include "../Utilities/Math.h"

struct AABB
{
	Vector3 min, max;

	bool Collides(AABB a)
	{
		return min.x <= a.max.x && max.x >= a.min.x &&
			   min.y <= a.max.y && max.y >= a.min.y &&
			   min.z <= a.max.z && max.z >= a.min.z;
	}

	bool Inside(Vector3 p)
	{
		return min.x <= p.x && p.x <= max.x &&
			min.y <= p.y && p.y <= max.y &&
			min.z <= p.z && p.z <= max.z;
	}

};