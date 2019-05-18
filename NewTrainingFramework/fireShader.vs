attribute vec4 aPosition;
attribute vec2 aUV;
attribute vec3 aNormal;
attribute vec3 aBinormal;
attribute vec3 aTangent;

uniform mat4 uMVP;
uniform mat4 uModel;

varying vec2 vUV;
varying vec3 vPosition;

void main()
{
	vUV = aUV;
	vPosition = (uModel * aPosition).xyz;
	gl_Position = uMVP * aPosition;
}
