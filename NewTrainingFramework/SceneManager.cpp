#include "stdafx.h"
#include "SceneManager.h"
#include "XMLUtils.h"
#include "ResourceManager.h"
#include "Globals.h"
#include "terrainObject.h"
#include "Vertex.h"
#include "MeshFactory.h"
#include "fireObject.h"
#include <chrono>
#include <iostream>
#include "Gamemode.h"
#include "DebugGamemode.h"
#include "bulletGamemode.h"
#include "Player.h"
#include "Projectile.h"
#include "Enemy.h"

SceneManager* SceneManager::instance = nullptr;

void SceneManager::Init(ESContext* esContext)
{
	instance = new SceneManager(esContext);
	instance->Reload("bulletHell.xml");
	//instance->Reload();
	glClearColor(instance->backgroundColor.x, instance->backgroundColor.y, instance->backgroundColor.z, 1);
}

void SceneManager::Destroy()
{
	delete instance;
}

SceneManager::SceneManager(ESContext* esContext)
{
	showAABBs = false;
	showDebug = false;
	input = new Input(esContext);
	printf("Renderer: %s\n", (const char*)glGetString(GL_RENDERER));
	printf("Vendor: %s\n", (const char*)glGetString(GL_VENDOR));
}

SceneManager::~SceneManager()
{
	Delete();
	delete input;
}

void SceneManager::Delete()
{
	//activeCameraData = activeCamera->data;
	//activeCameraDataSaved = true;
	for (auto it = objects.begin(); it != objects.end(); it++)
		delete it->second;
	for (auto it = cameras.begin(); it != cameras.end(); it++)
		delete it->second;
	if (skyboxCube != nullptr)
		delete skyboxCube;
	for (auto it = trajectories.begin(); it != trajectories.end(); it++)
		delete it->second;
	if (dirLight)
		delete dirLight;
	for (PointLight* l : pointLights)
		delete l;
	for (SpotLight* l : spotLights)
		delete l;
	if (gamemode)
		delete gamemode;
}

void SceneManager::Reload(const std::string& scenePath)
{
	objects.clear();
	pointLights.clear();
	spotLights.clear();
	cameras.clear();
	trajectories.clear();
	activeCamera = nullptr;
	dirLight = nullptr;

	rapidxml::file<> fin(scenePath.c_str());
	sceneManagerSource = fin.data();

	doc.parse<0>((char*)sceneManagerSource.c_str());

	rapidxml::xml_node<>* root = doc.first_node();

	auto trajectories = root->first_node("trajectories");
	for (auto obj = trajectories->first_node("trajectory"); obj; obj = obj->next_sibling("trajectory"))
		ParseTrajectory(obj);

	backgroundColor = XMLUtils::ParseVector3(root->first_node("backgroundColor"));
	auto objects = root->first_node("objects");
	for (auto obj = objects->first_node("object"); obj; obj = obj->next_sibling("object"))
	{
		SceneObject* so = ParseSceneObject(obj);
		this->objects[so->id] = so;
	}
	auto cams = root->first_node("cameras");
	for (auto obj = cams->first_node("camera"); obj; obj = obj->next_sibling("camera"))
		ParseCamera(obj);
	if (activeCameraDataSaved)
	{
		activeCamera->data = activeCameraData;
	}
	auto fog = root->first_node("fog");
	fogColor = XMLUtils::ParseVector3(fog->first_node("color"));
	fogStartRadius = XMLUtils::ParseFloat(fog->first_node("start_radius"), 1e20f);
	fogEndRadius = XMLUtils::ParseFloat(fog->first_node("end_radius"), 1e20f);

	uint32 skyboxTextureid = XMLUtils::ParseIdFromValue(fog->first_node("skyboxTexture"), -1);
	skyboxTexture = ResourceManager::instance->LoadTexture(skyboxTextureid);
	uint32 skyboxShaderid = XMLUtils::ParseIdFromValue(fog->first_node("skyboxShader"), -1);
	skyboxShader = ResourceManager::instance->LoadShader(skyboxShaderid);
	if (skyboxShader != nullptr && skyboxTexture != nullptr)
	{
		float size = XMLUtils::ParseFloat(fog->first_node("skyboxSize"), 1);
		ModelData md = MeshFactory::GenerateCube(size);
		skyboxCube = new Model(md);
		md.Delete();
	}
	else skyboxCube = nullptr;
	skyboxYOffset = XMLUtils::ParseFloat(fog->first_node("skyboxYOffset"));
	ambientLight = XMLUtils::ParseVector4(root->first_node("ambientalLight"));
	ParseLights(root->first_node("lights"));
	//ParsePostprocess(root->first_node("postprocess"));

	ParseGamemode(root->first_node("gamemode"));
}

