#pragma once
#include "../Utilities/utilities.h"

class Shaders 
{
public:
	GLuint program, vertexShader, fragmentShader;
	char fileVS[260];
	char fileFS[260];
	GLint positionAttribute;
	GLint uvAttribute;
	GLint normAttribute;
	GLint binormAttribute;
	GLint tgtAttribute;
	GLint textureUniform;
	GLint modelUniform;

	int Init(char * fileVertexShader, char * fileFragmentShader);
	~Shaders();
};