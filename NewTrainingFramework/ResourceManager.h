#pragma once

#include "stdafx.h"
#include <string>
#include <unordered_map>
#include <functional>
#include "utils.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "Model.h"
#include "Texture.h"
#include "Sound.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "fmod.hpp"

class Resource
{
public:
	uint32 id;
	uint handles = 0;
};

enum class ModelType
{
	LOADED,
	CUBE,
	AXIS,
	SCREEN_QUAD
};

class ModelResource : public Resource
{
public:
	union
	{
		Model* model;
		void* object;
	};
	ModelType type;
	std::string path;
	// Used for the axis
	float thickness, lineLength, headLength, headOffset;
	int quality;
};

class ShaderResource : public Resource
{
public:
	union 
	{
		Shader* shader; 
		void* object;
	};
	std::string vertexPath, fragmentPath;
};

class TextureResource : public Resource
{
public:
	union
	{
		Texture* texture;
		void* object;
	};
	bool isCube;
	TextureType type;
	uint width, height;
	std::string path;
	std::string minFilter, magFilter, wraps, wrapt, wrapr;
};

class SoundResource : public Resource
{
public:
	union
	{
		Sound* sound;
		void* object;
	};
	std::string path;
	bool loop;
	bool is3d;
	Vector2 distances3d;
};

class ResourceManager
{
public:
	static ResourceManager* instance;
	static void Init();
	static void Destroy();
private:
	ResourceManager();
	~ResourceManager();
	void Load();
public:
	Model* LoadModel(uint32 id, bool weak = false) { return LoadResource<Model>(id, models, weak); }
	void FreeModel(uint32 id) { FreeResource<Model>(id, models); }
	Model* LoadModel(const std::string& name, bool weak = false) { return LoadModel(Utils::Hash(name), weak); }
	void FreeModel(const std::string& name) { FreeModel(Utils::Hash(name)); }

	Shader* LoadShader(uint32 id, bool weak = false) { return LoadResource<Shader>(id, shaders, weak); }
	void FreeShader(uint32 id) { FreeResource<Shader>(id, shaders); }
	Shader* LoadShader(const std::string& name, bool weak = false) { return LoadShader(Utils::Hash(name), weak); }
	void FreeShader(const std::string& name) { FreeShader(Utils::Hash(name)); }

	Texture* LoadTexture(uint32 id, bool weak = false) { return LoadResource<Texture>(id, textures, weak); }
	void FreeTexture(uint32 id) { FreeResource<Texture>(id, textures); }
	Texture* LoadTexture(const std::string& name, bool weak = false) { return LoadTexture(Utils::Hash(name), weak); }
	void FreeTexture(const std::string& name) { FreeTexture(Utils::Hash(name)); }

	Sound* LoadSound(uint32 id, bool weak = false) { return LoadResource<Sound>(id, sounds, weak); }
	void FreeSound(uint32 id) { FreeResource<Sound>(id, sounds); }
	Sound* LoadSound(const std::string& name, bool weak = false) { return LoadSound(Utils::Hash(name), weak); }
	void FreeSound(const std::string& name) { FreeSound(Utils::Hash(name)); }

	FMOD::System* GetSoundSystem() { return soundSystem; }
private:
	template <typename T>
	using ResourcesMap = std::unordered_map<uint32, T>;

	void LoadModel(rapidxml::xml_node<>* doc, uint32 id);
	void LoadShader(rapidxml::xml_node<>* doc, uint32 id);
	void LoadTexture(rapidxml::xml_node<>* doc, uint32 id);
	void LoadSound(rapidxml::xml_node<>* doc, uint32 id);

	template <typename T, typename TT>
	T* LoadResource(uint32 id, ResourcesMap<TT>& map, bool weak)
	{
		auto it = map.find(id);
		if (it == map.end())
			return nullptr;
		TT& res = it->second;
		if (res.handles == 0 && !weak)
			res.object = (void*)(new T(&res));
		if(!weak)
			res.handles++;
		return (T*) res.object;
	}

	template <typename T, typename TT>
	void FreeResource(uint32 id, ResourcesMap<TT>& map)
	{
		auto it = map.find(id);
		if (it == map.end())
			return;
		TT& res = it->second;
		res.handles--;
		if (res.handles == 0)
			delete (T*)res.object;
	}
private:
	FMOD::System* soundSystem;
	ResourcesMap<ModelResource> models;
	ResourcesMap<ShaderResource> shaders;
	ResourcesMap<TextureResource> textures;
	ResourcesMap<SoundResource> sounds;
};
