#pragma once

#include <unordered_map>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "SceneObject.h"
#include "Camera.h"
#include "Trajectory.h"
#include "Light.h"
#include "Postprocess.h"
#include "fmod.hpp"
#include "Input.h"

class Gamemode;

class SceneManager
{
public:
	static SceneManager* instance;
	static void Init(ESContext* esContext);
	static void Destroy();
private:
	SceneManager(ESContext* esContext);
	~SceneManager();
public:
	void Delete();
	void Reload(const std::string& scenePath = "sceneManager.xml");
	void Update(ESContext* context, float delta);
	void Draw(ESContext* context);
	void Key(ESContext *esContext, unsigned char key, bool bIsPressed);

	SceneObject* CreatePrefab(const std::string& name);

	template <typename T>
	static void RemoveIfDead(T& map, bool deleteObjects)
	{
		for (auto it = map.begin(); it != map.end();)
		{
			if (it->second->dead)
			{
				if (deleteObjects)
					delete it->second;
				it = map.erase(it);
			}
			else it++;
		}
	}
private:
	SceneObject* ParseSceneObject(rapidxml::xml_node<>* node);
	void ParseCamera(rapidxml::xml_node<>* node);
	void ParseTrajectory(rapidxml::xml_node<>* node);
	void ParseLights(rapidxml::xml_node<>* node);
	void ParseGamemode(rapidxml::xml_node<>* node);
	std::vector<PostprocessEffect*> ParsePostprocess(rapidxml::xml_node<>* node);
	Framebuffer* ParseFramebuffer(rapidxml::xml_node<>* node);
	void DrawDebug(ESContext* context);
public:
	std::string sceneManagerSource;
	rapidxml::xml_document<> doc;
	uint32 nextId = 0;
	DirectionalLight* dirLight;
	std::vector<PointLight*> pointLights;
	std::vector<SpotLight*> spotLights;
	Vector4 ambientLight = Vector4(1, 1, 1, 1);

	Vector3 backgroundColor;
	Vector3 fogColor;
	float fogStartRadius, fogEndRadius;
	std::unordered_map<uint32, SceneObject*> objects;
	std::unordered_map<uint32, Camera*> cameras;
	std::unordered_map<uint32, Trajectory*> trajectories;
	Camera* activeCamera;
	Texture* skyboxTexture;
	Shader* skyboxShader;
	Model* skyboxCube;
	float skyboxYOffset;
	bool showAABBs;
	bool showDebug;

	Input* input;
	CameraData activeCameraData;
	bool activeCameraDataSaved = false;
	Gamemode* gamemode = nullptr;
};
