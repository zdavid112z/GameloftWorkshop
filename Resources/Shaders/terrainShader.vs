precision highp float;

attribute vec4 aPosition;
attribute vec2 aUV;
attribute vec3 aNormal;
attribute vec3 aBinormal;
attribute vec3 aTangent;

uniform mat4 uMVP;
uniform mat4 uModel;

uniform sampler2D uTexture0;
uniform vec2 uTexMult;
uniform vec3 uHeights;
uniform vec2 uUVOffset;

varying vec2 vTexCoords;
varying vec2 vNormUV;
varying vec3 vPosition;
varying mat3 vTBN;

void main()
{
	vec2 newUV = aUV + uUVOffset;
	vec2 normUV = newUV;
	newUV.x *= uTexMult.x;
	newUV.y *= uTexMult.y;
	vec3 blendmapValue = vec3(texture2D(uTexture0, normUV));
	vec4 position = aPosition;
	position.y += blendmapValue.r * uHeights.r + blendmapValue.g * uHeights.g + blendmapValue.b * uHeights.b;
	vTexCoords = newUV;
	vNormUV = normUV;
	gl_Position = uMVP * position;
	vPosition = (uModel * position).xyz;
	vec3 n = normalize((uModel * vec4(aNormal, 0)).xyz);
	vec3 t = normalize((uModel * vec4(aTangent, 0)).xyz);
	t = normalize(t - dot(t, n) * n);
	vec3 b = cross(n, t);
	vTBN = mat3(t, b, n);
}
