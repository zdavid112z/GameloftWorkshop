#pragma once

#include "../Utilities/utilities.h"

class TextureResource;
class Framebuffer;

enum class TextureType
{
	LOADED,
	DEPTH,
	RENDERTARGET_RGB,
	RENDERTARGET_RGBA5551,
	RENDERTARGET_RGBA4444
};

class Texture
{
	friend class Framebuffer;
public:
	Texture(TextureResource* res);
	~Texture();

	void Bind(int index);
private:
	TextureResource* resource = nullptr;
	GLuint texture;
	bool isCube;
	uint width, height;
};