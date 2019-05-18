#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include "ResourceManager.h"
#include "../Utilities/Math.h"
#include "XMLUtils.h"

#define TYPE_MAX_SIZE 4096

enum class ReflectedType
{
	UNKNOWN,
	CUSTOM,
	INT,
	BOOL,
	FLOAT,
	UINT,
	VEC2,
	VEC3,
	VEC4,
	SHADER_PTR,
	MODEL_PTR,
	TEXTURE_ARRAY
};

struct Reflected
{
	std::string name;
	size_t offset;
	ReflectedType type;
	std::string tname;
	size_t size;
	bool hasDefaultValue;
	byte defaultValue[TYPE_MAX_SIZE];
	std::function<void(byte* object, byte* reflected)> onLoad = nullptr;
};

struct ObjectReflection
{
	std::vector<Reflected> reflected;
	std::function<void(byte* object, rapidxml::xml_node<>* node)> onLoad = nullptr;
	size_t size;
};

class Reflection
{
public:
	static std::unordered_map<std::string, ObjectReflection > objects;
public:
	template <typename T> static ReflectedType GetType() { return ReflectedType::UNKNOWN; }
	template <> static ReflectedType GetType<bool>() { return ReflectedType::BOOL; }
	template <> static ReflectedType GetType<float>() { return ReflectedType::FLOAT; }
	template <> static ReflectedType GetType<int>() { return ReflectedType::INT; }
	template <> static ReflectedType GetType<unsigned int>() { return ReflectedType::UINT; }
	template <> static ReflectedType GetType<Vector2>() { return ReflectedType::VEC2; }
	template <> static ReflectedType GetType<Vector3>() { return ReflectedType::VEC3; }
	template <> static ReflectedType GetType<Vector4>() { return ReflectedType::VEC4; }
	template <> static ReflectedType GetType<Shader*>() { return ReflectedType::SHADER_PTR; }
	template <> static ReflectedType GetType<Model*>() { return ReflectedType::MODEL_PTR; }
	template <> static ReflectedType GetType<std::vector<Texture*> >() { return ReflectedType::TEXTURE_ARRAY; }

	static size_t GetSize(Reflected v)
	{
		if (v.type == ReflectedType::UNKNOWN)
		{
			auto it = objects.find(v.tname);
			if(it == objects.end())
				return 0;
			return it->second.size;
		}
		return v.size;
	}

	static void ReadFromNode(Reflected t, rapidxml::xml_node<>* node, byte* result)
	{
		switch (t.type)
		{
		case ReflectedType::BOOL:
		{
			bool b = XMLUtils::ParseBool(node, *(bool*)result);
			memcpy(result, &b, sizeof(bool));
		}
			break;
		case ReflectedType::INT:
		{
			int i = XMLUtils::ParseInt(node, *(int*)result);
			memcpy(result, &i, sizeof(int));
		}
			break;
		case ReflectedType::UINT:
		{
			unsigned int ui = XMLUtils::ParseUint(node, *(unsigned int*)result);
			memcpy(result, &ui, sizeof(unsigned int));
		}
			break;
		case ReflectedType::FLOAT:
		{
			float f = XMLUtils::ParseFloat(node, *(float*)result);
			memcpy(result, &f, sizeof(float));
		}
			break;
		case ReflectedType::VEC2:
		{
			Vector2 v2 = XMLUtils::ParseVector2(node, *(Vector2*)result);
			memcpy(result, &v2, sizeof(Vector2));
		}
			break;
		case ReflectedType::VEC3:
		{
			Vector3 v3 = XMLUtils::ParseVector3(node, *(Vector3*)result);
			memcpy(result, &v3, sizeof(Vector3));
		}
			break;
		case ReflectedType::VEC4:
		{
			Vector4 v4 = XMLUtils::ParseVector4(node, *(Vector4*)result);
			memcpy(result, &v4, sizeof(Vector4));
		}
			break;
		case ReflectedType::SHADER_PTR:
		{
			uint32 shaderid = XMLUtils::ParseIdFromValue(node, -1);
			Shader* s = nullptr;
			if (shaderid != -1)
			{
				s = ResourceManager::instance->LoadShader(shaderid);
				memcpy(result, &s, sizeof(Shader*));
			}
		}
			break;
		case ReflectedType::MODEL_PTR:
		{
			uint32 modelid = XMLUtils::ParseIdFromValue(node, -1);
			Model* s = nullptr;
			if (modelid != -1)
			{
				s = ResourceManager::instance->LoadModel(modelid);
				memcpy(result, &s, sizeof(Model*));
			}
		}
			break;
		case ReflectedType::TEXTURE_ARRAY:
		{
			int i = 0;
			std::vector<Texture*> textures;
			for (auto tex = node->first_node("texture"); tex; tex = tex->next_sibling("texture"))
			{
				uint32 texid = XMLUtils::ParseIdFromAttribute(tex, -1);
				textures.push_back(ResourceManager::instance->LoadTexture(texid));
				ASSERT(textures.back(), "Texture with id %u (index %d) not found and required by an object!", texid, i);
				i++;
			}
			std::vector<Texture*>* t = (std::vector<Texture*>*)result;
			*t = textures;
		}
			break;
		case ReflectedType::UNKNOWN:
		{
			if(objects.find(t.tname) != objects.end())
				LoadVariables(result, node, t.tname);
		}
			break;
		}
	}

