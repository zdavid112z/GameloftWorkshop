precision mediump float;

uniform samplerCube uTexture0;

varying vec3 vPosition;

void main()
{
	gl_FragColor = textureCube(uTexture0, normalize(vPosition));
}