std::vector<PostprocessEffect*> SceneManager::ParsePostprocess(rapidxml::xml_node<>* node)
{
	std::vector<PostprocessEffect*> postprocessEffects;
	if(node->first_node("effects"))
	for (auto n = node->first_node("effects")->first_node(); n != nullptr; n = n->next_sibling())
	{
		std::string type = XMLUtils::ParseString(n->first_node("type"), "");
		Framebuffer* fb = ParseFramebuffer(n->first_node("output"));
		if (type == "grayscale")
		{
			postprocessEffects.push_back(new GrayscaleEffect(type, fb));
		}
		else if (type == "sepia")
		{
			postprocessEffects.push_back(new SepiaEffect(type, fb));
		}
		else if (type == "passthrough")
		{
			postprocessEffects.push_back(new PlainPassEffect(type, fb));
		}
		else if (type == "blur")
		{
			bool horizontal = XMLUtils::ParseBool(n->first_node("horizontal"));
			postprocessEffects.push_back(new BlurEffect(type, fb, horizontal));
		}
		else if (type == "lumaThreshold")
		{
			Vector2 threshold = XMLUtils::ParseVector2(n->first_node("threshold"));
			postprocessEffects.push_back(new LumaThresholdEffect(type, fb, threshold));
		}
		else if (type == "additive")
		{
			Vector4 factors = XMLUtils::ParseVector4(n->first_node("factors"), Vector4(1, 1, 0, 0));
			postprocessEffects.push_back(new AdditiveEffect(type, fb, factors));
		}
		else
		{
			delete fb;
			continue;
		}
		PostprocessEffect* eff = postprocessEffects.back();
		if(n->first_node("textures"))
		for (auto tn = n->first_node("textures")->first_node(); tn != nullptr; tn = tn->next_sibling())
		{
			PostprocessEffectDesc desc;
			std::string type = XMLUtils::ParseString(tn->first_node("type"), "");
			if (type == "source")
			{
				desc.type = PostprocessTextureType::SOURCE;
				desc.depth = XMLUtils::ParseBool(tn->first_node("depth"), false);
				desc.cameraId = XMLUtils::ParseIdFromValue(tn->first_node("camera"), -1);
			}
			if (type == "output")
			{
				desc.type = PostprocessTextureType::OUTPUT;
				desc.depth = XMLUtils::ParseBool(tn->first_node("depth"), false);
				desc.cameraId = XMLUtils::ParseIdFromValue(tn->first_node("camera"), -1);
			}
			else if (type == "effectOutput")
			{
				desc.type = PostprocessTextureType::EFFECT_OUTPUT;
				desc.id = XMLUtils::ParseIdFromValue(tn->first_node("effectType"));
				desc.index = XMLUtils::ParseInt(tn->first_node("index"), 0);
				desc.depth = XMLUtils::ParseBool(tn->first_node("depth"), false);
				desc.cameraId = XMLUtils::ParseIdFromValue(tn->first_node("camera"), -1);
			}
			else if (type == "texture")
			{
				desc.type = PostprocessTextureType::TEXTURE;
				desc.id = XMLUtils::ParseIdFromValue(tn->first_node("texture"));
			}
			eff->texturesDesc.push_back(desc);
		}
	}
	return postprocessEffects;
}

