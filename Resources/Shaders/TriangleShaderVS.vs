attribute vec3 aPosition;
attribute vec2 aUV;
attribute vec3 aNormal;
attribute vec3 aBinormal;
attribute vec3 aTangent;

uniform mat4 uMVP;
uniform mat4 uModel;

varying vec3 vPosition;
varying vec2 vTexCoords;
varying mat3 vTBN;

void main()
{
	gl_Position = uMVP * vec4(aPosition, 1);
	vTexCoords = aUV;
	vPosition = (uModel * vec4(aPosition, 1)).xyz;
	vec3 n = normalize((uModel * vec4(aNormal, 0)).xyz);
	vec3 t = normalize((uModel * vec4(aTangent, 0)).xyz);
	t = normalize(t - dot(t, n) * n);
	vec3 b = cross(n, t);
	vTBN = mat3(t, b, n);
}
