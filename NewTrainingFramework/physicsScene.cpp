#include "puppy.h"
#include "physicsScene.h"
#include "thirdparty/imgui/imgui.h"
#include "core/event/eventManager.h"
#include "core/game.h"

namespace puppy {

	const uint32 PhysicsScene::INVALID_STATIC_OBJECT_ID = 0xffffffff;

	bool PhysicsScene::GetLowestRoot(float a, float b, float c, float maxR, float* root)
	{
		// Check if a solution exists
		float determinant = b * b - 4.0f * a * c;
		// If determinant is negative it means no solutions.
		if (determinant < 0.0f) return false;
		// calculate the two roots: (if determinant == 0 then
		// x1==x2 but let’s disregard that slight optimization)
		float sqrtD = sqrt(determinant);
		float r1 = (-b - sqrtD) / (2 * a);
		float r2 = (-b + sqrtD) / (2 * a);
		// Sort so x1 <= x2
		if (r1 > r2) {
			float temp = r2;
			r2 = r1;
			r1 = temp;
		}
		// Get lowest root:
		if (r1 > 0 && r1 < maxR) {
			*root = r1;
			return true;
		}
		// It is possible that we want x2 - this can happen
		// if x1 < 0
		if (r2 > 0 && r2 < maxR) {
			*root = r2;
			return true;
		}
		// No (valid) solutions
		return false;
	}

	PhysicsScene::PhysicsScene(Scene* scene, const glm::vec3& gravity) : 
		m_Gravity(gravity), m_Scene(scene)
	{
		m_BuildListener = m_Scene->GetGame()->GetEventManager()->AddEventListener(EventType::BUILD_PHYSICS_SCENE,
			EventListener([this](Event e) -> bool
		{
			m_KDTree.Init(m_Triangles);
			return false;
		}
			, 1000));
	}

	PhysicsScene::~PhysicsScene()
	{
		m_Scene->GetGame()->GetEventManager()->RemoveEventListener(EventType::BUILD_PHYSICS_SCENE, m_BuildListener);
		m_KDTree.Delete();
	}

	void PhysicsScene::Update(PhysicsPlayer& player, float delta)
	{
		/*if (m_Player->m_Hooked)
		{
			glm::vec3 vel = GetHookResultVelocity(m_Player->m_Position, m_Player->m_HookPoint, m_Player->m_HookLength, m_Player->m_Mass);
			m_Player->m_HookVelocity += vel;
		}
		m_Player->m_Position = UpdatePlayer(m_Player->m_Position, m_Player->m_HookVelocity * delta, m_Player->m_Size);
		m_Player->m_HookVelocity *= 0.95f;*/

		player.m_Position = UpdatePlayer(player.m_Position, player.m_MovementVelocity * delta, player.m_Size);
		player.m_GravityVelocity += m_Gravity * delta * 4.f;
		float gravityLen2 = glm::length2(player.m_GravityVelocity);
		if (gravityLen2 >= m_GravityMaxValue * m_GravityMaxValue)
		{
			player.m_GravityVelocity /= glm::sqrt(gravityLen2);
			player.m_GravityVelocity *= m_GravityMaxValue;
		}
		glm::vec3 oldPosition = player.m_Position;
		player.m_Position = UpdatePlayer(player.m_Position, player.m_GravityVelocity  * delta, player.m_Size);

		RaycastHitData hit = Raycast(player.m_Position, glm::vec3(0, -1, 0));
		if (hit.hit && hit.distance <= 0.25f + player.m_Size.y)
		{
			player.m_GravityVelocity = m_Gravity;
			player.m_InAir = false;
		}
		else player.m_InAir = true;
	}

