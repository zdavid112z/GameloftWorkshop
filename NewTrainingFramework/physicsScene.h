#pragma once

#include "puppy.h"
#include "physicsPlayer.h"
#include "physicsStaticObject.h"
#include "kdtree.h"
#include "core/scene.h"

namespace puppy {

	class PhysicsScene
	{
	private:
		struct CollisionStats
		{
			bool hit;
			triangle t;
			float time;
			float dist;
			glm::vec3 newPosition;
			plane slidingPlane;
		};
	public:
		static const uint32 INVALID_STATIC_OBJECT_ID;

		PhysicsScene(Scene* scene, const glm::vec3& gravity);
		~PhysicsScene();

		void Update(PhysicsPlayer& player, float delta);
		glm::vec3 UpdatePlayer(glm::vec3 pos, glm::vec3 vel, glm::vec3 size);
		CollisionStats GetFirstCollision(const glm::vec3& pos, const glm::vec3& vel, const glm::vec3& trans, const glm::vec3& size);
		CollisionStats GetFirstCollision(const PhysicsStaticObject& s, const glm::vec3& pos, const glm::vec3& vel, const glm::vec3& trans);
		bool CheckCollision(const triangle& t, const glm::vec3& pos, const glm::vec3& vel, CollisionStats& hit);
		bool GetLowestRoot(float a, float b, float c, float maxR, float* root);
		RaycastHitData Raycast(const glm::vec3& start, const glm::vec3& direction);
		RaycastHitData RaycastObject(const PhysicsStaticObject& object, const glm::vec3& start, const glm::vec3& direction);
		uint32 AllocateStaticObject(MeshData meshData, const glm::mat4& model = glm::mat4(1), float sizeThreshold = 0.1f);
		void AddImGuiMenu();
		bool ShouldDebugDisplayTree() { return m_DisplayTriangleAABBs; }
		std::vector<std::pair<aabb, int> > GetTrianglesInTree();

		glm::vec3 m_Gravity;
		float m_GravityMaxValue = 40.f;
		float m_SmallDistance = 0.01f;
		float m_Epsilon = 1e-6f;
		float m_BigFloat = 1e30f;
	private:
		Scene* m_Scene;
		KDTree m_KDTree;
		std::vector<triangle> m_Triangles;
		std::vector<PhysicsStaticObject> m_StaticObjects;
		float m_ElapsedTime = 0;
		uint32 m_BuildListener;
		bool m_DisplayTriangleAABBs = false;
		int m_DisplayTriangleAABBsMinDepth = 0;
		int m_DisplayTriangleAABBsMaxDepth = 0;
	};

}