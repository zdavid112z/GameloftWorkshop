#include "stdafx.h"
#include "Input.h"

const int Input::LButton = 1;
const int Input::MButton = 2;
const int Input::RButton = 3;

Input::Input(ESContext* esContext)
{
	memset(keys, 0, sizeof(keys));
	memset(mouseButtons, 0, sizeof(mouseButtons));
	UpdateMousePos(esContext);
}

Input::~Input()
{
	
}

void Input::Update(ESContext* esContext, float delta)
{
	UpdateMousePos(esContext);

	if ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
		mouseButtons[1] = true;
	else mouseButtons[1] = false;

	if ((GetKeyState(VK_MBUTTON) & 0x100) != 0)
		mouseButtons[2] = true;
	else mouseButtons[2] = false;

	if ((GetKeyState(VK_RBUTTON) & 0x100) != 0)
		mouseButtons[3] = true;
	else mouseButtons[3] = false;
}

void Input::UpdateMousePos(ESContext* esContext)
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(esContext->hWnd, &p);
	mousePos.x = p.x / (float)esContext->width;
	mousePos.y = (esContext->height - p.y) / (float)esContext->height;
	mousePos *= 2;
	mousePos -= Vector2(1, 1);
}

Vector2 Input::GetMousePos()
{
	return mousePos;
}

void Input::Key(ESContext* esContext, unsigned char key, bool bIsPressed)
{
	keys[key] = bIsPressed;
}

bool Input::KeyDown(unsigned char key)
{
	return keys[key];
}

bool Input::KeyUp(unsigned char key)
{
	return !keys[key];
}

bool Input::IsMouseButtonDown(int index)
{
	return mouseButtons[index];
}

bool Input::IsMouseButtonUp(int index)
{
	return !mouseButtons[index];
}