	template <typename P, typename T>
	static void Add(const std::string& name, size_t offset, const T& defaultValue, const T& unused, std::function<void(byte* object, byte* reflected)> onLoad = nullptr, bool hasDefaultValue = true)
	{
		Reflected v;
		v.name = name;
		v.offset = offset;
		v.type = GetType<T>();
		v.onLoad = onLoad;
		v.size = sizeof(T);
		v.hasDefaultValue = hasDefaultValue;
		if(hasDefaultValue)
			memcpy(v.defaultValue, &defaultValue, GetSize(v));
		std::string pname = typeid(P).name();
		auto it = objects.find(pname);
		if (it != objects.end())
			it->second.reflected.push_back(v);
		else
		{
			ObjectReflection r;
			r.onLoad = nullptr;
			r.size = sizeof(P);
			r.reflected.push_back(v);
			objects[pname] = r;
		}
	}

	template <typename P>
	static void SetOnLoadFunc(std::function<void(byte* object, rapidxml::xml_node<>* node)> onLoad)
	{
		std::string pname = typeid(P).name();
		auto it = objects.find(pname);
		if (it != objects.end())
			it->second.onLoad = onLoad;
		else
		{
			ObjectReflection r;
			r.onLoad = onLoad;
			r.size = sizeof(P);
			objects[pname] = r;
		}
	}

	template <typename T>
	static void LoadVariables(T* obj, rapidxml::xml_node<>* node)
	{
		std::string pname = typeid(T).name();
		LoadVariables((byte*)obj, node, pname);
	}

	static void LoadVariables(byte* obj, rapidxml::xml_node<>* node, const std::string& pname)
	{
		auto it = objects.find(pname);
		if (it == objects.end())
			return;
		for (const Reflected& v : it->second.reflected)
		{
			auto child = node->first_node(v.name.c_str());
			byte* data = ((byte*)obj) + v.offset;
			if(v.hasDefaultValue)
				memcpy(data, v.defaultValue, GetSize(v));
			if (child != nullptr)
				ReadFromNode(v, child, data);
			if (v.onLoad)
				v.onLoad((byte*)obj, ((byte*)obj) + v.offset);
		}
		if (it->second.onLoad)
			it->second.onLoad((byte*)obj, node);
	}

	static void Init();

};

#define REFLECT_NODEFAULT(type, member) Reflection::Add<type>(#member, offsetof(type, member), (((type*)nullptr)->member), (((type*)nullptr)->member) , nullptr, false)
#define REFLECT(type, member, defaultValue) Reflection::Add<type>(#member, offsetof(type, member), defaultValue, (((type*)nullptr)->member) , nullptr, true)
// element on load
#define REFLECT_EOL(type, member, defaultValue, onLoad) Reflection::Add<type>(#member, offsetof(type, member), defaultValue, (((type*)nullptr)->member) , onLoad, true)
// element on load
#define REFLECT_EOL_NODEFAULT(type, member, onLoad) Reflection::Add<type>(#member, offsetof(type, member), (((type*)nullptr)->member), (((type*)nullptr)->member) , onLoad, false)
// object on load
#define REFLECT_OOL(type, onLoad) Reflection::SetOnLoadFunc<type>(onLoad)
