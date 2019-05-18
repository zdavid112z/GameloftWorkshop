#pragma once

#include "stdafx.h"
#include "../Utilities/utilities.h"
#include "Projectile.h"
#include "WaveEvent.h"

class BulletGamemode;

class Wave
{
public:
	void AddEvent(WaveEvent e);
	void Start(ESContext* context, float delta, BulletGamemode* gamemode);
	// true if has events left to process
	bool Update(ESContext* context, float delta);
private:
	void SpawnEnemy(WaveEventSpawn* e, int index);
	Vector3 PointInCircle(Vector3 center, float radius);
	Vector3 PointAboveMap(float width);

	void GenerateLinePositions(WaveEventSpawn* e);
private:
	BulletGamemode* gamemode;
	int lastDiv;
	float timer;
	int currentEvent;
	std::vector<WaveEvent> events;

	std::vector<float> linePositions;
};