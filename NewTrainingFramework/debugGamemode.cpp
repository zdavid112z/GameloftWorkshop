#include "stdafx.h"
#include "debugGamemode.h"
#include "SceneManager.h"
#include "Globals.h"
#include <Windows.h>

DebugGamemode::DebugGamemode()
{
	memset(moveKeysPressed, 0, sizeof(moveKeysPressed));
	memset(keys, 0, sizeof(keys));
}

DebugGamemode::~DebugGamemode()
{

}

void DebugGamemode::CenterCursor(ESContext* esContext)
{
	RECT r;
	GetWindowRect(esContext->hWnd, &r);
	mouseX = (r.right + r.left) / 2;
	mouseY = (r.bottom + r.top) / 2;
	SetCursorPos(mouseX, mouseY);
}

void DebugGamemode::Update(ESContext* esContext, float deltaTime)
{
	for (auto it = SceneManager::instance->objects.begin(); it != SceneManager::instance->objects.end(); it++)
	{
		if (it->second->noCollision)
			continue;
		SceneObject* a = it->second;
		AABB aabb1 = a->GetAABB();
		if (aabb1.Inside(SceneManager::instance->activeCamera->data.position))
		{
			printf("The camera is inside %s!\n", a->name.c_str());
			Sound* s = ResourceManager::instance->LoadSound("toc");
			s->Play();
		}
		auto it2 = it;
		it2++;
		for (; it2 != SceneManager::instance->objects.end(); it2++)
		{
			if (it2->second->noCollision)
				continue;
			SceneObject* b = it2->second;
			if (a == b)
				continue;
			AABB aabb2 = b->GetAABB();
			if (aabb1.Collides(aabb2))
			{
				//printf("%s collides with %s!\n", a->name.c_str(), b->name.c_str());
			}
		}
	}

	DebugCameraUpdate(esContext, deltaTime);
}

void DebugGamemode::Draw(ESContext* context)
{

}

void DebugGamemode::Key(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	DebugCameraKey(esContext, key, bIsPressed);
}

void DebugGamemode::DebugCameraUpdate(ESContext* esContext, float deltaTime)
{
	Camera* camera = SceneManager::instance->activeCamera;
	if (camera != nullptr)
	{
		camera->BeginUpdate(deltaTime);
		if (canMove)
		{
			if (moveKeysPressed[0])
				camera->MoveX(1);
			if (moveKeysPressed[1])
				camera->MoveX(-1);
			if (moveKeysPressed[2])
				camera->MoveY(1);
			if (moveKeysPressed[3])
				camera->MoveY(-1);
			if (moveKeysPressed[4])
				camera->MoveZ(1);
			if (moveKeysPressed[5])
				camera->MoveZ(-1);
		}
		if (mouseGrabbed && canLook)
		{
			POINT p;
			GetCursorPos(&p);
			int deltaX = p.x - mouseX;
			int deltaY = p.y - mouseY;
			CenterCursor(esContext);
			camera->RotateX((float)deltaY);
			camera->RotateY((float)deltaX);
		}
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(esContext->hWnd, &p);
		cmouseX = p.x / (float)esContext->width;
		cmouseY = (esContext->height - p.y) / (float)esContext->height;
		camera->UpdateViewProjection();
	}
}

void DebugGamemode::DebugCameraKey(ESContext *esContext, unsigned char key, bool bIsPressed)
{
	const unsigned char moveKeys[] = { 'D', 'A', 'E', 'Q', 'W', 'S' };
	keys[key] = bIsPressed;
	if (key == 'F' && bIsPressed)
	{
		if (SceneManager::instance->activeCamera)
		{
			Camera* c = SceneManager::instance->activeCamera;
			if (c->aspect == 1.f)
				c->aspect = float(Globals::screenWidth) / Globals::screenHeight;
			else c->aspect = 1.f;
			c->UpdateProjection();
		}
	}
	if (key == 'G' && bIsPressed)
	{
		if (SceneManager::instance->activeCamera)
		{
			Camera* c = SceneManager::instance->activeCamera;
			printf("<position>\n<x>%f</x>\n<y>%f</y>\n<z>%f</z>\n</position>\n", c->data.position.x, c->data.position.y, c->data.position.z);
			printf("<up>\n<x>%f</x>\n<y>%f</y>\n<z>%f</z>\n</up>\n", c->data.up.x, c->data.up.y, c->data.up.z);
			printf("<target>\n<x>%f</x>\n<y>%f</y>\n<z>%f</z>\n</target>\n", c->data.position.x - c->data.forward.x, c->data.position.y - c->data.forward.y, c->data.position.z - c->data.forward.z);
			printf("<direction>\n<x>%f</x>\n<y>%f</y>\n<z>%f</z>\n</direction>\n", -c->data.forward.x, -c->data.forward.y, -c->data.forward.z);
		}
	}
	for (int i = 0; i < 6; i++)
	{
		if (key == moveKeys[i])
		{
			if (bIsPressed)
				moveKeysPressed[i] = true;
			else moveKeysPressed[i] = false;
			return;
		}
	}
	if (key == ' ')
		if (bIsPressed)
			moveKeysPressed[2] = true;
		else moveKeysPressed[2] = false;
	if (key == 'R' && bIsPressed)
	{
		mouseGrabbed = !mouseGrabbed;
		if (mouseGrabbed)
		{
			CenterCursor(esContext);
			//ShowCursor(false);
		}
	}
	else if (key == 'R')
	{
		//ShowCursor(true);
	}
	else if (key == 'T' && bIsPressed)
	{
		SceneManager::instance->showAABBs = !SceneManager::instance->showAABBs;
	}
	else if (key == 'Y' && bIsPressed)
	{
		SceneManager::instance->showDebug = !SceneManager::instance->showDebug;
	}
	else if (key == 'U' && bIsPressed)
	{
		SceneManager::instance->Delete();
		static int x = 0;
		x++;
		if(x % 2 == 0)
			SceneManager::instance->Reload("bulletHell.xml");
		else SceneManager::instance->Reload();
;	}
	else if (key == 'B' && bIsPressed)
	{
		for (auto p : SceneManager::instance->objects)
		{
			printf("%s\n", p.second->name.c_str());
		}
		printf("\n");
	}
}