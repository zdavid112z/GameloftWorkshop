#pragma once

#include "../Utilities/utilities.h"
#include "../Utilities/Math.h"
#include "stdafx.h"
#include "Texture.h"

struct FramebufferInitData
{
	Texture* color = nullptr, *depth = nullptr, *stencil = nullptr;
	bool hasDepthRenderbuffer = false, hasStencilRenderbuffer = false;
	bool hasColorRenderbuffer = false;
	TextureType colorRenderbufferFormat;
	uint width = 0, height = 0;
};

class Framebuffer
{
public:
	Framebuffer(FramebufferInitData res);
	void Init(uint w, uint h,
		Texture* color, bool addColorRenderbuffer, TextureType colorRbFormat,
		Texture* depth, bool addDepthRenderbuffer, 
		Texture* stencil, bool addStencilRenderbuffer);
	~Framebuffer();
	void Bind();

	FramebufferInitData initData;
	Texture* color;
	Texture* depth;
	Texture* stencil;
private:
	GLuint fbo, drbo, srbo, crbo;
	bool hasDepthrb, hasStencilrb, hasColorrb;
};