void SceneManager::ParseGamemode(rapidxml::xml_node<>* node)
{
	std::string type = node->first_attribute("type")->value();
	if (type == "debug")
	{
		gamemode = new DebugGamemode();
	}
	else if (type == "bullet")
	{
		BulletGamemode* g = new BulletGamemode();
		g->Parse(node);
		gamemode = g;
	}
}

SceneObject* SceneManager::CreatePrefab(const std::string& name)
{
	auto prefabs = doc.first_node()->first_node("prefabs");
	for (auto pf = prefabs->first_node("object"); pf; pf = pf->next_sibling("object"))
	{
		if (std::string(pf->first_attribute("prefab")->value()) == name)
			return ParseSceneObject(pf);
	}
	return nullptr;
}

SceneObject* SceneManager::ParseSceneObject(rapidxml::xml_node<>* node)
{
	SceneObject* obj;
	bool modelGenerated = false;
	std::string type = XMLUtils::ParseString(node->first_node("type"), "normal");
	if (type == "normal")
		obj = new SceneObject();
	else if (type == "terrain")
	{
		TerrainObject* tobj = new TerrainObject();
		Reflection::LoadVariables(tobj, node);
		modelGenerated = true;
		obj = tobj;
	}
	else if (type == "fire")
	{
		FireObject* fobj = new FireObject();
		Reflection::LoadVariables(fobj, node);
		modelGenerated = true;
		obj = fobj;
	}
	else if (type == "player")
	{
		Player* pobj = new Player();
		Reflection::LoadVariables(pobj, node);
		obj = pobj;
	}
	else if (type == "projectile")
	{
		Projectile* proj = new Projectile();
		Reflection::LoadVariables(proj, node);
		obj = proj;
	}
	else if (type == "enemy")
	{
		Enemy* enemy = new Enemy();
		Reflection::LoadVariables(enemy, node);
		enemy->player = (Player*)objects[HASH("player")];
		obj = enemy;
	}

	obj->dead = false;
	obj->id = XMLUtils::ParseIdFromAttribute(node, -1);
	obj->name = XMLUtils::ParseString(node->first_node("name"), "unnamed");
	if (obj->name == "unnamed")
	{
		auto nameAttr = node->first_attribute("name");
		if (nameAttr)
			obj->name = std::string(nameAttr->value(), nameAttr->value() + nameAttr->value_size());
	}
	if (obj->id == -1)
	{
		if(obj->name != "unnamed")
			obj->id = Utils::Hash(obj->name);
		else
		{
			bool found;
			std::string newName;
			do
			{
				nextId++;
				newName = "unnamed_" + std::to_string(nextId);
				found = objects.find(Utils::Hash(newName)) != objects.end();
			} while (found);
			obj->name = newName;
			obj->id = Utils::Hash(newName);
		}
	}

	Reflection::LoadVariables(obj, node);

	uint32 trajectoryId = XMLUtils::ParseIdFromValue(node->first_node("trajectory"), -1);
	if (trajectoryId != -1)
	{
		float trajectorySpeed = XMLUtils::ParseFloat(node->first_node("trajectorySpeed"), 1);
		obj->trajectoryData.Init(trajectories[trajectoryId], trajectorySpeed);
	}
	int followcamera = XMLUtils::ParseInt(node->first_node("followingCamera"), -1);
	if (followcamera == -1)
		obj->following = nullptr;
	else obj->following = objects[followcamera];

	if (obj->model == nullptr)
		printf("Warning: Object with name %s and id %u does not have a model!\n", obj->name.c_str(), obj->id);
	if (obj->shader == nullptr)
		printf("Warning: Object with name %s and id %u does not have a shader!\n", obj->name.c_str(), obj->id);
	ASSERT(objects.find(obj->id) == objects.end(), "Object has the same id as another object in the scene! (name = '%s', id = %u)", obj->name.c_str(), obj->id);
	//printf("Parsed scene object with id %u and name %s\n", obj->id, obj->name.c_str());
	return obj;
}

