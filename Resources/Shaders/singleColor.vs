attribute vec4 aPosition;
attribute vec2 aUV;
attribute vec3 aNormal;
attribute vec3 aBinormal;
attribute vec3 aTangent;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform vec3 uSize;

varying vec3 vColorFromNormal;

void main()
{
	vec3 pos = aPosition.xyz;
	pos.x *= uSize.x;
	pos.y *= uSize.y;
	pos.z *= uSize.z;
	gl_Position = uMVP * vec4(pos, 1.0);
	vColorFromNormal = aNormal;
}
