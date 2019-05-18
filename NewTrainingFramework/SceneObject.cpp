#include "stdafx.h"
#include "SceneObject.h"
#include "SceneManager.h"

void SceneObject::Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{

}

void SceneObject::Update(ESContext* context, float delta)
{
	if (sound != nullptr)
	{

		sound->Play();
	}
	if (!trajectoryData.Finished() && trajectoryData.trajectory != nullptr)
	{
		bool rotate180;
		position = trajectoryData.Update(delta, rotate180);
		if (rotate180)
			newRotation = rotation + Vector3(0, PI, 0);
	}
	rotation = rotation.Lerp(newRotation, rotationTransitionSpeed * delta);
}

Matrix SceneObject::GetModelMatrix()
{
	Matrix t, rx, ry, rz, s, m, mvp;
	t.SetTranslation(position);
	rx.SetRotationX(rotation.x);
	ry.SetRotationY(rotation.y);
	rz.SetRotationZ(rotation.z);
	s.SetScale(scale);
	m = s * rx * ry * rz * t;
	return m;
}

void SceneObject::Draw(ESContext* context)
{
	if (SceneManager::instance->activeCamera != nullptr)
	{
		Matrix m = GetModelMatrix(), mvp;
		if (following != nullptr)
		{
			Matrix camModel = following->GetModelMatrix();
			m = camModel * m;
		}
		mvp = m * SceneManager::instance->activeCamera->GetViewProjection();
		shader->Bind();
		shader->UniformMatrix("uMVP", mvp);
		shader->UniformMatrix("uModel", m);
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
			shadowFb->depth->Bind(3);
		}

		for (uint i = 0; i < textures.size(); i++)
			shader->BindTexture(textures[i], i);
		if(SceneManager::instance->skyboxTexture)
			shader->BindEnvMap(SceneManager::instance->skyboxTexture);
		model->Draw(shader, wired);
	}
}

AABB SceneObject::GetAABB()
{
	AABB aabb = model->GetAABB();
	Vector3 center = (aabb.min + aabb.max) / 2.f;
	Vector3 size = (aabb.max - aabb.min) / 2.f;
	size.x *= scale.x;
	size.y *= scale.y;
	size.z *= scale.z;
	aabb.min = center - size + position;
	aabb.max = center + size + position;
	return aabb;
}

AABB SceneObject::GetCollisionBox()
{
	AABB aabb;
	if (!customCollisionBox)
	{
		aabb = model->GetAABB();
		Vector3 center = (aabb.min + aabb.max) / 2.f;
		Vector3 size = (aabb.max - aabb.min) / 2.f;
		size.x *= scale.x;
		size.y *= scale.y;
		size.z *= scale.z;
		size *= colBoxScaleFactor;
		aabb.min = center - size + position;
		aabb.max = center + size + position;
	}
	else
	{
		aabb.min = collisionBoxMin;
		aabb.max = collisionBoxMax;
		aabb.min.x *= scale.x;
		aabb.min.y *= scale.y;
		aabb.min.z *= scale.z;
		aabb.max.x *= scale.x;
		aabb.max.y *= scale.y;
		aabb.max.z *= scale.z;
		aabb.min += position;
		aabb.max += position;
	}
	return aabb;
}
