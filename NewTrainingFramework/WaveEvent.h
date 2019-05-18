#pragma once

#include "Enemy.h"

enum class WaveEventType
{
	WAIT,
	SPAWN_ENEMY
};

enum class WaveEventSpawnTrajectoryType
{
	STRAIGHT_DOWN,
	THROUGH_CIRCLE,
	HOMING_CIRCLE,
};

enum class WaveEventSpawnInitType
{
	ABOVE_MAP,
	POINT_CIRCLE
};

struct WaveEventWait
{
	float amount;
};

struct WaveEventSpawn
{
	WaveEventSpawnTrajectoryType trajectoryType;
	WaveEventSpawnInitType initType;

	int numRepeats;
	int numEnemies;
	float delayBetweenSpawns;

	std::string enemyType;
	float enemySpeed = 1;

	float spawnLineWidth;
	float minSpacingFactor;
	float paddingFactor;
	Vector3 spawnCenter;
	float spawnRadius;

	float deviationAngle;
	Vector3 targetCircleCenter;
	float targetCircleRadius;
};

struct WaveEvent
{
	//WaveEvent() {}
	WaveEventType type;
	//union
	//{
		WaveEventWait wait;
		WaveEventSpawn spawn;
	//};
};
