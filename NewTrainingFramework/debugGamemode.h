#pragma once

#include "Gamemode.h"

class DebugGamemode : public Gamemode
{
public:
	DebugGamemode();
	~DebugGamemode();
	virtual void Update(ESContext* context, float delta) override;
	virtual void Draw(ESContext* context) override;
	virtual void Key(ESContext *esContext, unsigned char key, bool bIsPressed) override;

protected:
	void DebugCameraUpdate(ESContext* context, float delta);
	void DebugCameraKey(ESContext *esContext, unsigned char key, bool bIsPressed);
	void CenterCursor(ESContext* esContext);
protected:
	bool canMove = true;
	bool canLook = true;
	bool keys[1000];
	bool moveKeysPressed[10];
	bool mouseGrabbed = false;
	int mouseX, mouseY, screenWidth2, screenHeight2;
	float cmouseX, cmouseY;
};