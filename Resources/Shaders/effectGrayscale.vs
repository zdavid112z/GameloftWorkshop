attribute vec4 aPosition;
attribute vec2 aUV;
attribute vec3 aNormal;
attribute vec3 aBinormal;
attribute vec3 aTangent;

varying vec2 vTexCoord;

void main()
{
	gl_Position = vec4(aPosition.xyz, 1.0);
	vTexCoord = aPosition.xy / 2.0 + 0.5;
}
