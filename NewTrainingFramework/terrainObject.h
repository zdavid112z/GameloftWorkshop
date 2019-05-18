#pragma once

#include "SceneObject.h"
#include "reflection.h"

class TerrainObject : public SceneObject
{
public:
	virtual ~TerrainObject();
	void Draw(ESContext* context) override;
	void Update(ESContext* context, float delta) override;

	Vector2 terrainSize;
	float cameraHeight;
	Vector2 texMult;
	float texMultSingle;
	Vector3 terrainHeights;

	bool fakeMoveCenter;
	Vector3 fakePosition = Vector3(0, 0, 0);
	Vector3 fakeDirection;
	float fakeMoveSpeed;
};