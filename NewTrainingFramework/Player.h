#pragma once

#include "SceneObject.h"

class Player : public SceneObject
{
public:
	Player();
	virtual ~Player();
	void Draw(ESContext* context) override;
	void Update(ESContext* context, float delta) override;
	void Key(ESContext *esContext, unsigned char key, bool bIsPressed) override;
	void OnHit();

public:
	float speed;
	int hp;
	float invulnerabilityTimeout;
	float invulnerabilityTimer = 0;
	float invulnerabilityFlashSpeed;
	Vector4 invulnerabilityWhite;
	Vector4 invulnerabilityRed;
};