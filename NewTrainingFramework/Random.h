#pragma once

#include <random>

class Random
{
public:
	static void Init();

	static int Int(int lower, int upper);
	static float Float(float lower, float upper);
	
	template <typename T>
	static int Index(const std::vector<T>& v)
	{
		if (v.empty())
			return -1;
		return Int(0, v.size() - 1);
	}

	template <typename T>
	static int Index(const std::vector<T>& v, int notEqualToThis)
	{
		if (notEqualToThis < 0 || notEqualToThis >= v.size())
			return Index(v);
		if (v.size() < 2)
			return 0;
		int val = Int(0, v.size() - 2);
		if (val >= notEqualToThis)
			return val + 1;
		return val;
	}
private:
	static std::mt19937 randEngine;
};