#pragma once

#include "debugGamemode.h"
#include "Wave.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

class Player;
class Projectile;
class Enemy;

class BulletGamemode : public DebugGamemode
{
public:
	BulletGamemode();
	~BulletGamemode();
	void Update(ESContext* context, float delta) override;
	void Draw(ESContext* context) override;
	void Key(ESContext *esContext, unsigned char key, bool bIsPressed) override;

	void SpawnProjectile(Vector3 position, Vector3 direction, float speed, bool isPlayers);
	void SpawnEnemy(Vector3 position, float angle, float speed, const std::string& prefabSuffix);

	bool InsidePlayerMap(Vector3 point);
	bool OutsideMap(Vector3 point);
	bool PassedBelowMap(Vector3 point);

	void Parse(rapidxml::xml_node<>* node);
public:
	Wave ParseWave(rapidxml::xml_node<>* node);
	WaveEvent ParseWaveEvent(rapidxml::xml_node<>* node);
	void UpdateLogic(ESContext* context, float delta);

	Vector2 playerMapSize;
	Vector2 outsideMapSize;

	Player* player;
	std::unordered_map<uint32, Enemy*> enemies;
	std::unordered_map<uint32, Projectile*> playerProjectiles;
	std::unordered_map<uint32, Projectile*> enemyProjectiles;

	std::vector<Wave> waves;
	bool wavesRandom;

	int lastWaveIndex = -1;
	int currentWaveIndex = -1;
	float waveTimer = 0;
	int enemiesSpawned = 0;
	float difficulty;
	float currentDifficulty = 1.f;
	float maxDifficulty;
};