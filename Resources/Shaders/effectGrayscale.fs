precision mediump float;

#define NO_UNIFORMS_OR_VARYING
#include "lighting.glsl"

uniform sampler2D uTexture0;

varying vec2 vTexCoord;

void main()
{
	vec4 color = GammaCorrectTexture(texture2D(uTexture0, vTexCoord));
	float value = (color.x + color.y + color.z) / 3.0;
	gl_FragColor = GammaCorrectFinal(vec4(vec3(value), 1));
}