	glm::vec3 PhysicsScene::UpdatePlayer(glm::vec3 orgPos, glm::vec3 orgVel, glm::vec3 size)
	{
		glm::vec3 trans(1.f / size.x, 1.f / size.y, 1.f / size.z);
		glm::vec3 pos = orgPos, vel = orgVel;
		pos.x *= trans.x; pos.y *= trans.y; pos.z *= trans.z;
		vel.x *= trans.x; vel.y *= trans.y; vel.z *= trans.z;
		plane firstPlane;
		glm::vec3 dest = pos + vel;
		//LOG_INFO(pos, vel, dest);
		for (int i = 0; i < 3; i++)
		{
			CollisionStats stats = GetFirstCollision(pos, vel, trans, size);
			if (!stats.hit)
			{
				dest.x *= size.x;
				dest.y *= size.y;
				dest.z *= size.z;
				return dest;
			}
			if (glm::isnan(stats.newPosition) != glm::bvec3(0, 0, 0))
				break;
			pos = stats.newPosition;
			if (i == 0)
			{
				float longRadius = 1 + m_SmallDistance;
				firstPlane = stats.slidingPlane;
				dest -= (Shapes::DistanceFromPointToPlane(firstPlane, dest) - longRadius) * firstPlane.normal;
				vel = dest - pos;
			}
			else if (i == 1)
			{
				plane secondPlane = stats.slidingPlane;
				glm::vec3 crease = glm::normalize(glm::cross(firstPlane.normal, secondPlane.normal));
				float dis = glm::dot(dest - pos, crease);
				vel = dis * crease;
				dest = pos + vel;
			}
			if (glm::isnan(vel) != glm::bvec3(0, 0, 0) || glm::isnan(dest) != glm::bvec3(0, 0, 0))
				break;
			//LOG_INFO("i = ", i);
			//LOG_INFO(pos, vel, dest, stats.slidingPlane.normal, stats.slidingPlane.origin);
			//LOG_INFO(stats.dist, " ", stats.time);
		}
		pos.x *= size.x;
		pos.y *= size.y;
		pos.z *= size.z;
		return pos;
	}

	PhysicsScene::CollisionStats PhysicsScene::GetFirstCollision(const glm::vec3& pos, const glm::vec3& vel, const glm::vec3& trans, const glm::vec3& size)
	{
		glm::vec3 orgPos = pos / trans;
		glm::vec3 orgVel = vel / trans;
		glm::vec3 direction = glm::normalize(orgVel);
		aabb firstAABB;
		firstAABB.min = orgPos - size;
		firstAABB.max = orgPos + size;
		aabb finalAABB;
		finalAABB.min = orgPos + orgVel - size;
		finalAABB.max = orgPos + orgVel + size;
		aabb trajectoryAABB = Shapes::JoinAABBs(firstAABB, finalAABB);
		std::vector<triangle> list = m_KDTree.GetTrianglesInside(trajectoryAABB);
		CollisionStats hit;
		hit.hit = false;
		for (auto& t : list)
		{
			t.p1 *= trans;
			t.p2 *= trans;
			t.p3 *= trans;
			CollisionStats newhit;
			bool hitFace = false;
			hitFace = CheckCollision(t, pos, vel, newhit);
			if (!hitFace)
			{
				std::swap(t.p1, t.p2);
				hitFace = CheckCollision(t, pos, vel, newhit);
			}
			if (hitFace)
			{
				if (newhit.hit && (!hit.hit || newhit.time < hit.time))
					hit = newhit;
			}
		}
		return hit;
		/*
		sphere player;
		player.center = pos;
		player.radius = 1;
		for (const PhysicsStaticObject& s : m_StaticObjects)
		{
			aabb a = s.m_AABB;
			a.min.x *= trans.x; a.max.x *= trans.x;
			a.min.y *= trans.y; a.max.y *= trans.y;
			a.min.z *= trans.z; a.max.z *= trans.z;
			if (Shapes::Collide(a, player))
			{
				CollisionStats newhit = GetFirstCollision(s, pos, vel, trans);
				if (newhit.hit && (!hit.hit || newhit.time < hit.time))
					hit = newhit;
			}
		}
		return hit;
		*/
	}

