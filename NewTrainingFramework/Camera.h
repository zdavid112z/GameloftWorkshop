#pragma once

#include "../Utilities/Math.h"
#include <vector>
#include "Postprocess.h"

class PostprocessEffect;
class Framebuffer;

struct CameraData
{
	Vector3 position;
	Vector3 up, forward, right;
};

enum class CullFace
{
	NONE, FRONT, BACK
};

class Camera
{
public:
	Camera() {}
	Camera(float _znear, float _zfar, float _fov, float _aspect, float _moveSpeed = 1, float _rotateSpeed = 0.01f);
	Camera(float _znear, float _zfar, Vector4 ortho);
	~Camera();
	void BeginUpdate(float delta);
	void MoveX(float amount);
	void MoveY(float amount);
	void MoveZ(float amount);
	void RotateX(float amount);
	void RotateY(float amount);
	// aka. EndUpdate()
	void UpdateViewProjection();
	Matrix GetInvViewProjection();
	void UpdateProjection();
	Matrix GetViewProjection() const;
	Matrix GetModel();
	Framebuffer* GetOutput();
	PostprocessEffect* GetEffect(PostprocessEffectDesc, PostprocessEffect*);
public:
	CullFace cullFace;
	bool viewsDebug;
	Vector4 clearColor;
	bool clearColorOnBegin, clearDepthOnBegin, clearStencilOnBegin;
	bool hasSkybox;
	float znear, zfar, fov, aspect;
	float moveSpeed;
	float rotateSpeed;
	bool hasPerspective;
	Vector4 ortho;
	Matrix projection;
	CameraData data;
	std::vector<PostprocessEffect*> postprocessEffects;
	Framebuffer* targetFramebuffer;
	float priority;
private:
	float deltaTime;
	Matrix viewProjection;
};

