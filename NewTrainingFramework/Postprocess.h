#pragma once

#include "Framebuffer.h"
#include "../Utilities/Math.h"
#include "stdafx.h"
#include "Shader.h"
#include "utils.h"

enum class PostprocessTextureType
{
	EFFECT_OUTPUT,
	SOURCE,
	TEXTURE,
	OUTPUT
};

struct PostprocessEffectDesc
{
	PostprocessTextureType type;
	uint32 id;
	int index;
	bool depth;
	uint32 cameraId;
};


class PostprocessEffect
{
public:
	const uint32 typeId;
	PostprocessEffect(std::string name, const std::string& shaderName, Framebuffer* output) : 
		typeId(Utils::Hash(name)), shaderName(shaderName), output(output) {}
	virtual ~PostprocessEffect();
	virtual void ApplyEffect(const std::vector<Texture*>& textures);
	virtual void BindUniforms(Shader* shader) {}

	const std::string shaderName;
	Framebuffer* output;
	std::vector< PostprocessEffectDesc> texturesDesc;
};

class PlainPassEffect : public PostprocessEffect
{
public:
	PlainPassEffect(const std::string& name, Framebuffer* output) : PostprocessEffect(name, "effectPassthrough", output) {}
	void BindUniforms(Shader* shader) override {}
};

class GrayscaleEffect : public PostprocessEffect
{
public:
	GrayscaleEffect(const std::string& name, Framebuffer* output) : PostprocessEffect(name, "effectGrayscale", output) {}
	void BindUniforms(Shader* shader) override {}
};

class SepiaEffect : public PostprocessEffect
{
public:
	SepiaEffect(const std::string& name, Framebuffer* output) : PostprocessEffect(name, "effectSepia", output) {}
	void BindUniforms(Shader* shader) override {}
};

class AdditiveEffect : public PostprocessEffect
{
public:
	AdditiveEffect(const std::string& name, Framebuffer* output, Vector4 factors) : 
		PostprocessEffect(name, "effectAdditive", output), factors(factors) {}
	void BindUniforms(Shader* shader) override
	{
		shader->Uniform4f("uFactors", factors);
	}
	Vector4 factors;
};

class BlurEffect : public PostprocessEffect
{
public:
	BlurEffect(const std::string& name, Framebuffer* output, bool horizontal) :
		PostprocessEffect(name, "effectGaussianBlur", output),
		horizontal(horizontal) {}

	void BindUniforms(Shader* shader) override
	{
		shader->Uniform1f("uHorizontal", horizontal);
		shader->Uniform2f("uTexOffset", Vector2(1.f / output->initData.width, 1.f / output->initData.height));
	}
private:
	bool horizontal;
};

class LumaThresholdEffect : public PostprocessEffect
{
public:
	LumaThresholdEffect(const std::string& name, Framebuffer* output, Vector2 threshold) :
		PostprocessEffect(name, "effectLumaThreshold", output),
		threshold(threshold) {}

	void BindUniforms(Shader* shader) override
	{
		shader->Uniform2f("uThreshold", threshold);
	}
private:
	Vector2 threshold;
};