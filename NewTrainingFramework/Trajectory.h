#pragma once

#include "../Utilities/Math.h"

struct TrajectoryPointNode
{
	TrajectoryPointNode() {}
	TrajectoryPointNode(Vector3 pos, bool rotOnFinish, bool teleportOnStart) :
		position(pos), rotateOnFinish(rotOnFinish), teleportOnStart(teleportOnStart) {}

	bool teleportOnStart = false;
	bool rotateOnFinish = false;
	float speed;
	Vector3 position;
	TrajectoryPointNode* next;
};

class Trajectory
{
public:
	~Trajectory();

	int count;
	bool isPointList;
	bool alternate;
	TrajectoryPointNode* start = nullptr;
	Vector3 circleCenter;
	float circleRadius;
	Matrix circleRotation;
	float speed;

	void CreateLine(Vector3 start, Vector3 end, float speed, int count, bool alternate);
	void CreateLineStrip(Vector3* points, int numPoints, float speed, int count, bool alternate, bool loop);
	void CreateLineLoop(Vector3* points, int numPoints, float speed, int count);
	void CreateCircle(Vector3 center, float radius, Vector3 rotation, float speed, int count);
	void CalculateSpeed();
};

class TrajectoryData
{
public:
	void Init(Trajectory* t, float speed = 1);

	Vector3 Update(float deltaTime, bool& rotate180);
	bool Finished();

	Vector3 lastPosition;
	Trajectory* trajectory = nullptr;
	TrajectoryPointNode* currentNode;
	float speed;
	float progress;
	int reps;
	bool finished = true;
};
