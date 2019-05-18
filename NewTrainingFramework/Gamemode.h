#pragma once

#include "Player.h"

class Gamemode
{
public:
	Gamemode();
	virtual ~Gamemode();
	virtual void Update(ESContext* context, float delta);
	virtual void Draw(ESContext* context);
	virtual void Key(ESContext *esContext, unsigned char key, bool bIsPressed);
};