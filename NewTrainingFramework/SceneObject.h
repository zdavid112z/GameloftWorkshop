#pragma once

#include "stdafx.h"
#include <vector>
#include "../Utilities/utilities.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Trajectory.h"
#include "reflection.h"
#include "Sound.h"

class SceneManager;

class SceneObject
{
public:
	virtual ~SceneObject() {}

	virtual void Update(ESContext* context, float delta);
	virtual void Draw(ESContext* context);
	virtual void Key(ESContext *esContext, unsigned char key, bool bIsPressed);
	virtual AABB GetAABB();
	virtual AABB GetCollisionBox();
	Matrix GetModelMatrix();

	bool customCollisionBox;
	Vector3 collisionBoxMin;
	Vector3 collisionBoxMax;
	float colBoxScaleFactor;
	Sound* sound = nullptr;
	bool noCollision = false;
	Vector3 aabbColor = Vector3(0.2f, 0.8f, 0.3f);
	uint32 id;
	Vector3 position = Vector3(0, 0, 0);
	Vector3 rotation = Vector3(0, 0, 0);
	Vector3 scale = Vector3(1, 1, 1);
	Model* model = nullptr;
	Shader* shader = nullptr;
	std::string name;
	bool wired = false;
	bool dontDrawNormals = false;
	std::vector<Texture*> textures;
	SceneObject* following = nullptr;
	Vector2 shininess = Vector2(0, 1);
	TrajectoryData trajectoryData;
	Vector3 newRotation = Vector3(0, 0, 0);
	float rotationTransitionSpeed = 5;
	Vector4 diffuseColor = Vector4(1, 0, 1, 1);
	Vector4 specularColor = Vector4(1, 1, 1, 1);
	Vector4 defaultNormal = Vector4(0, 0, 1, 1);
	float specularPower = 8;
	float diffuseIntensity = 1;
	float specularIntensity = 1;
	bool dead = false;
};