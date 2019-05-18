#include "stdafx.h"
#include "Postprocess.h"
#include "ResourceManager.h"
#include "Globals.h"

PostprocessEffect::~PostprocessEffect()
{
	if (output)
		delete output;
}

void PostprocessEffect::ApplyEffect(const std::vector<Texture*>& textures)
{
	if (output == nullptr)
	{
		glViewport(0, 0, Globals::screenWidth, Globals::screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else output->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	Shader* shader = ResourceManager::instance->LoadShader(shaderName);
	Model* quad = ResourceManager::instance->LoadModel("screenQuad");
	shader->Bind();
	shader->BindCommonAttributes();
	for (uint i = 0; i < textures.size(); i++)
		shader->BindTexture(textures[i], i);
	BindUniforms(shader);
	quad->Draw(shader);
}
