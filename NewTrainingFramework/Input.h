#pragma once

#include "../Utilities/utilities.h"

class Input
{
public:
	Input(ESContext* esContext);
	~Input();

	void Update(ESContext* esContext, float delta);
	void Key(ESContext* esContext, unsigned char key, bool bIsPressed);

	bool KeyDown(unsigned char key);
	bool KeyUp(unsigned char key);
	bool IsMouseButtonDown(int index);
	bool IsMouseButtonUp(int index);
	Vector2 GetMousePos();

	static const int LButton;
	static const int MButton;
	static const int RButton;
private:
	void UpdateMousePos(ESContext* esContext);
private:
	bool mouseButtons[5];
	Vector2 mousePos;
	bool keys[260];
};