void SceneManager::ParseCamera(rapidxml::xml_node<>* node)
{
	float znear, zfar, aspect, fov, moveSpeed, rotSpeed;
	znear = XMLUtils::ParseFloat(node->first_node("near"));
	zfar = XMLUtils::ParseFloat(node->first_node("far"));
	fov = XMLUtils::ParseFloat(node->first_node("fov")) * PI / 180.f;
	moveSpeed = XMLUtils::ParseFloat(node->first_node("translationSpeed"));
	rotSpeed = XMLUtils::ParseFloat(node->first_node("rotationSpeed"));
	aspect = XMLUtils::ParseFloat(node->first_node("aspect"),
		Globals::screenWidth / (float)Globals::screenHeight);
	Vector4 ortho = XMLUtils::ParseVector4(node->first_node("ortho"), Vector4());
	Camera* camera;
	if(ortho == Vector4())
		camera = new Camera(znear, zfar, fov, aspect, moveSpeed, rotSpeed);
	else camera = new Camera(znear, zfar, ortho);
	Vector3 pos = XMLUtils::ParseVector3(node->first_node("position"));
	Vector3 target = XMLUtils::ParseVector3(node->first_node("target"));
	Vector3 up = XMLUtils::ParseVector3(node->first_node("up"));
	std::string cullString = XMLUtils::ParseString(node->first_node("cullFace"), "none");
	if (cullString == "none")
		camera->cullFace = CullFace::NONE;
	else if (cullString == "back")
		camera->cullFace = CullFace::BACK;
	else if (cullString == "front")
		camera->cullFace = CullFace::FRONT;
	camera->data.position = pos;
	camera->data.up = up;
	camera->data.forward = -(target - pos).Normalize();
	camera->data.right = up.Cross(camera->data.forward);
	camera->UpdateViewProjection();
	uint32 id = XMLUtils::ParseIdFromAttribute(node);
	camera->hasSkybox = XMLUtils::ParseBool(node->first_node("hasSkybox"), true);
	camera->clearColor = XMLUtils::ParseVector4(node->first_node("clearColor"), Vector4(1, 0, 1, 1));
	camera->clearColorOnBegin = XMLUtils::ParseBool(node->first_node("clearColorOnBegin"), true);
	camera->clearDepthOnBegin = XMLUtils::ParseBool(node->first_node("clearDepthOnBegin"), true);
	camera->clearStencilOnBegin = XMLUtils::ParseBool(node->first_node("clearStencilOnBegin"), true);
	camera->viewsDebug = XMLUtils::ParseBool(node->first_node("viewsDebug"), false);
	auto pp = node->first_node("postprocess");
	camera->targetFramebuffer = ParseFramebuffer(pp->first_node("target"));
	camera->postprocessEffects = ParsePostprocess(pp);
	if (camera->GetOutput() == nullptr)
		activeCamera = camera;
	cameras[id] = camera;
}

