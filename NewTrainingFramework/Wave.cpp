#include "stdafx.h"
#include "Wave.h"
#include "bulletGamemode.h"
#include "Random.h"

void Wave::AddEvent(WaveEvent e)
{
	events.push_back(e);
}

void Wave::Start(ESContext* context, float delta, BulletGamemode* gamemode)
{
	this->gamemode = gamemode;
	currentEvent = 0;
	lastDiv = -1;
	timer = -delta;
}

// true if has events left to process
bool Wave::Update(ESContext* context, float delta)
{
	timer += delta;
	WaveEvent* e = &events[currentEvent];
	switch (e->type)
	{
	case WaveEventType::WAIT:
		if (timer >= e->wait.amount)
		{
			currentEvent++;
			timer = 0;
			lastDiv = -1;
		}
		break;
	case WaveEventType::SPAWN_ENEMY:
		int currentDiv;
		if (e->spawn.delayBetweenSpawns < 1e-4)
			currentDiv = 1000;
		else currentDiv = int(timer / e->spawn.delayBetweenSpawns);
		for (int i = lastDiv; i < std::min(currentDiv, e->spawn.numRepeats); i++)
		{
			if (e->spawn.initType == WaveEventSpawnInitType::ABOVE_MAP)
				GenerateLinePositions(&e->spawn);
			for(int j = 0; j < e->spawn.numEnemies; j++)
				SpawnEnemy(&e->spawn, j);
			lastDiv = currentDiv;
		}
		if (currentDiv >= e->spawn.numRepeats)
		{
			currentEvent++;
			timer = 0;
			lastDiv = -1;
		}
		break;
	}
	if (currentEvent >= int(events.size()))
		return false;
	return true;
}

void Wave::GenerateLinePositions(WaveEventSpawn* e)
{
	linePositions.clear();
	linePositions.push_back(Random::Float(0.001f, e->paddingFactor));
	for(int i = 1; i < e->numEnemies; i++)
		linePositions.push_back(Random::Float(e->minSpacingFactor, 1.f));
	linePositions.push_back(Random::Float(0.001f, e->paddingFactor));

	float sum = 0;
	for (float dist : linePositions)
		sum += dist;
	for (float& dist : linePositions)
	 	dist *= 2 * e->spawnLineWidth / sum;
	for (int i = 1; i < int(linePositions.size()); i++)
		linePositions[i] += linePositions[i - 1];
	for (float& dist : linePositions)
		dist -= e->spawnLineWidth;
}

Vector3 Wave::PointInCircle(Vector3 center, float radius)
{
	float r = Random::Float(0, radius);
	float angle = Random::Float(0, 2 * PI);
	return Vector3(std::cos(angle) * r + center.x, center.y, std::sin(angle) * 2 + center.z);
}

Vector3 Wave::PointAboveMap(float width)
{
	return Vector3(Random::Float(-width, width), 0, -gamemode->outsideMapSize.y);
}

void Wave::SpawnEnemy(WaveEventSpawn* e, int index)
{
	bool useAngle;
	float angle;
	Vector3 startPos, throughPos;
	switch (e->initType)
	{
	case WaveEventSpawnInitType::ABOVE_MAP:
		startPos = Vector3(linePositions[index], 0, -gamemode->outsideMapSize.y);
		break;
	case WaveEventSpawnInitType::POINT_CIRCLE:
		startPos = PointInCircle(e->spawnCenter, e->spawnRadius);
		break;
	}

	switch (e->trajectoryType)
	{
	case WaveEventSpawnTrajectoryType::STRAIGHT_DOWN:
		useAngle = true;
		angle = Random::Float(-e->deviationAngle, e->deviationAngle) + PI / 2;
		break;
	case WaveEventSpawnTrajectoryType::THROUGH_CIRCLE:
		useAngle = false;
		throughPos = PointInCircle(e->targetCircleCenter, e->targetCircleRadius);
		break;
	case WaveEventSpawnTrajectoryType::HOMING_CIRCLE:
		useAngle = false;
		throughPos = PointInCircle(gamemode->player->position, e->targetCircleRadius);
		break;
	}

	if (!useAngle)
	{
		Vector3 direction = (throughPos - startPos).Normalize();
		angle = atan2(direction.z, direction.x);
	}

	gamemode->SpawnEnemy(startPos, angle, e->enemySpeed, e->enemyType);
}
