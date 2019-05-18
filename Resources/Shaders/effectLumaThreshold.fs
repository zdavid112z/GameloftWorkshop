precision mediump float;

#define NO_UNIFORMS_OR_VARYING
#include "lighting.glsl"

uniform sampler2D uTexture0;
uniform vec2 uThreshold;

varying vec2 vTexCoord;

float mystep(float tmin, float tmax, float x)
{
	if(x < tmin)
		return 0.0;
	if(x > tmax)
		return 1.0;
	float xm = x - 1.0;
	xm = xm * xm;
	xm = xm * xm;
	xm = xm * xm;
	xm = xm * xm;
	return 1.0 - xm;
}

void main()
{
	vec4 color = texture2D(uTexture0, vTexCoord);
	float value = luma(color);
	value = mystep(uThreshold.x, uThreshold.y, value);
	//value = step(uThreshold.x, value);
	gl_FragColor = vec4(vec3(value), 1);
}