Framebuffer* SceneManager::ParseFramebuffer(rapidxml::xml_node<>* node)
{
	if (node == nullptr)
		return nullptr;
	std::string targetType = XMLUtils::ParseString(node->first_node("type"), "");
	if (targetType == "framebuffer")
	{
		FramebufferInitData fbdata;
		fbdata.hasDepthRenderbuffer = false;
		fbdata.hasStencilRenderbuffer = false;
		fbdata.hasColorRenderbuffer = false;
		std::string imageTypes[] = { "color", "depth", "stencil" };
		Texture** textures[] = { &fbdata.color, &fbdata.depth, &fbdata.stencil };
		TextureType defaultTypes[] = { TextureType::RENDERTARGET_RGB, TextureType::DEPTH, TextureType::DEPTH };
		int width = Globals::screenWidth, height = Globals::screenHeight;
		float screenRatio = XMLUtils::ParseFloat(node->first_node("screenRatio"), -1);
		if (screenRatio != -1)
		{
			width = int(screenRatio * width);
			height = int(screenRatio * height);
		}
		else
		{
			Vector2 size = XMLUtils::ParseVector2(node->first_node("size"), Vector2(0, 0));
			width = int(round(size.x));
			height = int(round(size.y));
		}
		fbdata.width = width;
		fbdata.height = height;
		for (int i = 0; i < 3; i++)
		{
			*(textures[i]) = nullptr;

			auto texNode = node->first_node(imageTypes[i].c_str());
			if (texNode == nullptr)
				continue;
			std::string texType = XMLUtils::ParseString(texNode->first_node("type"), "");
			if (texType == "texture")
			{
				TextureResource res;
				std::string texFormat = XMLUtils::ParseString(texNode->first_node("format"), "");
				if (texFormat == "rgba4444")
					res.type = TextureType::RENDERTARGET_RGBA4444;
				else if (texFormat == "rgba5551")
					res.type = TextureType::RENDERTARGET_RGBA5551;
				else res.type = defaultTypes[i];
				res.width = width;
				res.height = height;
				res.isCube = false;
				res.minFilter = XMLUtils::ParseString(node->first_node("min_filter"), "LINEAR");
				res.magFilter = XMLUtils::ParseString(node->first_node("mag_filter"), "LINEAR");
				res.wraps = XMLUtils::ParseString(node->first_node("wrap_s"), "CLAMP_TO_EDGE");
				res.wrapt = XMLUtils::ParseString(node->first_node("wrap_t"), "CLAMP_TO_EDGE");
				res.wrapr = XMLUtils::ParseString(node->first_node("wrap_r"), "CLAMP_TO_EDGE");
				*(textures[i]) = new Texture(&res);
			}
			else if (texType == "renderbuffer")
			{
				if (i == 1)
					fbdata.hasDepthRenderbuffer = true;
				else if (i == 2)
					fbdata.hasStencilRenderbuffer = true;
				else if (i == 0)
				{
					fbdata.hasColorRenderbuffer = true;
					std::string texFormat = XMLUtils::ParseString(texNode->first_node("format"), "");
					if (texFormat == "rgba4444")
						fbdata.colorRenderbufferFormat = TextureType::RENDERTARGET_RGBA4444;
					else if (texFormat == "rgba5551")
						fbdata.colorRenderbufferFormat = TextureType::RENDERTARGET_RGBA5551;
					else fbdata.colorRenderbufferFormat = TextureType::RENDERTARGET_RGB;
				}
			}
		}
		return new Framebuffer(fbdata);
	}
	return nullptr;
}

void SceneManager::ParseTrajectory(rapidxml::xml_node<>* node)
{
	Trajectory* t = new Trajectory();
	uint32 id = XMLUtils::ParseIdFromAttribute(node);
	std::string type = XMLUtils::ParseString(node->first_node("type"), "");
	std::string direction = XMLUtils::ParseString(node->first_node("direction"), "normal");
	bool alternate = false;
	if (direction == "normal")
		alternate = false;
	else if (direction == "alternate")
		alternate = true;
	int reps = XMLUtils::ParseInt(node->first_node("iteration-count"), 1);
	if (XMLUtils::ParseString(node->first_node("iteration-count"), "infinite") == "infinite")
		reps = 0x7fffffff;
	float speed = XMLUtils::ParseFloat(node->first_node("speed"), 1);
	std::vector<Vector3> points;
	auto pointsNode = node->first_node("points");
	if (pointsNode != nullptr)
	{
		for (auto pointNode = pointsNode->first_node("point"); pointNode != nullptr; pointNode = pointNode->next_sibling())
			points.push_back(XMLUtils::ParseVector3(pointNode));
	}
	if (type == "linear")
	{
		t->CreateLine(points[0], points[1], speed, reps, alternate);
	}
	else if (type == "line_strip")
	{
		t->CreateLineStrip(points.data(), points.size(), speed, reps, alternate, false);
	}
	else if (type == "line_loop")
	{
		t->CreateLineLoop(points.data(), points.size(), speed, reps);
	}
	else if (type == "circle")
	{
		Vector3 center = XMLUtils::ParseVector3(node->first_node("center"));
		Vector3 rot = XMLUtils::ParseVector3(node->first_node("rotation"));
		float radius = XMLUtils::ParseFloat(node->first_node("radius"));
		t->CreateCircle(center, radius, rot, speed, reps);
	}
	trajectories[id] = t;
}

