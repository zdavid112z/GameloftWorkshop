#include "stdafx.h"
#include "terrainObject.h"
#include "SceneManager.h"

TerrainObject::~TerrainObject()
{
	delete model;
}

void TerrainObject::Update(ESContext* context, float delta)
{
	fakePosition += fakeDirection * fakeMoveSpeed * delta;
}

void TerrainObject::Draw(ESContext* context)
{
	if (SceneManager::instance->activeCamera != nullptr)
	{
		Matrix t, rx, ry, rz, s, m, mvp;
		//position = SceneManager::instance->activeCamera->position + offsetToCamera;
		Camera* mainCamera = SceneManager::instance->activeCamera;
		Vector2 uvOffset;
		if (fakeMoveCenter)
		{
			position.x = -std::fmod(fakePosition.x, scale.x);
			position.y = 0;
			position.z = -std::fmod(fakePosition.z, scale.z);
			uvOffset = Vector2(fakePosition.x - std::fmod(fakePosition.x, scale.x), fakePosition.z - std::fmod(fakePosition.z, scale.z));
			position -= Vector3(terrainSize.x * scale.x / 2, 0, terrainSize.y * scale.z / 2);
			uvOffset.x /= scale.x * terrainSize.x;
			uvOffset.y /= scale.z * terrainSize.y;
		}
		else
		{
			position.x = mainCamera->data.position.x - std::fmod(mainCamera->data.position.x, scale.x);
			position.y = 0;
			position.z = mainCamera->data.position.z - std::fmod(mainCamera->data.position.z, scale.z);
			uvOffset = Vector2(position.x, position.z);
			position -= Vector3(terrainSize.x * scale.x / 2, 0, terrainSize.y * scale.z / 2);
			uvOffset.x /= scale.x * terrainSize.x;
			uvOffset.y /= scale.z * terrainSize.y;
		}

		t.SetTranslation(position);
		rx.SetRotationX(rotation.x);
		ry.SetRotationY(rotation.y);
		rz.SetRotationZ(rotation.z);
		s.SetScale(scale);
		m = s * rx * ry * rz * t;
		mvp = m * SceneManager::instance->activeCamera->GetViewProjection();
		shader->Bind();
		shader->Uniform3f("uHeights", terrainHeights);
		shader->Uniform2f("uTexMult", texMult);
		shader->Uniform2f("uUVOffset", uvOffset);
		shader->BindSceneData();
		shader->Uniform2f("uShininess", shininess);
		shader->Uniform4f("uDiffuseColor", diffuseColor);
		shader->Uniform4f("uSpecularColor", specularColor);
		shader->Uniform4f("uDefaultNormal", defaultNormal);
		shader->Uniform1f("uSpecularPower", specularPower);
		shader->Uniform1f("uDiffuseIntensity", diffuseIntensity);
		shader->Uniform1f("uSpecularIntensity", specularIntensity);

		auto shadowCameraIt = SceneManager::instance->cameras.find(HASH("shadowMap"));
		if (shadowCameraIt != SceneManager::instance->cameras.end())
		{
			Camera* shadowCamera = shadowCameraIt->second;
			shader->UniformMatrix("uShadowVP", shadowCamera->GetViewProjection());
			Framebuffer* shadowFb = shadowCamera->GetOutput();
			shader->Uniform2f("uShadowTexelSize", Vector2(1.f / shadowFb->initData.width, 1.f / shadowFb->initData.height));
			shadowFb->depth->Bind(4);
		}

		shader->UniformMatrix("uMVP", mvp);
		shader->UniformMatrix("uModel", m);

		for (uint i = 0; i < textures.size(); i++)
			textures[i]->Bind(i);
		model->Draw(shader, wired);
	}
}