	PhysicsScene::CollisionStats PhysicsScene::GetFirstCollision(const PhysicsStaticObject& s, const glm::vec3& pos, const glm::vec3& vel, const glm::vec3& trans)
	{
		CollisionStats hit, newhit;
		hit.hit = false;
		for (triangle t : s.m_Triangles)
		{
			t.p1.x *= trans.x; t.p2.x *= trans.x; t.p3.x *= trans.x;
			t.p1.y *= trans.y; t.p2.y *= trans.y; t.p3.y *= trans.y;
			t.p1.z *= trans.z; t.p2.z *= trans.z; t.p3.z *= trans.z;
			bool res = CheckCollision(t, pos, vel, newhit);
			if (res && (!hit.hit || newhit.time < hit.time))
				hit = newhit;
		}
		return hit;
	}

	bool PhysicsScene::CheckCollision(const triangle& t, const glm::vec3& pos, const glm::vec3& vel, CollisionStats& hit)
	{
		plane p(t);
		float signedDist = glm::dot(p.normal, pos) + p.ec[3];
		float ndotv = glm::dot(p.normal, vel);
		float t0, t1;
		if (ndotv > 0)
			return false;
		if (glm::epsilonEqual(ndotv, 0.f, m_Epsilon))
		{
			if (glm::abs(signedDist) > 1)
				return false;
			t0 = 0;
			t1 = 1;
		}
		else
		{
			t0 = (-1 - signedDist) / ndotv;
			t1 = ( 1 - signedDist) / ndotv;
			if (t0 > t1)
			{
				float aux = t0;
				t0 = t1;
				t1 = aux;
			}
			if (t0 > 1 || t1 < 0)
				return false;
			t0 = glm::clamp(t0, 0.f, 1.f);
			t1 = glm::clamp(t1, 0.f, 1.f);
		}
		glm::vec3 planeIntersection = pos - p.normal + t0 * vel;

		glm::vec3 intersectionPoint;
		float intersectionTime;

		if (Shapes::PointInsideTriangle(t, planeIntersection))
		{
			intersectionPoint = planeIntersection;
			intersectionTime = t0;
		}
		else
		{
			float vdotv = glm::dot(vel, vel);
			intersectionTime = 100;
			for (int i = 0; i < 3; i++)
			{
				if (!GetLowestRoot(vdotv, 2 * (glm::dot(vel, pos - t.p[i])), glm::length2(pos - t.p[i]) - 1, intersectionTime, &intersectionTime))
					continue;
				intersectionPoint = t.p[i];
			}
			for (int i = 0; i < 3; i++)
			{
				float intTime;
				glm::vec3 edge = t.p[(i + 1) % 3] - t.p[i];
				glm::vec3 baseToVertex = t.p[i] - pos;
				float edote = glm::dot(edge, edge);
				float edotv = glm::dot(edge, vel);
				float vdotb = glm::dot(vel, baseToVertex);
				float edotb = glm::dot(edge, baseToVertex);
				float bdotb = glm::dot(baseToVertex, baseToVertex);
				if (!GetLowestRoot(
					edote * -vdotv + edotv * edotv,
					edote * 2 * vdotb - 2 * edotv * edotb,
					edote * (1 - bdotb) + edotb * edotb,
					intersectionTime, &intTime))
					continue;
				float f0 = (edotv * intTime - edotb) / edote;
				if (f0 >= 0 && f0 <= 1)
				{
					intersectionTime = intTime;
					intersectionPoint = t.p[i] + f0 * edge;
				}
			}
			if (intersectionTime > 1)
				return false;
		}
		float lenv = glm::length(vel);
		float intersectionDistance = intersectionTime * lenv;
		glm::vec3 newPosition = pos + glm::max(intersectionDistance - m_SmallDistance, 0.f) * glm::normalize(vel);
		hit.slidingPlane = plane(intersectionPoint, glm::normalize(newPosition - intersectionPoint));
		hit.t = t;
		hit.time = intersectionTime;
		hit.dist = intersectionDistance;
		hit.newPosition = newPosition;
		hit.hit = true;
		return true;
	}

