#pragma once

#include "SceneObject.h"

class Player;

enum class EnemyProjectileType
{
	SIMPLE,
	CIRCULAR
};

class Enemy : public SceneObject
{
public:
	Enemy();
	virtual ~Enemy();

	virtual void Draw(ESContext* context) override;
	virtual void Update(ESContext* context, float delta) override;
	virtual void Key(ESContext *esContext, unsigned char key, bool bIsPressed) override;
	void OnHit();
	
	EnemyProjectileType projectileType = EnemyProjectileType::SIMPLE;
	bool homingPlayer = false;
	float projectileSpeed = 50;
	int numProjectiles = 1;
	float projectilesSpacing = 0.15f;
	float shootInterval = 2.f;
	int hp;
	Vector4 hurtRed;
	float hurtTimeout;
	float hurtTimer = 0.f;
	int numShot = 0;

	Player* player;
private:
	bool wasVisible = false;
	float shootTimer = 0.f;
};