#pragma once

#include "../Utilities/utilities.h"
#include "../Utilities/Math.h"
#include "Light.h"
#include <string>
#include <unordered_map>
#define NUM_TEXTURES 16

class ShaderResource;
class Model;
class Texture;

class Shader
{
	friend class Model;
public:
	Shader(ShaderResource* res);
	~Shader();
	void Bind();
	void BindCommonAttributes();
	/*void BindHeights(const Vector3& v);
	void BindTexMult(const Vector2& v);
	void BindMVPMatrix(const Matrix& mat);
	void BindModelMatrix(const Matrix& mat);
	void BindUVOffset(const Vector2& v);
	void BindFogColor(const Vector3& v);
	void BindFogStartRadius(float v);
	void BindFogEndRadius(float v);
	void BindCameraPosition(const Vector3& v);
	void BindTime(float v);
	void BindDispMax(float v);
	void BindSkyboxYOffset(float v);
	void BindShininess(float v);
	void BindSize(Vector3 v);
	void BindColor(Vector4 v);*/
	void BindEnvMap(Texture* texture);
	void BindTexture(Texture* texture, int slot = 0);

	void Uniform1i(const std::string& s, int v);
	void Uniform1f(const std::string& s, float v);
	void Uniform2f(const std::string& s, Vector2 v);
	void Uniform3f(const std::string& s, Vector3 v);
	void Uniform4f(const std::string& s, Vector4 v);
	void UniformMatrix(const std::string& s, Matrix v);
	void UniformDirLight(const std::string& s, DirectionalLight l);
	void UniformPointLight(const std::string& s, PointLight l);
	void UniformSpotLight(const std::string& s, SpotLight l);
	void UniformPointLights(const std::string& countName, const std::string& arrayName, PointLight* l, int n);
	void UniformSpotLights(const std::string& countName, const std::string& arrayName, SpotLight* l, int n);
	void BindSceneData();
	uint32 GetId();
private:
	std::string PreprocessSource(const std::string& s);
	GLuint GetUniform(const std::string& s);

	ShaderResource* resource;
	GLuint program;
	GLint positionAttribute;
	GLint uvAttribute;
	GLint normalAttribute;
	GLint binormalAttribute;
	GLint tangentAttribute;
	GLint envMap;
	GLint textureUniforms[NUM_TEXTURES];
	int envMapSlot;
	std::unordered_map<std::string, GLint> uniforms;
};