	RaycastHitData PhysicsScene::Raycast(const glm::vec3& start, const glm::vec3& direction)
	{
		return m_KDTree.Raycast(start, direction);
		std::vector<std::pair<float, uint32> > potentialHits;
		for(uint32 i = 0; i < m_StaticObjects.size(); i++)
		{
			const PhysicsStaticObject& o = m_StaticObjects[i];
			float dist;
			if (Shapes::LineIntersectsAABB(o.m_AABB, start, direction, dist))
				potentialHits.push_back(std::make_pair(dist, i));
		}
		std::sort(potentialHits.begin(), potentialHits.end());
		RaycastHitData hit;
		hit.hit = false;
		hit.distance = m_BigFloat;
		for (const auto& p : potentialHits)
		{
			if (p.first > hit.distance)
				break;
			RaycastHitData temp = RaycastObject(m_StaticObjects[p.second], start, direction);
			if (hit.distance > temp.distance)
				hit = temp;
		}
		return hit;
	}

	RaycastHitData PhysicsScene::RaycastObject(const PhysicsStaticObject& object, const glm::vec3& start, const glm::vec3& direction)
	{
		RaycastHitData hit;
		hit.hit = false;
		hit.distance = m_BigFloat;
		for (const triangle& tr : object.m_Triangles)
		{
			glm::vec3 vertex0 = tr.p1;
			glm::vec3 vertex1 = tr.p2;
			glm::vec3 vertex2 = tr.p3;
			glm::vec3 edge1, edge2, h, s, q;
			float a, f, u, v;
			edge1 = vertex1 - vertex0;
			edge2 = vertex2 - vertex0;
			h = glm::cross(direction, edge2);
			a = glm::dot(edge1, h);
			if (a > -m_Epsilon && a < m_Epsilon)
				continue;
			f = 1.0f / a;
			s = start - vertex0;
			u = f * (glm::dot(s, h));
			if (u < 0.0f || u > 1.0f)
				continue;
			q = glm::cross(s, edge1);
			v = f * glm::dot(direction, q);
			if (v < 0.0f || u + v > 1.0f)
				continue;
			float t = f * glm::dot(edge2, q);
			if (t > m_Epsilon)
			{
				if (hit.distance > t)
				{
					hit.point = start + direction * t;
					hit.distance = t;
					hit.hit = true;
					hit.triangle = tr;
				}
			}
		}
		return hit;
	}

	uint32 PhysicsScene::AllocateStaticObject(MeshData meshData, const glm::mat4& model, float sizeThreshold)
	{
		m_StaticObjects.emplace_back(meshData, model, sizeThreshold);
		for (const auto& t : m_StaticObjects.back().m_Triangles)
			m_Triangles.push_back(t);
		return m_StaticObjects.size() - 1;
	}

	void PhysicsScene::AddImGuiMenu()
	{
		ImGui::SliderFloat3("Gravity", &m_Gravity.x, -20, 20);
		ImGui::SliderFloat("GravityMaxValue", &m_GravityMaxValue, 0, 100);
		ImGui::InputFloat("SmallDistance", &m_SmallDistance, 0, 0, 10);
		ImGui::InputFloat("Epsilon", &m_Epsilon, 0, 0, 10);
		ImGui::InputFloat("BigFloat", &m_BigFloat);
		ImGui::Checkbox("Display Triangle AABBs", &m_DisplayTriangleAABBs);
		ImGui::InputInt("Tree Min Depth", &m_DisplayTriangleAABBsMinDepth);
		ImGui::InputInt("Tree Max Depth", &m_DisplayTriangleAABBsMaxDepth);
	}

	std::vector<std::pair<aabb, int>> PhysicsScene::GetTrianglesInTree()
	{
		return m_KDTree.GetAABBs(m_DisplayTriangleAABBsMinDepth, m_DisplayTriangleAABBsMaxDepth);
	}
}
