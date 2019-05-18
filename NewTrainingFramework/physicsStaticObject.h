#pragma once

#include "puppy.h"
#include "shapes.h"
#include "graphics/meshShared.h"

namespace puppy {

	class PhysicsStaticObject
	{
	public:
		PhysicsStaticObject() {}
		PhysicsStaticObject(MeshData meshData, const glm::mat4& model = glm::mat4(1), float sizeThreshold = 0.1f);
		~PhysicsStaticObject();

		float m_SizeThreshold;
		std::vector<triangle> m_Triangles;
		aabb m_AABB;
	};

}
