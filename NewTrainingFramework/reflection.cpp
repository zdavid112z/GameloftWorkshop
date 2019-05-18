#include "stdafx.h"
#include "reflection.h"

#include "SceneObject.h"
#include "terrainObject.h"
#include "fireObject.h"
#include "Light.h"
#include "MeshFactory.h"
#include "SceneManager.h"
#include "Enemy.h"
#include "Player.h"
#include "Projectile.h"
#include "WaveEvent.h"

std::unordered_map<std::string, ObjectReflection > Reflection::objects;

void Reflection::Init()
{
	REFLECT(SceneObject, rotationTransitionSpeed, 5.f);
	REFLECT(SceneObject, position, Vector3(0, 0, 0));
	REFLECT_EOL(SceneObject, rotation, Vector3(0, 0, 0),
		[](byte* object, byte* reflected)
	{ Vector3* v = (Vector3*)reflected; *v = *v / 180.f * PI; });
	REFLECT(SceneObject, scale, Vector3(1, 1, 1));
	REFLECT(SceneObject, specularPower, 8.f);
	REFLECT(SceneObject, diffuseIntensity, 1.f);
	REFLECT(SceneObject, specularIntensity, 1.f);
	REFLECT(SceneObject, aabbColor, Vector3(0.2f, 0.8f, 0.3f));
	REFLECT(SceneObject, noCollision, false);
	REFLECT(SceneObject, dontDrawNormals, false);
	REFLECT(SceneObject, shininess, Vector2(0, 1));
	REFLECT(SceneObject, diffuseColor, Vector4(1, 0, 1, 0));
	REFLECT(SceneObject, specularColor, Vector4(1, 1, 1, 1));
	REFLECT(SceneObject, defaultNormal, Vector4(0, 0, 1, 1));
	REFLECT(SceneObject, wired, false);
	REFLECT(SceneObject, colBoxScaleFactor, 1.f);
	REFLECT(SceneObject, customCollisionBox, false);
	REFLECT(SceneObject, collisionBoxMin, Vector3());
	REFLECT(SceneObject, collisionBoxMax, Vector3());
	REFLECT_NODEFAULT(SceneObject, shader);
	REFLECT_NODEFAULT(SceneObject, model);
	REFLECT_NODEFAULT(SceneObject, textures);
	REFLECT_OOL(SceneObject, [](byte* object, rapidxml::xml_node<>* node) {
		SceneObject* obj = (SceneObject*)object;
		obj->newRotation = obj->rotation; });

	REFLECT(TerrainObject, terrainHeights, Vector3(0, 0, 0));
	REFLECT(TerrainObject, texMultSingle, 0.f);
	REFLECT(TerrainObject, terrainSize, Vector2(0, 0));
	REFLECT(TerrainObject, cameraHeight, 0.f);
	REFLECT(TerrainObject, fakeMoveCenter, false);
	REFLECT(TerrainObject, fakeMoveSpeed, 1.f);
	REFLECT(TerrainObject, fakeDirection, Vector3(0, 0, 1));
	REFLECT_OOL(TerrainObject,
		[](byte* object, rapidxml::xml_node<>* node) {
		TerrainObject* tobj = (TerrainObject*)object;
		tobj->texMult = Vector2(tobj->terrainSize.x, tobj->terrainSize.y) * tobj->texMultSingle;
		ModelData md = MeshFactory::GenerateTerrain(int(tobj->terrainSize.x + 0.5f), int(tobj->terrainSize.y + 0.5f), tobj->texMultSingle);
		tobj->model = new Model(md);
		md.Delete();
	});

	REFLECT(FireObject, fireSpeed, 1.f);
	REFLECT(FireObject, dispMax, 1.f);
	REFLECT(FireObject, fireNumBlades, 2);
	REFLECT(FireObject, fireHeight, 1.f);
	REFLECT_OOL(FireObject,
		[](byte* object, rapidxml::xml_node<>* node) {
		FireObject* fobj = (FireObject*)object;
		ModelData md = MeshFactory::GenerateFire(fobj->fireNumBlades, fobj->fireHeight);
		fobj->model = new Model(md);
		md.Delete();
	});

	REFLECT(Player, speed, 50.f);
	REFLECT(Player, hp, 3);
	REFLECT(Player, invulnerabilityTimeout, 2.f);
	REFLECT(Player, invulnerabilityFlashSpeed, 0.4f);
	REFLECT(Player, invulnerabilityWhite, Vector4(1, 1, 1, 0.2f));
	REFLECT(Player, invulnerabilityRed, Vector4(1, 0, 0, 0.2f));

	REFLECT(Enemy, hurtTimeout, 0.3f);
	REFLECT(Enemy, hurtRed, Vector4(1, 0, 0, 0.2f));
	REFLECT(Enemy, homingPlayer, false);
	REFLECT(Enemy, hp, 4);
	REFLECT(Enemy, projectileSpeed, 50.f);
	REFLECT(Enemy, numProjectiles, 1);
	REFLECT(Enemy, projectilesSpacing, 0.15f);
	REFLECT(Enemy, shootInterval, 2.f);
	REFLECT(Enemy, projectileSpeed, 50.f);
	REFLECT_OOL(Enemy, 
		[](byte* object, rapidxml::xml_node<>* node) {
		Enemy* e = (Enemy*)object;
		std::string type = XMLUtils::ParseString(node->first_node("projectileType"), "simple");
		if (type == "simple")
			e->projectileType = EnemyProjectileType::SIMPLE;
		else if (type == "circular")
			e->projectileType = EnemyProjectileType::CIRCULAR;
	});

	REFLECT(DirectionalLight, intensity, 1.f);
	REFLECT(DirectionalLight, color, Vector4(1, 1, 1, 1));
	REFLECT_EOL(DirectionalLight, direction, Vector4(0, -1, 0, 0),
		[](byte* object, byte* reflected)
	{ Vector4* v = (Vector4*)reflected; v->Normalize(); });

	REFLECT(SpotLight, intensity, 1.f);
	REFLECT(SpotLight, color, Vector4(1, 1, 1, 1));
	REFLECT(SpotLight, position, Vector4(0, 0, 0, 1));
	REFLECT(SpotLight, attenuation, Vector4(0, 1, 0, 0));
	REFLECT(SpotLight, followsPosition, true);
	REFLECT(SpotLight, followsDirection, true);
	REFLECT_EOL(SpotLight, angleCosMin, 30.f,
		[](byte* object, byte* reflected)
	{ float* f = (float*)reflected; *f = cosf(*f / 180.f * PI); });
	REFLECT_EOL(SpotLight, angleCosMax, 60.f,
		[](byte* object, byte* reflected)
	{ float* f = (float*)reflected; *f = cosf(*f / 180.f * PI); });
	REFLECT_EOL(SpotLight, direction, Vector4(0, -1, 0, 0),
		[](byte* object, byte* reflected)
	{ Vector4* v = (Vector4*)reflected; v->Normalize(); });
	REFLECT_OOL(SpotLight,
		[](byte* object, rapidxml::xml_node<>* node) {
		SpotLight* pl = (SpotLight*)object;
		uint32 followingId = XMLUtils::ParseIdFromValue(node->first_node("following"), -1);
		if (followingId != -1)
			pl->following = SceneManager::instance->objects[followingId];
		else pl->following = nullptr;
	});

	REFLECT(PointLight, intensity, 1.f);
	REFLECT(PointLight, color, Vector4(1, 1, 1, 1));
	REFLECT(PointLight, position, Vector4(0, 0, 0, 1));
	REFLECT(PointLight, attenuation, Vector4(0, 1, 0, 0));
	REFLECT_OOL(PointLight,
		[](byte* object, rapidxml::xml_node<>* node) {
		PointLight* pl = (PointLight*)object;
		uint32 followingId = XMLUtils::ParseIdFromValue(node->first_node("following"), -1);
		if (followingId != -1)
			pl->following = SceneManager::instance->objects[followingId];
		else
			pl->following = nullptr;
	});

	REFLECT(WaveEventSpawn, numRepeats, 1);
	REFLECT(WaveEventSpawn, numEnemies, 1);
	REFLECT(WaveEventSpawn, delayBetweenSpawns, 1.f);
	REFLECT(WaveEventSpawn, enemySpeed, 1.f);
	REFLECT(WaveEventSpawn, deviationAngle, 0.f);
	REFLECT(WaveEventSpawn, spawnLineWidth, 20.f);
	REFLECT(WaveEventSpawn, minSpacingFactor, 1.f);
	REFLECT(WaveEventSpawn, paddingFactor, 1.5f);
	REFLECT(WaveEventSpawn, spawnCenter, Vector3(0, 0, 30));
	REFLECT(WaveEventSpawn, spawnRadius, 5.f);
	REFLECT(WaveEventSpawn, targetCircleCenter, Vector3(0, 0, 0));
	REFLECT(WaveEventSpawn, targetCircleRadius, 5.f);
	REFLECT_OOL(WaveEventSpawn, 
		[](byte* object, rapidxml::xml_node<>* node) {
		WaveEventSpawn* s = (WaveEventSpawn*)object;
		s->enemyType = XMLUtils::ParseString(node->first_node("enemyType"), "");
		std::string traj = XMLUtils::ParseString(node->first_node("trajectoryType"), "straight_down");
		if (traj == "straight_down")
			s->trajectoryType = WaveEventSpawnTrajectoryType::STRAIGHT_DOWN;
		else if (traj == "through_circle")
			s->trajectoryType = WaveEventSpawnTrajectoryType::THROUGH_CIRCLE;
		else if (traj == "homing_circle")
			s->trajectoryType = WaveEventSpawnTrajectoryType::HOMING_CIRCLE;
		std::string init = XMLUtils::ParseString(node->first_node("initType"), "above_map");
		if (init == "above_map")
			s->initType = WaveEventSpawnInitType::ABOVE_MAP;
		else if (init == "point_circle")
			s->initType = WaveEventSpawnInitType::POINT_CIRCLE;
	});

	REFLECT(WaveEventWait, amount, 1.f);

}