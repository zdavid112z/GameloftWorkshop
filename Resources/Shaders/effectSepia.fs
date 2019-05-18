precision mediump float;

#define NO_UNIFORMS_OR_VARYING
#include "lighting.glsl"

uniform sampler2D uTexture0;

varying vec2 vTexCoord;

void main()
{
	vec4 color = (texture2D(uTexture0, vTexCoord));
	vec3 final;
	final.r = (color.r * .393) + (color.g *.769) + (color.b * .189);
	final.g = (color.r * .349) + (color.g *.686) + (color.b * .168);
	final.b = (color.r * .272) + (color.g *.534) + (color.b * .131);
	gl_FragColor = (vec4(final, 1));
}
