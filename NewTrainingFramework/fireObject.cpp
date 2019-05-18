#include "stdafx.h"
#include "fireObject.h"
#include "SceneManager.h"

FireObject::~FireObject()
{
	delete model;
}

void FireObject::Draw(ESContext* context)
{
	if (SceneManager::instance->activeCamera != nullptr)
	{
		glDepthMask(GL_FALSE);
		Matrix m, mvp;
		m = GetModelMatrix();
		if (following != nullptr)
		{
			Matrix camModel = following->GetModelMatrix();
			m = m * camModel;
		}
		mvp = m * SceneManager::instance->activeCamera->GetViewProjection();
		shader->Bind();
		shader->UniformMatrix("uMVP", mvp);
		shader->UniformMatrix("uModel", m);
		shader->Uniform3f("uFogColor", SceneManager::instance->fogColor);
		shader->Uniform1f("uFogStartRadius", SceneManager::instance->fogStartRadius);
		shader->Uniform1f("uFogEndRadius", SceneManager::instance->fogEndRadius);
		shader->Uniform3f("uCameraPosition", SceneManager::instance->activeCamera->data.position);
		shader->Uniform1f("uTime", time);
		shader->Uniform1f("uDispMax", dispMax);
		for (uint i = 0; i < textures.size(); i++)
			shader->BindTexture(textures[i], i);
		model->Draw(shader, wired);
		glDepthMask(GL_TRUE);
	}
}

void FireObject::Update(ESContext* context, float delta)
{
	time += delta * fireSpeed;
}
