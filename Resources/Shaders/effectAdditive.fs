precision mediump float;

#define NO_UNIFORMS_OR_VARYING
#include "lighting.glsl"

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;
uniform sampler2D uTexture3;
uniform sampler2D uTexture4;

uniform vec4 uFactors;

varying vec2 vTexCoord;

void main()
{
	vec4 c1 = texture2D(uTexture0, vTexCoord);
	vec4 c2 = texture2D(uTexture1, vTexCoord);
	vec4 c3 = texture2D(uTexture3, vTexCoord);
	vec4 c4 = texture2D(uTexture4, vTexCoord);
	gl_FragColor = vec4(
		vec3(c1) * uFactors.x +
		vec3(c2) * uFactors.y + 
		vec3(c3) * uFactors.z + 
		vec3(c4) * uFactors.w, 1.0);
}
