#include "stdafx.h"
#include "Shader.h"
#include "ResourceManager.h"
#include "Vertex.h"
#include "Texture.h"
#include "SceneManager.h"

Shader::Shader(ShaderResource* res)
{
	resource = res;
	std::string vertexSource = PreprocessSource(Utils::ReadFile(res->vertexPath));
	std::string fragmentSource = PreprocessSource(Utils::ReadFile(res->fragmentPath));
	GLuint vert = esLoadShader(GL_VERTEX_SHADER, vertexSource.c_str(), res->vertexPath.c_str());
	ASSERT(vert, "Could not compile vertex shader from file %s!\nSource:\n%s", res->vertexPath.c_str(), Utils::AddLineNumbers(vertexSource).c_str());
	GLuint frag = esLoadShader(GL_FRAGMENT_SHADER, fragmentSource.c_str(), res->fragmentPath.c_str());
	ASSERT(frag, "Could not compile fragment shader from file %s!\nSource:\n%s", res->fragmentPath.c_str(), Utils::AddLineNumbers(fragmentSource).c_str());
	program = esLoadProgram(vert, frag);
	ASSERT(program, "Could not create program!");
	glDeleteShader(vert);
	glDeleteShader(frag);
	Bind();
	positionAttribute = glGetAttribLocation(program, "aPosition");
	uvAttribute = glGetAttribLocation(program, "aUV");
	normalAttribute = glGetAttribLocation(program, "aNormal");
	binormalAttribute = glGetAttribLocation(program, "aBinormal");
	tangentAttribute = glGetAttribLocation(program, "aTangent");
	envMapSlot = -1;
	for (int i = 0; i < NUM_TEXTURES; i++)
	{
		textureUniforms[i] = glGetUniformLocation(program, ("uTexture" + std::to_string(i)).c_str());
		if (textureUniforms[i] == -1)
		{
			if (envMapSlot == -1)
			{
				envMapSlot = i;
				envMap = glGetUniformLocation(program, "uEnvMap");
				glUniform1i(envMap, i);
			}
		}
		else glUniform1i(textureUniforms[i], i);
	}
}

Shader::~Shader()
{
	glDeleteProgram(program);
}

uint32 Shader::GetId() 
{
	return resource->id; 
}

void Shader::Bind()
{
	glUseProgram(program);
}

void Shader::BindCommonAttributes()
{
	if (positionAttribute != -1)
	{
		glEnableVertexAttribArray(positionAttribute);
		glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	}
	if (uvAttribute != -1)
	{
		glEnableVertexAttribArray(uvAttribute);
		glVertexAttribPointer(uvAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	}
	if (normalAttribute != -1)
	{
		glEnableVertexAttribArray(normalAttribute);
		glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, norm));
	}
	if (binormalAttribute != -1)
	{
		glEnableVertexAttribArray(binormalAttribute);
		glVertexAttribPointer(binormalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, binorm));
	}
	if (tangentAttribute != -1)
	{
		glEnableVertexAttribArray(tangentAttribute);
		glVertexAttribPointer(tangentAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tgt));
	}
}

void Shader::Uniform1i(const std::string& s, int v)
{
	GLint id = GetUniform(s);
	if (id != -1)
		glUniform1i(id, v);
}

void Shader::Uniform1f(const std::string& s, float v)
{
	GLint id = GetUniform(s);
	if (id != -1)
		glUniform1f(id, v);
}

void Shader::Uniform2f(const std::string& s, Vector2 v)
{
	GLint id = GetUniform(s);
	if (id != -1)
		glUniform2f(id, v.x, v.y);
}

void Shader::Uniform3f(const std::string& s, Vector3 v)
{
	GLint id = GetUniform(s);
	if (id != -1)
		glUniform3f(id, v.x, v.y, v.z);
}

void Shader::Uniform4f(const std::string& s, Vector4 v)
{
	GLint id = GetUniform(s);
	if (id != -1)
		glUniform4f(id, v.x, v.y, v.z, v.w);
}

void Shader::UniformMatrix(const std::string& s, Matrix v)
{
	GLint id = GetUniform(s);
	if (id != -1)
		glUniformMatrix4fv(id, 1, GL_FALSE, (GLfloat*)v.m);
}

void Shader::UniformDirLight(const std::string& s, DirectionalLight l)
{
	Uniform4f(s + ".direction", l.direction);
	Uniform4f(s + ".color", l.color);
}

void Shader::UniformPointLight(const std::string& s, PointLight l)
{
	Uniform4f(s + ".position", l.position);
	Uniform4f(s + ".color", l.color);
	Uniform4f(s + ".attenuation", l.attenuation);
}

void Shader::UniformSpotLight(const std::string& s, SpotLight l)
{
	Uniform4f(s + ".position", l.position);
	Uniform4f(s + ".direction", l.direction);
	Uniform4f(s + ".color", l.color);
	Uniform4f(s + ".attenuation", l.attenuation);
	Uniform1f(s + ".angleCosMin", l.angleCosMin);
	Uniform1f(s + ".angleCosMax", l.angleCosMax);
}

void Shader::UniformPointLights(const std::string& countName, const std::string& arrayName, PointLight* l, int n)
{
	Uniform1i(countName, n);
	for (int i = 0; i < n; i++)
		UniformPointLight(arrayName + "[" + std::to_string(i) + "]", l[i]);
}

