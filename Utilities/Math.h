#pragma once

#include "esUtil.h"
constexpr float PI = 3.14159265358979323846264338327950288f;

//Vector2

class Vector2
{
public:
	//Constructors
	Vector2() : x(0.0f), y(0.0f){}
	Vector2(GLfloat _x, GLfloat _y) : x(_x), y(_y) {}
	Vector2(GLfloat * pArg) : x(pArg[0]), y(pArg[1]) {}
	Vector2(const Vector2 & vector) : x(vector.x), y(vector.y) {}

	//Vector's operations
	GLfloat Length();
	Vector2 & Normalize();
	Vector2 operator + (Vector2 & vector);
	Vector2 & operator += (Vector2 & vector);
	Vector2 operator - ();
	Vector2 operator - (Vector2 & vector);
	Vector2 & operator -= (Vector2 & vector);
	Vector2 operator * (GLfloat k);
	Vector2 & operator *= (GLfloat k);
	Vector2 operator / (GLfloat k);
	Vector2 & operator /= (GLfloat k);
	Vector2 & operator = (Vector2 & vector);
	Vector2 Modulate(Vector2 & vector);
	GLfloat Dot(Vector2 & vector);
	Vector2 Lerp(Vector2 b, float amountToB);

	//access to elements
	GLfloat& operator [] (unsigned int idx);

	//data members
	GLfloat x;
	GLfloat y;
};

class Vector4;

//Vector3

class Vector3
{
public:
	//Constructors
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	Vector3(GLfloat x) : x(x), y(x), z(x) {}
	Vector3(GLfloat _x, GLfloat _y, GLfloat _z) : x(_x), y(_y), z(_z) {}
	Vector3(GLfloat * pArg) : x(pArg[0]), y(pArg[1]), z(pArg[2]) {}
	Vector3(const Vector3 & vector) : x(vector.x), y(vector.y), z(vector.z) {}
	Vector3(const Vector4& vector);
	
	//Vector's operations
	GLfloat Length();
	GLfloat Length2();
	Vector3 & Normalize();
	Vector3 operator + (const Vector3 & vector) const;
	Vector3 & operator += (Vector3 & vector);
	Vector3 operator - ();
	Vector3 operator - (const Vector3 & vector) const;
	Vector3 & operator -= (Vector3 & vector);
	Vector3 operator * (GLfloat k) const;
	Vector3 & operator *= (GLfloat k);
	Vector3 operator / (GLfloat k) const;
	Vector3 & operator /= (GLfloat k);
	Vector3 & operator = (const Vector3 & vector);
	Vector3 Modulate(Vector3 & vector);
	GLfloat Dot(const Vector3 & vector) const;
	Vector3 Cross(const Vector3 & vector) const;
	Vector3 Lerp(Vector3 b, float amountToB);
	bool operator==(const Vector3& v) const;
	//Vector3& operator=(const Vector3& v)  { x = v.x; y = v.y; z = v.z; }

	//access to elements
	GLfloat& operator [] (unsigned int idx);
	const GLfloat& operator [] (unsigned int idx) const;

	// data members
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

//Vector4

class Matrix;

class Vector4
{
public:
	//Constructors
	Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	Vector4(GLfloat _x, GLfloat _y, GLfloat _z) : x(_x), y(_y), z(_z), w(1.0f) {}
	Vector4(GLfloat _x, GLfloat _y, GLfloat _z, GLfloat _w) : x(_x), y(_y), z(_z), w(_w) {}
	Vector4(GLfloat * pArg) : x(pArg[0]), y(pArg[1]), z(pArg[2]), w(pArg[3]) {}
	Vector4(const Vector3 & vector) : x(vector.x), y(vector.y), z(vector.z), w(1.0f){}
	Vector4(const Vector3 & vector, GLfloat _w) : x(vector.x), y(vector.y), z(vector.z), w(_w) {}
	Vector4(const Vector4 & vector) : x(vector.x), y(vector.y), z(vector.z), w(vector.w) {}

	//Vector's operations
	GLfloat Length();
	Vector4 & Normalize();
	Vector4 operator + (Vector4 & vector);
	Vector4 & operator += (Vector4 & vector);
	Vector4 operator - ();
	Vector4 operator - (Vector4 & vector);
	Vector4 & operator -= (Vector4 & vector);
	Vector4 operator * (GLfloat k);
	Vector4 & operator *= (GLfloat k);
	Vector4 operator / (GLfloat k);
	Vector4 & operator /= (GLfloat k);
	Vector4 & operator = (const Vector4 & vector);
	Vector4 Modulate(Vector4 & vector);
	GLfloat Dot(Vector4 & vector);
	Vector4 Lerp(Vector4 b, float amountToB);
	bool operator==(const Vector4& v) const;

	//matrix multiplication
	Vector4 operator * ( Matrix & m );

	//access to elements
	GLfloat& operator [] (unsigned int idx);

	//data members
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
};

//Matrix 4 X 4


class Matrix
{
public:
	//constructors
	Matrix() {}
	Matrix(GLfloat val);
	Matrix(const Matrix & mat);

	// Matrix operations
	Matrix & SetZero();
	Matrix & SetIdentity();

	Matrix & SetRotationX(GLfloat angle);
	Matrix & SetRotationY(GLfloat angle);
	Matrix & SetRotationZ(GLfloat angle);
	Matrix & SetRotationAngleAxis( float angle, float x, float y, float z );

	Matrix & SetScale(GLfloat scale);
	Matrix & SetScale(GLfloat scaleX, GLfloat scaleY, GLfloat scaleZ);
	Matrix & SetScale(GLfloat * pScale);
	Matrix & SetScale(Vector3 &scaleVec);

	Matrix & SetTranslation(GLfloat x, GLfloat y, GLfloat z);
	Matrix & SetTranslation(GLfloat *pTrans);
	Matrix & SetTranslation(Vector3 &vec);

	Matrix & SetPerspective(GLfloat fovY, GLfloat aspect, GLfloat nearPlane, GLfloat farPlane);
	Matrix & SetInvPerspective(GLfloat fovY, GLfloat aspect, GLfloat nearPlane, GLfloat farPlane);
	Matrix & SetOrthographic(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearPlane, GLfloat farPlane);

	Matrix Transpose();

	Matrix operator + (Matrix & mat);
	Matrix & operator += (Matrix & mat);
	Matrix operator - (Matrix & mat);
	Matrix &operator -= (Matrix & mat);

	Matrix operator * (Matrix & mat);
	Matrix operator * (GLfloat k);
	Matrix & operator *= (GLfloat k);

	Vector4 operator * (Vector4 & vec);

	Matrix & operator = (const Matrix & mat);

	//data members
	GLfloat m[4][4];
};
