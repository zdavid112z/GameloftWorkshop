
#include "stdafx.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Postprocess.h"

Camera::Camera(float _znear, float _zfar, float _fov, float _aspect, float _moveSpeed, float _rotateSpeed) :
	hasPerspective(true),
	znear(_znear),
	zfar(_zfar),
	fov(_fov),
	aspect(_aspect),
	moveSpeed(_moveSpeed),
	rotateSpeed(_rotateSpeed)
{
	data.position = Vector3(0, 0, 1);
	data.forward = Vector3(0, 0, 1);
	data.up = Vector3(0, 1, 0);
	data.right = Vector3(1, 0, 0);
	UpdateProjection();
}

Camera::Camera(float _znear, float _zfar, Vector4 ortho) :
	hasPerspective(false),
	znear(_znear),
	zfar(_zfar),
	ortho(ortho)
{
	data.position = Vector3(0, 0, 1);
	data.forward = Vector3(0, 0, 1);
	data.up = Vector3(0, 1, 0);
	data.right = Vector3(1, 0, 0);
	UpdateProjection();
}

Camera::~Camera()
{
	if (targetFramebuffer)
		delete targetFramebuffer;
	for (auto p : postprocessEffects)
		delete p;
}

void Camera::UpdateProjection()
{
	if(hasPerspective)
		projection.SetPerspective(fov, aspect, znear, zfar);
	else projection.SetOrthographic(ortho.x, ortho.y, ortho.z, ortho.w, znear, zfar);
}

void Camera::BeginUpdate(float delta)
{
	deltaTime = delta;
}

void Camera::MoveX(float amount)
{
	data.position += data.right * amount * deltaTime * moveSpeed;
}

void Camera::MoveY(float amount)
{
	data.position += data.up * amount * deltaTime * moveSpeed;
}

void Camera::MoveZ(float amount)
{
	data.position -= data.forward * amount * deltaTime * moveSpeed;
}

void Camera::RotateX(float amount)
{
	float angle = amount * rotateSpeed;
	Matrix rotMatrix;
	rotMatrix.SetRotationAngleAxis(angle, data.right.x, data.right.y, data.right.z);
	data.forward = Vector3(rotMatrix * Vector4(data.forward, 0)).Normalize();
	data.up = data.forward.Cross(data.right).Normalize();
}

void Camera::RotateY(float amount)
{
	float angle = amount * rotateSpeed;
	Matrix rotMatrix;
	rotMatrix.SetRotationY(angle);
	data.right = Vector3(rotMatrix * Vector4(data.right, 0)).Normalize();
	data.forward = Vector3(rotMatrix * Vector4(data.forward, 0)).Normalize();
	data.up = data.forward.Cross(data.right).Normalize();
}

void Camera::UpdateViewProjection()
{
	Matrix view;
	view.m[0][0] = data.right.x;   view.m[1][0] = data.right.y;	  view.m[2][0] = data.right.z;   view.m[3][0] = 0;
	view.m[0][1] = data.up.x;      view.m[1][1] = data.up.y;	  view.m[2][1] = data.up.z;      view.m[3][1] = 0;
	view.m[0][2] = data.forward.x; view.m[1][2] = data.forward.y; view.m[2][2] = data.forward.z; view.m[3][2] = 0;
	view.m[0][3] = 0;			   view.m[1][3] = 0;			  view.m[2][3] = 0;		         view.m[3][3] = 1;
	Matrix t;
	t.SetTranslation(-data.position);
	view = t * view;
	viewProjection = view * projection;
}

Matrix Camera::GetInvViewProjection()
{
	Matrix view;
	view.m[0][0] = data.right.x;   view.m[1][0] = data.right.y;	  view.m[2][0] = data.right.z;   view.m[3][0] = 0;
	view.m[0][1] = data.up.x;      view.m[1][1] = data.up.y;	  view.m[2][1] = data.up.z;      view.m[3][1] = 0;
	view.m[0][2] = data.forward.x; view.m[1][2] = data.forward.y; view.m[2][2] = data.forward.z; view.m[3][2] = 0;
	view.m[0][3] = 0;			   view.m[1][3] = 0;			  view.m[2][3] = 0;		         view.m[3][3] = 1;
	view = view.Transpose();
	Matrix t;
	t.SetTranslation(data.position);
	view = view * t;
	Matrix invProjection;
	invProjection.SetInvPerspective(fov, aspect, znear, zfar);
	return invProjection * view;
}

Matrix Camera::GetModel()
{
	Matrix view;
	view.m[0][0] = data.right.x;   view.m[0][1] = data.right.y;	view.m[0][2] = data.right.z;   view.m[0][3] = 0;
	view.m[1][0] = data.up.x;      view.m[1][1] = data.up.y;		view.m[1][2] = data.up.z;      view.m[1][3] = 0;
	view.m[2][0] = data.forward.x; view.m[2][1] = data.forward.y; view.m[2][2] = data.forward.z; view.m[2][3] = 0;
	view.m[3][0] = 0;         view.m[3][1] = 0;         view.m[3][2] = 0;         view.m[3][3] = 1;
	Matrix t;
	t.SetTranslation(data.position);
	view = view * t;
	return view;
}

Matrix Camera::GetViewProjection() const
{
	return viewProjection;
}

Framebuffer* Camera::GetOutput()
{
	if (postprocessEffects.empty())
		return targetFramebuffer;
	return postprocessEffects.back()->output;
}

PostprocessEffect* Camera::GetEffect(PostprocessEffectDesc desc, PostprocessEffect* start)
{
	if (postprocessEffects.empty())
		return nullptr;
	if (desc.index >= 0)
	{
		for (uint i = 0; i < postprocessEffects.size(); i++)
		{
			if (desc.id == postprocessEffects[i]->typeId || desc.id == 0)
				desc.index--;
			if (desc.index == -1)
				return postprocessEffects[i];
		}
		return postprocessEffects.back();
	}
	else
	{
		int startIndex;
		for (startIndex = 0; startIndex < (int)postprocessEffects.size(); startIndex++)
			if (postprocessEffects[startIndex] == start)
				break;

		for (int i = startIndex - 1; i >= 0; i--)
		{
			if (desc.id == postprocessEffects[i]->typeId || desc.id == 0)
				desc.index++;
			if (desc.index == 0)
				return postprocessEffects[i];
		}
		return postprocessEffects[0];
	}
	return nullptr; //?
}