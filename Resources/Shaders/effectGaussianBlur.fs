precision mediump float;

#define NO_UNIFORMS_OR_VARYING
#include "lighting.glsl"

uniform sampler2D uTexture0;

uniform float uHorizontal;
uniform vec2 uTexOffset;

varying vec2 vTexCoord;

int myabs(int i)
{
	if(i < 0)
		return -i;
	return i;
}

void main()
{
	float kWeights[5];
	kWeights[0] = 0.227027;
	kWeights[1] = 0.1945946;
	kWeights[2] = 0.1216216;
	kWeights[3] = 0.054054;
	kWeights[4] = 0.016216;
	vec4 color = vec4(0);
	vec2 pixelOffset = vec2(uHorizontal * uTexOffset.x, (1.0 - uHorizontal) * uTexOffset.y);
	for(int i = -4; i <= 4; i++)
	{
		vec2 offset = pixelOffset * float(i);
		vec4 c = texture2D(uTexture0, vTexCoord + offset);
		c = GammaCorrectTexture(c);
		color += c * kWeights[myabs(i)];
	}
	gl_FragColor = GammaCorrectFinal(color);
}
