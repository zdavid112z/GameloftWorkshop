precision mediump float;

uniform sampler2D uTexture0;
uniform sampler2D uTexture1;
uniform sampler2D uTexture2;

uniform vec3 uFogColor;
uniform float uFogStartRadius;
uniform float uFogEndRadius;
uniform vec3 uCameraPosition;
uniform float uTime;
uniform float uDispMax;

varying vec2 vUV;
varying vec3 vPosition;
varying vec3 vAPosition;

void main()
{
	vec2 disp = texture2D(uTexture1, vec2(vUV.x, vUV.y - uTime + vAPosition.x / 5.0)).rg;
	vec2 offset = disp * uDispMax * 2.0 - vec2(uDispMax, uDispMax);
	vec4 color = texture2D(uTexture0, vUV + offset);
	float alpha = texture2D(uTexture2, vec2(vUV.x, 1.0 - vUV.y)).r;
	color.a *= alpha;

	float dist = distance(vPosition, uCameraPosition);
	float fogAlpha = clamp(1.0 - (dist - uFogStartRadius) / (uFogEndRadius - uFogStartRadius), 0.0, 1.0);
	color.a *= fogAlpha;

	if(color.a < 1e-2)
		discard;
	gl_FragColor = color;
}
