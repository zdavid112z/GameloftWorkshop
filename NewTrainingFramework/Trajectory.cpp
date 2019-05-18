#include "stdafx.h"
#include "Trajectory.h"

Trajectory::~Trajectory()
{
	if (start != nullptr)
	{
		TrajectoryPointNode *node, *next;
		node = start;
		do
		{
			next = node->next;
			delete node;
			node = next;
		} while (node != start);
	}
}

void Trajectory::CreateLine(Vector3 s, Vector3 e, float speed, int reps, bool alt)
{
	isPointList = true;
	this->count = reps;
	this->speed = speed;
	if (alt)
	{
		start = new TrajectoryPointNode(s, true, false);
		start->next = new TrajectoryPointNode(e, true, false);
		start->next->next = start;
	}
	else
	{
		start = new TrajectoryPointNode(s, false, true);
		start->next = new TrajectoryPointNode(e, false, false);
		start->next->next = start;
	}
	CalculateSpeed();
}

void Trajectory::CreateLineStrip(Vector3* points, int numPoints, float speed, int count, bool alternate, bool loop)
{
	isPointList = true;
	this->speed = speed;
	this->count = count;
	TrajectoryPointNode** node = &start, *prev;
	for (int i = 0; i < numPoints; i++)
	{
		*node = new TrajectoryPointNode(points[i], false, false);
		prev = *node;
		node = &(*node)->next;
	}
	if (alternate)
	{
		prev->rotateOnFinish = true;
		for (int i = numPoints - 2; i >= 1; i--)
		{
			*node = new TrajectoryPointNode(points[i], false, false);
			node = &(*node)->next;
		}
		*node = start;
		start->rotateOnFinish = true;
	}
	else
	{
		*node = start;
		start->teleportOnStart = true;
	}
	CalculateSpeed();
}

void Trajectory::CreateLineLoop(Vector3* points, int numPoints, float speed, int count)
{
	isPointList = true;
	this->count = count;
	this->speed = speed;
	TrajectoryPointNode** node = &start, *prev;
	for (int i = 0; i < numPoints; i++)
	{
		*node = new TrajectoryPointNode(points[i], false, false);
		prev = *node;
		node = &(*node)->next;
	}
	*node = start;
	CalculateSpeed();
}

void Trajectory::CreateCircle(Vector3 center, float radius, Vector3 rotation, float speed, int count)
{
	isPointList = false;
	circleCenter = center;
	circleRadius = radius;
	this->count = count;
	this->speed = speed * PI / 180.f;
	Matrix rx, ry, rz;
	rotation = rotation * PI / 180.f;
	rx.SetRotationX(rotation.x);
	ry.SetRotationX(rotation.y);
	rz.SetRotationX(rotation.z);
	circleRotation = rx * ry * rz;
}

void Trajectory::CalculateSpeed()
{
	TrajectoryPointNode *node, *next;
	node = start;
	next = start->next;
	do
	{
		Vector3 ab = next->position - node->position;
		float dist = ab.Length();
		node->speed = speed / dist;
		node = node->next;
		next = node->next;
	} while (node != start);
}

void TrajectoryData::Init(Trajectory* t, float speed)
{
	if (t->isPointList)
		currentNode = t->start;
	this->speed = speed;
	trajectory = t;
	progress = 0;
	reps = t->count;
	finished = false;
}

Vector3 TrajectoryData::Update(float deltaTime, bool& rotate180)
{
	if (finished && !trajectory->isPointList)
		return lastPosition;
	else if (finished)
		return trajectory->start->position;
	rotate180 = false;
	if (trajectory->isPointList)
	{
		progress += currentNode->speed * deltaTime * speed;
		if (progress >= 1)
		{
			float distAB = (currentNode->position - currentNode->next->position).Length();
			if (currentNode->next->rotateOnFinish)
			{
				rotate180 = true;
			}
			if (currentNode->next->next->teleportOnStart)
			{
				currentNode = currentNode->next;
				if (currentNode == trajectory->start)
					reps--;
			}
			currentNode = currentNode->next;
			if (currentNode == trajectory->start)
				reps--;
			if (reps <= 0)
				finished = true;
			float remainder = progress - int(progress);
			float distBC = (currentNode->next->position - currentNode->position).Length();
			progress--;
			progress = progress * distAB / distBC;
		}
		lastPosition = currentNode->position.Lerp(currentNode->next->position, progress);
		return lastPosition;
	}
	else
	{
		progress += deltaTime * speed * trajectory->speed;
		if (progress / (2.f * PI) >= reps)
		{
			finished = true;
			Vector4 local(1, 0, 0, 0);
			local = trajectory->circleRotation * local;
			lastPosition = trajectory->circleCenter + Vector3(local) * trajectory->circleRadius;
			return lastPosition;
		}
		else
		{
			float c = cos(progress);
			float s = sin(progress);
			Vector4 local(c, 0, s, 0);
			local = trajectory->circleRotation * local;
			lastPosition = trajectory->circleCenter + Vector3(local) * trajectory->circleRadius;
			return lastPosition;
		}
	}
}

bool TrajectoryData::Finished()
{
	return finished;
}