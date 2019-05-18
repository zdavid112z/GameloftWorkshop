#include "puppy.h"
#include "physicsStaticObject.h"

namespace puppy {

	PhysicsStaticObject::PhysicsStaticObject(MeshData meshData, const glm::mat4& model, float sphereThreshold)
	{
		Vertex* vertices = meshData.vertices;
		uint32 numVertices = meshData.numVertices;
		uint32* indices = meshData.indices;
		uint32 numIndices = meshData.numIndices;
		m_SizeThreshold = sphereThreshold;
		for (uint i = 0; i < numIndices; i += 3)
		{
			triangle t;
			t.p1 = vertices[indices[i]].position;
			t.p2 = vertices[indices[i + 1]].position;
			t.p3 = vertices[indices[i + 2]].position;

			glm::vec3 originalNormal = vertices[indices[i]].normal +
				vertices[indices[i + 1]].normal +
				vertices[indices[i + 2]].normal;

			glm::vec3 newNormal = glm::cross(
				t.p2 - t.p1,
				t.p3 - t.p1);

			if (glm::dot(originalNormal, newNormal) < 0)
			{
				std::swap(t.p2, t.p1);
			}
			if (glm::isnan(glm::normalize(newNormal)) == glm::bvec3(0, 0, 0))
				m_Triangles.push_back(t);
		}
		
		for (uint i = 0; i < m_Triangles.size(); i++)
		{
			m_Triangles[i].p1 = model * glm::vec4(m_Triangles[i].p1, 1);
			m_Triangles[i].p2 = model * glm::vec4(m_Triangles[i].p2, 1);
			m_Triangles[i].p3 = model * glm::vec4(m_Triangles[i].p3, 1);
		}
		float maxdist = 0;
		aabb a;
		a.min = a.max = m_Triangles[0].p1;
		for (uint i = 0; i < m_Triangles.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				a.min.x = glm::min(a.min.x, m_Triangles[i].p[j].x);
				a.max.x = glm::max(a.max.x, m_Triangles[i].p[j].x);
				a.min.y = glm::min(a.min.y, m_Triangles[i].p[j].y);
				a.max.y = glm::max(a.max.y, m_Triangles[i].p[j].y);
				a.min.z = glm::min(a.min.z, m_Triangles[i].p[j].z);
				a.max.z = glm::max(a.max.z, m_Triangles[i].p[j].z);
			}
		}
		a.min -= glm::vec3(m_SizeThreshold, m_SizeThreshold, m_SizeThreshold);
		a.max += glm::vec3(m_SizeThreshold, m_SizeThreshold, m_SizeThreshold);
		m_AABB = a;
	}

	PhysicsStaticObject::~PhysicsStaticObject()
	{
		
	}

}