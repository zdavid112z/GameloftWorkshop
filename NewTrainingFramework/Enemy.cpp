#include "stdafx.h"
#include "Enemy.h"
#include "Player.h"
#include "bulletGamemode.h"
#include "SceneManager.h"

Enemy::Enemy()
{

}

Enemy::~Enemy()
{
	delete trajectoryData.trajectory;
}

void Enemy::Draw(ESContext* context)
{
	if (hurtTimer > 0)
		diffuseColor = hurtRed;
	else diffuseColor.w = 0.f;
	SceneObject::Draw(context);
}

void Enemy::Update(ESContext* context, float delta)
{
	SceneObject::Update(context, delta);

	hurtTimer = std::max(hurtTimer - delta, 0.f);

	BulletGamemode* g = (BulletGamemode*)SceneManager::instance->gamemode;

	if (!g->OutsideMap(position))
		wasVisible = true;
	if (g->OutsideMap(position) && wasVisible)
	{
		dead = true;
	}

	shootTimer += delta;

	if (!g->InsidePlayerMap(position) && shootTimer >= shootInterval)
		shootTimer = shootInterval - 0.001f;

	while(shootTimer >= shootInterval)
	{
		numShot++;
		shootTimer -= shootInterval;
		Vector3 mainDirection = Vector3(0, 0, -1);
		if (projectileType != EnemyProjectileType::CIRCULAR && homingPlayer && player != nullptr)
		{
			mainDirection = (player->position - position).Normalize();
		}

		float angle;
		float spacing;
		float start;

		switch (projectileType)
		{
		case EnemyProjectileType::SIMPLE:
			angle = atan2(mainDirection.z, mainDirection.x);
			spacing = projectilesSpacing;
			start = angle - spacing * (numProjectiles - 1) / 2.f;
			break;
		case EnemyProjectileType::CIRCULAR:
			angle = (numShot % 2) * PI / numProjectiles;
			spacing = 2.f * PI / numProjectiles;
			start = 0;
			break;
		}

		float currentAngle = start;
		for (int i = 0; i < numProjectiles; i++)
		{
			Vector3 dir = Vector3(cosf(currentAngle), 0, sinf(currentAngle));
			g->SpawnProjectile(position, dir, projectileSpeed, false);
			currentAngle += spacing;
		}
	}
}

void Enemy::Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{

}

void Enemy::OnHit()
{
	hp--;
	if (hp <= 0)
	{
		ResourceManager::instance->LoadSound("destroyed")->Play(true);
		dead = true;
	}
	else
	{
		ResourceManager::instance->LoadSound("hit")->Play(true);
		hurtTimer = hurtTimeout;
	}
}
