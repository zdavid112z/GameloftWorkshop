// NewTrainingFramework.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Vertex.h"
#include "Shaders.h"
#include <conio.h>
#include "Globals.h"
#include "Camera.h"
#include "ModelParser.h"
#include "ResourceManager.h"
#include "Model.h"
#include "Texture.h"
#include "Shader.h"
#include "utils.h"
#include "SceneManager.h"
#include "terrainObject.h"
#include "fireObject.h"
#include "MeshFactory.h"
#include "fmod.hpp"
#include "Random.h"
#include "../Utilities/utilities.h" // if you use STL, please include this line AFTER all other include

#define CheckGLError() { GLenum err = glGetError(); if(err) printf("GL Error: %d\nFile: %s\nLine: %d\n\n", err, __FILE__, __LINE__); }

float fpsTimer = 0;
int fps = 0;

int Init ( ESContext *esContext )
{
	Random::Init();
	Reflection::Init();

	glClearColor ( Globals::clearColor.x, Globals::clearColor.y, Globals::clearColor.z, Globals::clearColor.w);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	SceneManager::Init(esContext);
	return 0;
}

void Draw ( ESContext *esContext )
{
	CheckGLError();
	SceneManager::instance->Draw(esContext);
	eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );
}

void Update(ESContext *esContext, float deltaTime)
{
	fps++;
	fpsTimer += deltaTime;
	if (fpsTimer >= 1)
	{
		printf("FPS: %d\n", fps);
		fps = 0;
		fpsTimer -= int(fpsTimer);
	}

	static float elapsed = 0;
	elapsed += deltaTime;
	while (elapsed >= 1.f / 60.f)
	{
		elapsed -= 1.f / 60.f;
		deltaTime = 1.f / 60.f;
		SceneManager::instance->Update(esContext, deltaTime);
	}
	
}

void Key ( ESContext *esContext, unsigned char key, bool bIsPressed)
{
	SceneManager::instance->Key(esContext, key, bIsPressed);
}

void CleanUp()
{
	SceneManager::Destroy();
	ResourceManager::Destroy();
}

int _tmain(int argc, _TCHAR* argv[])
{
	//identifying memory leaks
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); 
	Globals::LoadGlobals();
	ResourceManager::Init();
	ESContext esContext;

    esInitContext ( &esContext );

	esCreateWindow ( &esContext, Globals::windowTitle.c_str(), Globals::screenWidth, Globals::screenHeight, ES_WINDOW_RGB | ES_WINDOW_DEPTH);

	if ( Init ( &esContext ) != 0 )
	{
		printf("\n");
		system("pause");
		return 0;
	}

	esRegisterDrawFunc ( &esContext, Draw );
	esRegisterUpdateFunc ( &esContext, Update );
	esRegisterKeyFunc ( &esContext, Key);

	esMainLoop ( &esContext );

	CleanUp();
	return 0;
}