void SceneManager::ParseLights(rapidxml::xml_node<>* node)
{
	for (auto l = node->first_node(); l != nullptr; l = l->next_sibling())
	{
		std::string type = XMLUtils::ParseString(l->first_node("type"), "");
		if (type == "directional")
		{
			dirLight = new DirectionalLight();
			Reflection::LoadVariables(dirLight, l);
		}
		else if (type == "point")
		{
			PointLight* pl = new PointLight();
			Reflection::LoadVariables(pl, l);
			pointLights.push_back(pl);
		}
		else if (type == "spot")
		{
			SpotLight* pl = new SpotLight();
			Reflection::LoadVariables(pl, l);
			spotLights.push_back(pl);
		}
	}
}

void SceneManager::Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	for (auto it = objects.begin(); it != objects.end(); it++)
		it->second->Key(esContext, key, bIsPressed);
	if (gamemode != nullptr)
		gamemode->Key(esContext, key, bIsPressed);
	input->Key(esContext, key, bIsPressed);
}

void SceneManager::Update(ESContext* context, float delta)
{
	input->Update(context, delta);
	ResourceManager::instance->GetSoundSystem()->update();
	// FMOD_VECTOR pos, vel, forward, up;
	// ResourceManager::instance->GetSoundSystem()->set3DListenerAttributes(0, &listenerpos, &vel, &forward, &up);

	for (auto it = objects.begin(); it != objects.end(); it++)
		it->second->Update(context, delta);

	if (gamemode != nullptr)
		gamemode->Update(context, delta);

	RemoveIfDead(objects, true);
}

