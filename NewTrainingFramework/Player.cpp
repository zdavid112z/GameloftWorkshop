#include "stdafx.h"
#include "Player.h"
#include "bulletGamemode.h"
#include "SceneManager.h"

Player::Player()
{
	
}

Player::~Player()
{

}

void Player::Draw(ESContext* context)
{
	if (invulnerabilityTimer > 0)
	{
		int phase = (int(invulnerabilityTimer / invulnerabilityFlashSpeed)) % 2;
		if (phase == 0)
			diffuseColor = invulnerabilityWhite;
		else diffuseColor = invulnerabilityRed;
	}
	else diffuseColor.w = 0.f;

	SceneObject::Draw(context);
}

void Player::Update(ESContext* context, float delta)
{
	SceneObject::Update(context, delta);

	invulnerabilityTimer = std::max(invulnerabilityTimer - delta, 0.f);

	BulletGamemode* g = (BulletGamemode*)SceneManager::instance->gamemode;
	Vector3 newPos = position;
	if (SceneManager::instance->input->KeyDown('A'))
		newPos.x -= speed * delta;
	if (SceneManager::instance->input->KeyDown('D'))
		newPos.x += speed * delta;
	if (SceneManager::instance->input->KeyDown('W'))
		newPos.z -= speed * delta;
	if (SceneManager::instance->input->KeyDown('S'))
		newPos.z += speed * delta;

	newPos.x = std::min(newPos.x, g->playerMapSize.x);
	newPos.z = std::min(newPos.z, g->playerMapSize.y);
	newPos.x = std::max(newPos.x, -g->playerMapSize.x);
	newPos.z = std::max(newPos.z, -g->playerMapSize.y);

	position = newPos;

	Matrix invVP = SceneManager::instance->activeCamera->GetInvViewProjection();
	Vector2 mouseCoords = SceneManager::instance->input->GetMousePos();
	Vector4 screenCoords = Vector4(mouseCoords.x, mouseCoords.y, 0, 1);
	Vector4 worldCoords = screenCoords * invVP;
	worldCoords /= worldCoords.w;

	//Matrix vp = SceneManager::instance->activeCamera->GetViewProjection();
	//Vector4 objCoords = Vector4(position, 1);
	//Vector4 oldScreen = objCoords * vp;
	//oldScreen /= oldScreen.w;

	//Vector4 objInapoi = oldScreen * invVP;

	Vector3 c = SceneManager::instance->activeCamera->data.position;
	Vector3 p = worldCoords;

	Vector3 point = Vector3(
		c.x + (c.y * (p.x - c.x)) / (c.y - p.y),
		0,
		c.z + (c.y * (p.z - c.z)) / (c.y - p.y));

	Vector3 direction = (point - position).Normalize();
	float angle = -std::atan2(direction.z, direction.x) + PI;
	rotation.y = angle;

	static int x = 0;
	x = std::min(x + 1, 100);
	if (SceneManager::instance->input->IsMouseButtonDown(Input::LButton) && x > 8)
	{
		x = 0;

		g->SpawnProjectile(position, direction, 80, true);
		//angle += 0.2f;
		//Vector3 dir1 = Vector3(std::cos(angle), 0, std::sin(angle));
		//g->SpawnProjectile(position, dir1, 45);
		//angle -= 0.4f;
		//dir1 = Vector3(std::cos(angle), 0, std::sin(angle));
		//g->SpawnProjectile(position, dir1, 45);
	}

	static int k = 0;
	k = (k + 1) % 30;

	if (k == 0 && 0)
	{
		printf("Mouse: %f %f\n", mouseCoords.x, mouseCoords.y);
		printf("Pozitie obj: %f %f %f\n", position.x, position.y, position.z);
		//printf("Obj pe ecran: %f %f %f\n", oldScreen.x, oldScreen.y, oldScreen.z);
		//printf("Obj inapoi: %f %f %f\n", objInapoi.x, objInapoi.y, objInapoi.z);
		printf("Gaseste pct: %f %f %f\n", worldCoords.x, worldCoords.y, worldCoords.z);
		printf("Intersectie cu plan: %f %f %f\n", point.x, point.y, point.z);
		printf("\n");
	}
}

void Player::Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	SceneObject::Key(esContext, key, bIsPressed);
}

void Player::OnHit()
{
	if (invulnerabilityTimer <= 0.f)
	{
		ResourceManager::instance->LoadSound("died")->Play(false);
		invulnerabilityTimer = invulnerabilityTimeout;
	}
}