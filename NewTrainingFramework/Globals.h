#pragma once

#include <unordered_map>
#include "../Utilities/Math.h"

enum class VariableType
{
	INT, FLOAT, STRING, VEC2, VEC3, VEC4
};

struct GlobalData
{
	void* address;
	VariableType type;

	static GlobalData CreateInt(int* value) { GlobalData data; data.address = value; data.type = VariableType::INT; return data; }
	static GlobalData CreateFloat(float* value) { GlobalData data; data.address = value; data.type = VariableType::FLOAT; return data; }
	static GlobalData CreateString(std::string* value) { GlobalData data; data.address = value; data.type = VariableType::STRING; return data; }
	static GlobalData CreateVec2(Vector2* value) { GlobalData data; data.address = value; data.type = VariableType::VEC2; return data; }
	static GlobalData CreateVec3(Vector3* value) { GlobalData data; data.address = value; data.type = VariableType::VEC3; return data; }
	static GlobalData CreateVec4(Vector4* value) { GlobalData data; data.address = value; data.type = VariableType::VEC4; return data; }
};

class Globals
{
public:
	static int screenWidth;
	static int screenHeight;
	static std::string windowTitle;
	static Vector4 clearColor;

	static void AddGlobal(const std::string& name, GlobalData data);
	static void LoadGlobals();
private:
	static std::unordered_map<std::string, GlobalData> globals;
};
