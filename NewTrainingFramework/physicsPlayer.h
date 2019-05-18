#pragma once

#include "puppy.h"

namespace puppy {

	class PhysicsPlayer
	{
	public:
		PhysicsPlayer() {}
		PhysicsPlayer(const glm::vec3& size, const glm::vec3& pos) :
			m_Size(size), m_Position(pos)
		{

		}

		~PhysicsPlayer()
		{

		}

		glm::vec3 m_Position = glm::vec3(0, 0, 0);
		glm::vec3 m_MovementVelocity = glm::vec3(0, 0, 0);
		glm::vec3 m_GravityVelocity = glm::vec3(0, 0, 0);
		float m_Mass = 1;
		bool m_InAir = true;
		glm::vec3 m_Size;
	};

}