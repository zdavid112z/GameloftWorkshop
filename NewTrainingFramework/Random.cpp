#include "stdafx.h"
#include "Random.h"

std::mt19937 Random::randEngine;

void Random::Init()
{
	std::random_device rd;
	randEngine = std::mt19937(rd());
}

int Random::Int(int lower, int upper)
{
	std::uniform_int_distribution<int> uid(lower, upper);
	return uid(randEngine);
}

float Random::Float(float lower, float upper)
{
	std::uniform_real_distribution<float> urd(lower, upper);
	return urd(randEngine);
}