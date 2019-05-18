#include "stdafx.h"
#include "Framebuffer.h"
#include "utils.h"
#include "ResourceManager.h"
#include "Globals.h"

Framebuffer::Framebuffer(FramebufferInitData res) :
	initData(res)
{
	Texture* color = nullptr, *depth = nullptr, *stencil = nullptr;
	color = res.color;
	depth = res.depth;
	stencil = res.stencil;
	if (res.width == 0)
		res.width = Globals::screenWidth;
	if (res.height == 0)
		res.height = Globals::screenHeight;
	Init(res.width, res.height, color, res.hasColorRenderbuffer, res.colorRenderbufferFormat, depth, res.hasDepthRenderbuffer, stencil, res.hasStencilRenderbuffer);
}

void Framebuffer::Init(uint w, uint h, 
	Texture* color, bool addColorRenderbuffer, TextureType colorRbFormat,
	Texture* depth, bool addDepthRenderbuffer,
	Texture* stencil, bool addStencilRenderbuffer)
{
	this->color = color;
	this->depth = depth;
	this->stencil = stencil;
	if (addDepthRenderbuffer || addStencilRenderbuffer)
	{
		GLint maxRenderbufferSize;
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
		ASSERT(maxRenderbufferSize > (int)w && maxRenderbufferSize > (int)h,
			"Trying to create a framebuffer with dimensions %ud x %ud bigger than the maximum %d!\n", w, h, maxRenderbufferSize);
	}
	if (color)
		ASSERT(color->width == w && color->height == h, "Framebuffer dimensions (%d x %d) do not match color texture dimensions (%d x %d)!\n", w, h, color->width, color->height);
	if (depth)
		ASSERT(depth->width == w && depth->height == h, "Framebuffer dimensions (%d x %d) do not match depth texture dimensions (%d x %d)!\n", w, h, depth->width, depth->height);
	if (stencil)
		ASSERT(stencil->width == w && stencil->height == h, "Framebuffer dimensions (%d x %d) do not match stencil texture dimensions (%d x %d)!\n", w, h, stencil->width, stencil->height);
		
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (color)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->texture, 0);
		hasColorrb = false;
	}
	else if (addColorRenderbuffer)
	{
		glGenRenderbuffers(1, &crbo);
		glBindRenderbuffer(GL_RENDERBUFFER, crbo);
		GLenum format = GL_RGB565;
		switch (colorRbFormat)
		{
		case TextureType::RENDERTARGET_RGB:
			format = GL_RGB565;
			break;
		case TextureType::RENDERTARGET_RGBA4444:
			format = GL_RGBA4;
			break;
		case TextureType::RENDERTARGET_RGBA5551:
			format = GL_RGB5_A1;
			break;
		}
		glRenderbufferStorage(GL_RENDERBUFFER, format, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, crbo);
		hasColorrb = true;
	}
	else hasColorrb = false;
	if (depth)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->texture, 0);
		hasDepthrb = false;
	}
	else if(addDepthRenderbuffer)
	{
		glGenRenderbuffers(1, &drbo);
		glBindRenderbuffer(GL_RENDERBUFFER, drbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16	, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, drbo);
		hasDepthrb = true;
	}
	else hasDepthrb = true;
	if (stencil)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencil->texture, 0);
		hasStencilrb = false;
	}
	else if(addStencilRenderbuffer)
	{
		glGenRenderbuffers(1, &srbo);
		glBindRenderbuffer(GL_RENDERBUFFER, srbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, srbo);
		hasStencilrb = true;
	}
	else hasStencilrb = false;
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Warning: framebuffer not complete!\n");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
	if (color)
		delete color;
	if (depth)
		delete depth;
	if (stencil)
		delete stencil;
	if (hasColorrb)
		glDeleteRenderbuffers(1, &crbo);
	if (hasDepthrb)
		glDeleteRenderbuffers(1, &drbo);
	if (hasStencilrb)
		glDeleteRenderbuffers(1, &srbo);
	glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::Bind()
{
	glViewport(0, 0, initData.width, initData.height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}