precision mediump float;

uniform vec4 uColor;
uniform float uUseColorFromNormal;

varying vec3 vColorFromNormal;

void main()
{
	gl_FragColor = uColor * (1.0 - uUseColorFromNormal) + vec4(vColorFromNormal, 1) * uUseColorFromNormal;
}
