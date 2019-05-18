#include "stdafx.h"
#include "ImageUtils.h"

byte** ImageUtils::SplitCubemap(byte* image, int width, int height, int bpp)
{
	int s = width / 4;
	byte** res = new byte*[6];
	res[0] = Subimage(image, width, height, bpp, 2 * s, s, s, s);
	res[1] = Subimage(image, width, height, bpp, 0, s, s, s);
	res[2] = Subimage(image, width, height, bpp, s, 2 * s, s, s);
	res[3] = Subimage(image, width, height, bpp, s, 0, s, s);
	res[4] = Subimage(image, width, height, bpp, s, s, s, s);
	res[5] = Subimage(image, width, height, bpp, 3 * s, s, s, s);
	return res;
}

byte* ImageUtils::Subimage(byte* image, int width, int height, int bpp, int x, int y, int w, int h)
{
	byte* res = new byte[bpp / 8 * w * h];
	for (int i = 0; i < h; i++)
		memcpy(res + (h - i - 1) * w * bpp / 8, image + (i + y) * width * bpp / 8 + x * bpp / 8, w * bpp / 8);
	return res;
}
