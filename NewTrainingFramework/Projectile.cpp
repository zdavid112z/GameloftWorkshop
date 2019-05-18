#include "stdafx.h"
#include "Projectile.h"
#include "SceneManager.h"
#include "bulletGamemode.h"

Projectile::Projectile()
{

}

Projectile::~Projectile()
{
	delete trajectoryData.trajectory;
}

void Projectile::Draw(ESContext* context)
{
	SceneObject::Draw(context);
}

void Projectile::Update(ESContext* context, float delta)
{
	SceneObject::Update(context, delta);
	BulletGamemode* g = (BulletGamemode*)SceneManager::instance->gamemode;
	if (g->OutsideMap(position))
	{
		Kill();
	}
}

void Projectile::Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	SceneObject::Key(esContext, key, bIsPressed);
}

void Projectile::Kill()
{
	dead = true;
	if (trail)
		trail->dead = true;
}