#pragma once

#include "stdafx.h"

class ImageUtils
{
public:
	static byte** SplitCubemap(byte* image, int width, int height, int bpp);
	static byte* Subimage(byte* image, int width, int height, int bpp, int x, int y, int w, int h);
};