void Shader::UniformSpotLights(const std::string& countName, const std::string& arrayName, SpotLight* l, int n)
{
	Uniform1i(countName, n);
	for (int i = 0; i < n; i++)
		UniformSpotLight(arrayName + "[" + std::to_string(i) + "]", l[i]);
}

void Shader::BindSceneData()
{
	auto*& s = SceneManager::instance;
	if(s->dirLight)
		UniformDirLight("uDirLight", *s->dirLight);
	Uniform1i("uNumPointLights", s->pointLights.size());
	Uniform1i("uNumSpotLights", s->spotLights.size());
	for (uint i = 0; i < s->pointLights.size(); i++)
		UniformPointLight("uPointLights[" + std::to_string(i) + "]", *s->pointLights[i]);
	for (uint i = 0; i < s->spotLights.size(); i++)
		UniformSpotLight("uSpotLights[" + std::to_string(i) + "]", *s->spotLights[i]);
	Uniform4f("uAmbientLight", s->ambientLight);
	Uniform3f("uFogColor", s->fogColor);
	Uniform1f("uFogStartRadius", s->fogStartRadius);
	Uniform1f("uFogEndRadius", s->fogEndRadius);
	Uniform3f("uCameraPosition", s->activeCamera->data.position);
}

GLuint Shader::GetUniform(const std::string& s)
{
	auto it = uniforms.find(s);
	if (it == uniforms.end())
	{
		GLint val = glGetUniformLocation(program, s.c_str());
		uniforms[s] = val;
		return val;
	}
	return it->second;
}

/*
void Shader::BindTexMult(const Vector2& v)
{
	if (texMult != -1)
	{
		glUniform2f(texMult, v.x, v.y);
	}
}

void Shader::BindMVPMatrix(const Matrix& mat)
{
	if (mvpUniform != -1)
	{
		glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, (GLfloat*)mat.m);
	}
}

void Shader::BindModelMatrix(const Matrix& mat)
{
	if (modelUniform != -1)
	{
		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, (GLfloat*)mat.m);
	}
}

void Shader::BindHeights(const Vector3& v)
{
	if (heightsUniform!= -1)
	{
		glUniform3f(heightsUniform, v.x, v.y, v.z);
	}
}

void Shader::BindTexture(Texture* texture, int slot)
{
	if (textureUniforms[slot] != -1)
	{
		texture->Bind(slot);
		glUniform1i(textureUniforms[slot], slot);
	}
}

void Shader::BindUVOffset(const Vector2& v)
{
	if (uvOffset != -1)
	{
		glUniform2f(uvOffset, v.x, v.y);
	}
}

void Shader::BindFogColor(const Vector3& v)
{
	if (fogColor != -1)
	{
		glUniform3f(fogColor, v.x, v.y, v.z);
	}
}

void Shader::BindFogStartRadius(float v)
{
	if (fogStartRadius != -1)
	{
		glUniform1f(fogStartRadius, v);
	}
}

void Shader::BindFogEndRadius(float v)
{
	if (fogEndRadius != -1)
	{
		glUniform1f(fogEndRadius, v);
	}
}

void Shader::BindCameraPosition(const Vector3& v)
{
	if (cameraPosition != -1)
	{
		glUniform3f(cameraPosition, v.x, v.y, v.z);
	}
}

void Shader::BindTime(float v)
{
	if (time != -1)
	{
		glUniform1f(time, v);
	}
}

void Shader::BindDispMax(float v)
{
	if (dispMax != -1)
	{
		glUniform1f(dispMax, v);
	}
}

void Shader::BindSkyboxYOffset(float v)
{
	if (skyboxYOffset!= -1)
	{
		glUniform1f(skyboxYOffset, v);
	}
}

void Shader::BindShininess(float v)
{
	if (shininess != -1)
	{
		glUniform1f(shininess, v);
	}
}

void Shader::BindEnvMap(Texture* texture)
{
	texture->Bind(envMapSlot);
}
*/

void Shader::BindTexture(Texture* texture, int slot)
{
	if (textureUniforms[slot] != -1)
	{
		texture->Bind(slot);
		glUniform1i(textureUniforms[slot], slot);
	}
}

void Shader::BindEnvMap(Texture* texture)
{
	texture->Bind(envMapSlot);
}

// TODO: does not handle recursive inclusion
std::string Shader::PreprocessSource(const std::string& src)
{
	std::string result;
	size_t pos = 0, last = 0;
	while ((pos = src.find("#include", last)) != std::string::npos)
	{
		result += src.substr(last, pos - last);
		pos += 8; // move after #include
		// skip spaces
		while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t'))
			pos++;
		ASSERT(pos < src.size() && (src[pos] == '"' || src[pos] == '<'), "Could not preprocess a shader source! Expected '\"' or '<' after #include");
		pos++;
		size_t begin = pos; // of the included file
		while (pos < src.size() && src[pos] != '"' && src[pos] != '>')
			pos++; // skip until there is a closing symbol
		ASSERT(pos < src.size(), "Could not preprocess a shader source! Expected '\"' or '>' after #include <path>");
		// get the included file's name
		std::string includedFile = src.substr(begin, pos - begin);
		result += '\n'; // safety
		result += PreprocessSource(Utils::ReadFile("../Resources/Shaders/" + includedFile));
		result += '\n'; // safety
		// go after the closing symbol
		pos++;
		last = pos;
	}
	result += src.substr(last);
	return  result;
}
