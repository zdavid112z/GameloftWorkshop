#include "stdafx.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "ImageUtils.h"
#include "Globals.h"

Texture::Texture(TextureResource* res)
{
	resource = new TextureResource(*res);
	int w, h, bpp = 32;
	char* data = nullptr;
	if (res->type == TextureType::LOADED)
	{
		data = LoadTGA(res->path.c_str(), &w, &h, &bpp);
		if (data == nullptr)
		{
			printf("Could not load image %s!\n", res->path.c_str());
			// TODO: Could not load image
		}
	}
	else
	{
		w = res->width;
		h = res->height;
		if (w == 0)
			w = Globals::screenWidth;
		if (h == 0)
			h = Globals::screenHeight;
	}
	width = w;
	height = h;
	glGenTextures(1, &texture);
	isCube = res->isCube;
	GLenum minFilter = res->minFilter == "LINEAR" ? GL_LINEAR : GL_NEAREST;
	if(data != nullptr && !res->isCube)
		minFilter = res->minFilter == "LINEAR" ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
	GLenum magFilter = res->magFilter == "LINEAR" ? GL_LINEAR : GL_NEAREST;
	GLenum wraps = res->wraps == "REPEAT" ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	GLenum wrapt = res->wrapt == "REPEAT" ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	GLenum wrapr = res->wrapr == "REPEAT" ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	GLenum target = res->isCube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
	GLenum format = bpp == 32 ? GL_RGBA : GL_RGB;
	GLenum dataType = GL_UNSIGNED_BYTE;
	switch (res->type)
	{
	case TextureType::DEPTH:
		format = GL_DEPTH_COMPONENT;
		dataType = GL_UNSIGNED_SHORT;
		break;
	case TextureType::RENDERTARGET_RGB:
		format = GL_RGB;
		dataType = GL_UNSIGNED_SHORT_5_6_5;
		break;
	case TextureType::RENDERTARGET_RGBA5551:
		format = GL_RGBA;
		dataType = GL_UNSIGNED_SHORT_5_5_5_1;
		break;
	case TextureType::RENDERTARGET_RGBA4444:
		format = GL_RGBA;
		dataType = GL_UNSIGNED_SHORT_4_4_4_4;
		break;
	}
	glBindTexture(target, texture);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, wraps);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapt);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapr);

	if (!res->isCube)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, dataType, data);
		if (data != nullptr)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		byte** faces = ImageUtils::SplitCubemap((byte*)data, w, h, bpp);
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, w / 4, h / 3, 0, format, dataType, faces[i]);
			delete[] faces[i];
		}
		delete[] faces;
	}
	if(data)
		delete[] data;
}

Texture::~Texture()
{
	if(resource)
		delete resource;
	glDeleteTextures(1, &texture);
}

void Texture::Bind(int index)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(isCube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, texture);
}
