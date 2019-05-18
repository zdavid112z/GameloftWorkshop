#pragma once

#include "SceneObject.h"
#include "fireObject.h"

class Projectile : public SceneObject
{
public:
	Projectile();
	virtual ~Projectile();

	void Draw(ESContext* context) override;
	void Update(ESContext* context, float delta) override;
	void Key(ESContext *esContext, unsigned char key, bool bIsPressed) override;
	void Kill();

	bool isPlayers;
	FireObject* trail = nullptr;
private:
};
