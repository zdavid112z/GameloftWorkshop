#include "stdafx.h"
#include "ResourceManager.h"
#include "XMLUtils.h"

ResourceManager* ResourceManager::instance = nullptr;

void ResourceManager::Init()
{
	instance = new ResourceManager();
	instance->Load();
}

void ResourceManager::Destroy()
{
	delete instance;
}

ResourceManager::ResourceManager()
{
	FMOD::System_Create(&soundSystem);
	soundSystem->init(100, FMOD_INIT_NORMAL, 0);
	soundSystem->set3DSettings(1.0, 100.0, 1.0f);
}

ResourceManager::~ResourceManager()
{
	for (auto it = models.begin(); it != models.end(); it++)
		if (it->second.handles != 0)
			delete it->second.model;
	for (auto it = shaders.begin(); it != shaders.end(); it++)
		if (it->second.handles != 0)
			delete it->second.shader;
	for (auto it = textures.begin(); it != textures.end(); it++)
		if (it->second.handles != 0)
			delete it->second.texture;
	for (auto it = sounds.begin(); it != sounds.end(); it++)
		if (it->second.handles != 0)
			delete it->second.sound;
}

void ResourceManager::Load()
{
	rapidxml::file<> fin("resourceManager.xml");
	rapidxml::xml_document<> doc;
	doc.parse<0>(fin.data());
	
	rapidxml::xml_node<>* root = doc.first_node();
	if(root->first_node("models"))
	for (auto node = root->first_node("models")->first_node(); node; node = node->next_sibling())
	{
		if (strncmp(node->name(), "model", node->name_size()) == 0)
			LoadModel(node, XMLUtils::ParseIdFromAttribute(node));
	}
	if (root->first_node("shaders"))
	for (auto node = root->first_node("shaders")->first_node(); node; node = node->next_sibling())
	{
		if (strncmp(node->name(), "shader", node->name_size()) == 0)
			LoadShader(node, XMLUtils::ParseIdFromAttribute(node));
	}
	if (root->first_node("textures"))
	for (auto node = root->first_node("textures")->first_node(); node; node = node->next_sibling())
	{
		if (strncmp(node->name(), "texture", node->name_size()) == 0)
			LoadTexture(node, XMLUtils::ParseIdFromAttribute(node));
	}
	if (root->first_node("sounds"))
	for (auto node = root->first_node("sounds")->first_node(); node; node = node->next_sibling())
	{
		if (strncmp(node->name(), "sound", node->name_size()) == 0)
			LoadSound(node, XMLUtils::ParseIdFromAttribute(node));
	}
}

void ResourceManager::LoadModel(rapidxml::xml_node<>* node, uint32 id)
{
	ModelResource res;
	res.id = id;
	auto path = node->first_node("path");
	res.path = XMLUtils::ParseString(path, "");
	res.path = "../" + Utils::Trim(res.path);
	std::string type = XMLUtils::ParseString(node->first_node("type"), "loaded");
	if (type == "loaded")
		res.type = ModelType::LOADED;
	else if (type == "cube")
		res.type = ModelType::CUBE;
	else if (type == "axis")
	{
		res.type = ModelType::AXIS;
		res.thickness = XMLUtils::ParseFloat(node->first_node("thickness"), 5);
		res.lineLength = XMLUtils::ParseFloat(node->first_node("lineLength"), 12);
		res.headLength = XMLUtils::ParseFloat(node->first_node("headLength"), 6);
		res.headOffset = XMLUtils::ParseFloat(node->first_node("headOffset"), 1.5f);
		res.quality = XMLUtils::ParseInt(node->first_node("quality"), 8);
	}
	else if (type == "screenQuad")
		res.type = ModelType::SCREEN_QUAD;
	if (models.find(id) != models.end())
	{
		// TODO: Warning: two objects with the same id
		// Should change hash seed
		// Or the ids in the file idk
	}
	models[id] = res;
}

void ResourceManager::LoadShader(rapidxml::xml_node<>* node, uint32 id)
{
	ShaderResource res;
	res.id = id;
	auto vpath = node->first_node("vs");
	res.vertexPath = std::string(vpath->value(), vpath->value() + vpath->value_size());
	auto fpath = node->first_node("fs");
	res.fragmentPath = std::string(fpath->value(), fpath->value() + fpath->value_size());
	res.vertexPath = "../" + Utils::Trim(res.vertexPath);
	res.fragmentPath = "../" + Utils::Trim(res.fragmentPath);
	shaders[id] = res;
}

void ResourceManager::LoadTexture(rapidxml::xml_node<>* node, uint32 id)
{
	TextureResource res;
	res.id = id;
	auto path = node->first_node("path");
	if(path)
		res.path = std::string(path->value(), path->value() + path->value_size());
	else res.path = "";
	auto tatr = node->first_attribute("type");
	if (tatr != nullptr)
	{
		std::string type = std::string(tatr->value(), tatr->value() + tatr->value_size());
		if (type == "cube")
			res.isCube = true;
		else if (type == "2d")
			res.isCube = false;
		else
		{
			res.isCube = false;
			// TODO: Unknown texture type
		}
	}
	else res.isCube = true;

	res.minFilter = XMLUtils::ParseString(node->first_node("min_filter"), "LINEAR");
	res.magFilter = XMLUtils::ParseString(node->first_node("mag_filter"), "LINEAR");
	res.wraps = XMLUtils::ParseString(node->first_node("wrap_s"), "REPEAT");
	res.wrapt = XMLUtils::ParseString(node->first_node("wrap_t"), "REPEAT");
	res.wrapr = XMLUtils::ParseString(node->first_node("wrap_r"), "REPEAT");
	res.width = XMLUtils::ParseUint(node->first_node("width"), 0);
	res.height = XMLUtils::ParseUint(node->first_node("height"), 0);
	std::string type = XMLUtils::ParseString(node->first_node("type"), "loaded");
	if (type == "loaded")
		res.type = TextureType::LOADED;
	else if (type == "depth")
		res.type = TextureType::DEPTH;
	else if (type == "rendertarget_rgb")
		res.type = TextureType::RENDERTARGET_RGB;
	else if (type == "rendertarget_rgba5551")
		res.type = TextureType::RENDERTARGET_RGBA5551;
	else if (type == "rendertarget_rgba4444")
		res.type = TextureType::RENDERTARGET_RGBA4444;
	res.path = "../" + Utils::Trim(res.path);
	textures[id] = res;
}

void ResourceManager::LoadSound(rapidxml::xml_node<>* node, uint32 id)
{
	SoundResource res;
	res.path = "../" + XMLUtils::ParseString(node->first_node("path"), "");
	res.is3d = node->first_node("is3d") != nullptr;
	res.loop = node->first_node("loop") != nullptr;
	res.id = id;
	res.distances3d = XMLUtils::ParseVector2(node->first_node("distances3d"), Vector2(5, 5000));
	sounds[id] = res;
}