void SceneManager::Draw(ESContext* context)
{
	for (PointLight* pl : pointLights)
	{
		if (pl->following)
			pl->position = pl->following->position;
	}
	for (SpotLight* sl : spotLights)
	{
		if (sl->following)
		{
			if(sl->followsPosition)
				sl->position = sl->following->position;
			if (sl->followsDirection)
			{
				Matrix rx, ry, rz, r;
				rx.SetRotationX(sl->following->rotation.x);
				ry.SetRotationY(sl->following->rotation.y);
				rz.SetRotationZ(sl->following->rotation.z);
				r = rx * ry * rz;
				Vector4 dir = r * Vector4(0, 0, -1, 0);
				sl->direction = Vector3(dir);
			}
		}
	}
	for (auto p : cameras)
	{
		Camera* c = p.second;
		activeCamera = c;
		Framebuffer* fb = c->targetFramebuffer;
		if (fb == nullptr)
		{
			glViewport(0, 0, Globals::screenWidth, Globals::screenHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else fb->Bind();
		switch (c->cullFace)
		{
		case CullFace::NONE:
			glDisable(GL_CULL_FACE);
			break;
		case CullFace::BACK:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		case CullFace::FRONT:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		}
		GLenum clear = 0;
		if (c->clearColorOnBegin)
		{
			clear |= GL_COLOR_BUFFER_BIT;
			glClearColor(c->clearColor.x, c->clearColor.y, c->clearColor.z, c->clearColor.w);
		}
		if (c->clearDepthOnBegin)
			clear |= GL_DEPTH_BUFFER_BIT;
		if (c->clearStencilOnBegin)
			clear |= GL_STENCIL_BUFFER_BIT;
		if (clear)
			glClear(clear);
		if (c->hasSkybox)
		{
			if (skyboxCube && skyboxShader && skyboxTexture)
			{
				glDepthMask(GL_FALSE);
				Matrix t, ct;
				ct.SetTranslation(activeCamera->data.position);
				t.SetTranslation(0, skyboxYOffset, 0);
				ct = t * ct;
				skyboxShader->Bind();
				skyboxShader->UniformMatrix("uMVP", ct * activeCamera->GetViewProjection());
				skyboxShader->UniformMatrix("uModel", ct);
				skyboxShader->BindTexture(skyboxTexture);
				skyboxShader->Uniform1f("uSkyboxYOffset", skyboxYOffset);
				skyboxCube->Draw(skyboxShader, false);
				glDepthMask(GL_TRUE);
			}
		}
		for (auto it = objects.begin(); it != objects.end(); it++)
		{
			if (showDebug && c->viewsDebug)
			{
				bool wasWired = it->second->wired;
				it->second->wired = true;
				it->second->Draw(context);
				it->second->wired = wasWired;
			}
			else it->second->Draw(context);
		}
		if (c->viewsDebug)
			DrawDebug(context);
		Framebuffer* src = c->targetFramebuffer;
		glDisable(GL_BLEND);
		for (uint i = 0; i < c->postprocessEffects.size(); i++)
		{
			std::vector<Texture*> textures;
			textures.push_back(src->color);
			for (PostprocessEffectDesc d : c->postprocessEffects[i]->texturesDesc)
			{
				Framebuffer* aux;
				Camera* sourceCamera = c;
				if (d.cameraId != -1)
					sourceCamera = cameras[d.cameraId];
				switch (d.type)
				{
				case PostprocessTextureType::OUTPUT:
					aux = sourceCamera->GetOutput();
					if (d.depth)
						textures.push_back(aux->depth);
					else textures.push_back(aux->color);
					break;
				case PostprocessTextureType::SOURCE:
					aux = sourceCamera->targetFramebuffer;
					if(d.depth)
						textures.push_back(aux->depth);
					else textures.push_back(aux->color);
					break;
				case PostprocessTextureType::TEXTURE:
					textures.push_back(ResourceManager::instance->LoadTexture(d.id));
					break;
				case PostprocessTextureType::EFFECT_OUTPUT:
					aux = sourceCamera->GetEffect(d, c->postprocessEffects[i])->output;
					if (d.depth)
						textures.push_back(aux->depth);
					else textures.push_back(aux->color);
					break;
				}
			}
			c->postprocessEffects[i]->ApplyEffect(textures);
			src = c->postprocessEffects[i]->output;
		}
		glEnable(GL_BLEND);
	}

	if (gamemode != nullptr)
		gamemode->Draw(context);
}

void SceneManager::DrawDebug(ESContext* context)
{
	if (showAABBs)
	{
		Shader* singleColor = ResourceManager::instance->LoadShader("singleColor");
		Model* cube = ResourceManager::instance->LoadModel("cube");
		for (auto it = objects.begin(); it != objects.end(); it++)
		{
			SceneObject* o = it->second;
			if (o->noCollision)
				continue;
			Matrix t, s, m, mvp;
			AABB aabb = o->GetCollisionBox();
			Vector3 center = (aabb.min + aabb.max) / 2.f;
			Vector3 size = (aabb.max - aabb.min) / 2.f;
			t.SetTranslation(center);
			s.SetScale(o->scale);
			m = s * t;
			mvp = m * SceneManager::instance->activeCamera->GetViewProjection();
			singleColor->Bind();
			singleColor->UniformMatrix("uMVP", mvp);
			singleColor->UniformMatrix("uModel", m);
			singleColor->Uniform3f("uSize", size);
			singleColor->Uniform4f("uColor", Vector4(o->aabbColor, 1));
			singleColor->Uniform1f("uUseColorFromNormal", 0);
			cube->Draw(singleColor, true);
		}
	}

	if (showDebug)
	{
		Shader* singleColor = ResourceManager::instance->LoadShader("singleColor");
		Model* axis = ResourceManager::instance->LoadModel("axis");
		Model* cube = ResourceManager::instance->LoadModel("cube");
		for (auto it = objects.begin(); it != objects.end(); it++)
		{
			SceneObject* o = it->second;
			{
				// Draw axis
				Matrix t, rx, ry, rz, s, m, mvp;
				t.SetTranslation(o->position);
				rx.SetRotationX(o->rotation.x);
				ry.SetRotationY(o->rotation.y);
				rz.SetRotationZ(o->rotation.z);
				m = rx * ry * rz * t;
				if (o->following != nullptr)
				{
					Matrix camModel = o->following->GetModelMatrix();
					m = m * camModel;
				}
				mvp = m * SceneManager::instance->activeCamera->GetViewProjection();
				singleColor->Bind();
				singleColor->UniformMatrix("uMVP", mvp);
				singleColor->UniformMatrix("uModel", m);
				singleColor->Uniform3f("uSize", Vector3(0.1f, 0.1f, 0.1f));
				singleColor->Uniform4f("uColor", Vector4(1, 1, 0.5f, 1));
				singleColor->Uniform1f("uUseColorFromNormal", 1);
				axis->Draw(singleColor, false);
			}
			if (!o->dontDrawNormals)
			{
				Matrix t, rx, ry, rz, s, m, mvp;
				t.SetTranslation(o->position);
				rx.SetRotationX(o->rotation.x);
				ry.SetRotationY(o->rotation.y);
				rz.SetRotationZ(o->rotation.z);
				s.SetScale(o->scale);
				m = s * rx * ry * rz * t;
				if (o->following != nullptr)
				{
					Matrix camModel = o->following->GetModelMatrix();
					m = camModel * m;
				}
				mvp = m * SceneManager::instance->activeCamera->GetViewProjection();
				singleColor->Bind();
				singleColor->UniformMatrix("uMVP", mvp);
				singleColor->UniformMatrix("uModel", m);
				singleColor->Uniform3f("uSize", Vector3(1, 1, 1));
				singleColor->Uniform4f("uColor", Vector4(1, 1, 0.5f, 1));
				singleColor->Uniform1f("uUseColorFromNormal", 0);
				o->model->DrawNormals(singleColor);
			}

		}

		for (PointLight* l : pointLights)
		{
			Matrix t, mvp;
			t.SetTranslation(Vector3(l->position));
			mvp = t * SceneManager::instance->activeCamera->GetViewProjection();
			singleColor->Bind();
			singleColor->UniformMatrix("uMVP", mvp);
			singleColor->UniformMatrix("uModel", t);
			singleColor->Uniform3f("uSize", Vector3(1, 1, 1) * 8);
			singleColor->Uniform4f("uColor", Vector4(Vector3(l->color), 1));
			singleColor->Uniform1f("uUseColorFromNormal", 0);
			cube->Draw(singleColor, false);
		}

		for (SpotLight* l : spotLights)
		{
			Matrix t, mvp;
			t.SetTranslation(Vector3(l->position));
			mvp = t * SceneManager::instance->activeCamera->GetViewProjection();
			singleColor->Bind();
			singleColor->UniformMatrix("uMVP", mvp);
			singleColor->UniformMatrix("uModel", t);
			singleColor->Uniform3f("uSize", Vector3(1, 1, 1) * 8);
			singleColor->Uniform4f("uColor", Vector4(Vector3(l->color), 1));
			singleColor->Uniform1f("uUseColorFromNormal", 0);
			cube->Draw(singleColor, false);
		}

		Matrix identity;
		identity.SetIdentity();
		singleColor->Bind();
		singleColor->UniformMatrix("uMVP", SceneManager::instance->activeCamera->GetViewProjection());
		singleColor->UniformMatrix("uModel", identity);
		singleColor->Uniform3f("uSize", Vector3(2, 2, 2) * 0.1f);
		singleColor->Uniform4f("uColor", Vector4(1, 1, 0.5f, 1));
		singleColor->Uniform1f("uUseColorFromNormal", 1);
		axis->Draw(singleColor, false);
	}
}
