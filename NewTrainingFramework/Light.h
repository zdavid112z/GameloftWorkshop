#pragma once

#include "../Utilities/Math.h"

class SceneObject;

struct DirectionalLight
{
	Vector4 direction;
	Vector4 color;
	float intensity;
};

struct PointLight
{
	SceneObject* following;
	Vector4 position;
	Vector4 color;
	Vector4 attenuation;
	float intensity;
};

struct SpotLight
{
	SceneObject* following;
	bool followsPosition;
	bool followsDirection;
	Vector4 position;
	Vector4 direction;
	Vector4 color;
	Vector4 attenuation;
	float angleCosMin;
	float angleCosMax;
	float intensity;
	float unused1;
};
