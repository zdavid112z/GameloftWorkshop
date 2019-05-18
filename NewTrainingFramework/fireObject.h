#pragma once

#include "SceneObject.h"

class FireObject : public SceneObject
{
public:
	virtual ~FireObject();
	void Draw(ESContext* context) override;
	void Update(ESContext* context, float delta) override;

	float time = 0;
	float dispMax;
	float fireSpeed;
	int fireNumBlades;
	float fireHeight;
};
