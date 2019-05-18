#include "stdafx.h"
#include "bulletGamemode.h"
#include "SceneManager.h"
#include "Projectile.h"
#include "fireObject.h"
#include "MeshFactory.h"
#include "Random.h"

BulletGamemode::BulletGamemode()
{
	canMove = false;
	canLook = false;
	player = (Player*)SceneManager::instance->objects[HASH("player")];
}

BulletGamemode::~BulletGamemode()
{
	ResourceManager::instance->LoadSound("music")->Stop();
}

void BulletGamemode::Parse(rapidxml::xml_node<>* node)
{
	playerMapSize = XMLUtils::ParseVector2(node->first_node("playerMapSize"));
	outsideMapSize = XMLUtils::ParseVector2(node->first_node("outsideMapSize"));
	difficulty = XMLUtils::ParseFloat(node->first_node("difficulty"));
	maxDifficulty = XMLUtils::ParseFloat(node->first_node("maxDifficulty"));
	auto waves = node->first_node("waves");
	wavesRandom = XMLUtils::ParseBool(waves->first_node("wavesRandom"), true);
	for (auto n = waves->first_node("wave"); n; n = n->next_sibling("wave"))
	{
		Wave w = ParseWave(n);
		this->waves.push_back(w);
	}
}

Wave BulletGamemode::ParseWave(rapidxml::xml_node<>* node)
{
	Wave w;
	auto waves = node->first_node("events");
	for (auto n = waves->first_node("event"); n; n = n->next_sibling("event"))
	{
		WaveEvent e = ParseWaveEvent(n);
		w.AddEvent(e);
	}
	return w;
}

WaveEvent BulletGamemode::ParseWaveEvent(rapidxml::xml_node<>* node)
{
	WaveEvent e;
	std::string type = XMLUtils::ParseString(node->first_node("type"), "");
	if (type == "wait")
	{
		e.type = WaveEventType::WAIT;
		Reflection::LoadVariables(&e.wait, node);
	}
	else if (type == "spawn")
	{
		e.type = WaveEventType::SPAWN_ENEMY;
		Reflection::LoadVariables(&e.spawn, node);
	}
	return e;
}

void BulletGamemode::UpdateLogic(ESContext* context, float delta)
{
	if (currentWaveIndex == -1)
	{
		if (wavesRandom)
			currentWaveIndex = Random::Index(waves, lastWaveIndex);
		else currentWaveIndex = (lastWaveIndex + 1) % waves.size();
		lastWaveIndex = currentWaveIndex;

		waves[currentWaveIndex].Start(context, delta * currentDifficulty, this);
	}

	if (!waves[currentWaveIndex].Update(context, delta * currentDifficulty))
	{
		currentWaveIndex = -1;
	}
}

void BulletGamemode::Update(ESContext* context, float delta)
{
	ResourceManager::instance->LoadSound("music")->Play(false);
	currentDifficulty = std::min(currentDifficulty + delta * difficulty, maxDifficulty);
	DebugCameraUpdate(context, delta);
	UpdateLogic(context, delta);

	SceneManager::RemoveIfDead(playerProjectiles, false);
	SceneManager::RemoveIfDead(enemyProjectiles, false);
	SceneManager::RemoveIfDead(enemies, false);
	for(auto it : enemyProjectiles)
	{
		if (it.second->dead == false && player->GetCollisionBox().Collides(it.second->GetCollisionBox()))
		{
			player->OnHit();
			it.second->Kill();
		}
	}

	for (auto e : enemies)
	{
		if (e.second->dead)
			continue;
		for (auto pp : playerProjectiles)
		{
			if (pp.second->dead == false && e.second->GetCollisionBox().Collides(pp.second->GetCollisionBox()))
			{
				e.second->OnHit();
				pp.second->Kill();
				break;
			}
		}
	}
	SceneManager::RemoveIfDead(playerProjectiles, false);
	SceneManager::RemoveIfDead(enemyProjectiles, false);
	SceneManager::RemoveIfDead(enemies, false); 
}

void BulletGamemode::Draw(ESContext* context)
{

}

void BulletGamemode::Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	DebugCameraKey(esContext, key, bIsPressed);
}

void BulletGamemode::SpawnProjectile(Vector3 position, Vector3 direction, float speed, bool isPlayers)
{
	if (isPlayers)
		ResourceManager::instance->LoadSound("shoot_player")->Play(true);
	else ResourceManager::instance->LoadSound("shoot")->Play(true);

	float angle = -atan2(direction.z, direction.x);

	Projectile* p = (Projectile*)SceneManager::instance->CreatePrefab("projectile");
	p->rotation = Vector3(0, angle, 0);
	p->newRotation = Vector3(0, angle, 0);
	p->position = position;
	p->trajectoryData = TrajectoryData();
	p->isPlayers = isPlayers;

	if (isPlayers)
		p->diffuseColor = Vector4(.98f, .62f, .06f, 1) * 1.1f;
	else p->diffuseColor = Vector4(.35f, .44f, .96f, 1) * 1.3f;
	p->diffuseColor.w = 1;

	Trajectory* traj = new Trajectory();
	traj->CreateLine(position, position + direction * 200, speed, 1, false);
	p->trajectoryData.Init(traj, 1);

	FireObject* f = (FireObject*)SceneManager::instance->CreatePrefab("projectileTrail");
	f->following = p;

	p->trail = f;

	SceneManager::instance->objects[p->id] = p;
	SceneManager::instance->objects[f->id] = f;

	if (isPlayers)
		playerProjectiles[p->id] = p;
	else 
		enemyProjectiles[p->id] = p;
}

void BulletGamemode::SpawnEnemy(Vector3 position, float angle, float speed, const std::string& prefabSuffix)
{
	Vector3 direction = Vector3(cosf(angle), 0, sinf(angle));
	Enemy* e = (Enemy*)SceneManager::instance->CreatePrefab("enemy" + prefabSuffix);
	e->rotation = Vector3(0, -angle + PI, 0);
	e->newRotation = Vector3(0, -angle + PI, 0);
	e->position = position;
	e->trajectoryData = TrajectoryData();
	Trajectory* traj = new Trajectory();
	traj->CreateLine(position, position + direction * 200, speed, 1, false);
	e->trajectoryData.Init(traj, 1);

	SceneManager::instance->objects[e->id] = e;

	enemies[e->id] = e;
}

bool BulletGamemode::InsidePlayerMap(Vector3 point)
{
	return -playerMapSize.x <= point.x && point.x <= playerMapSize.x &&
		-playerMapSize.y <= point.z && point.z <= playerMapSize.y;
}

bool BulletGamemode::OutsideMap(Vector3 point)
{
	return -outsideMapSize.x > point.x || point.x > outsideMapSize.x ||
		-outsideMapSize.y > point.z || point.z > outsideMapSize.y;
}

bool BulletGamemode::PassedBelowMap(Vector3 point)
{
	return point.z < outsideMapSize.y;
}
