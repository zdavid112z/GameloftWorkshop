precision mediump float;

#include "lighting.glsl"

// Diffuse map
uniform sampler2D uTexture0;
// Specular map
uniform sampler2D uTexture1;
// Normal map
uniform sampler2D uTexture2;

uniform sampler2D uTexture3;

uniform samplerCube uEnvMap;

uniform vec2 uShininess;

void main()
{
	vec4 diffuse = texture2D(uTexture0, vTexCoords);
	vec4 specular = texture2D(uTexture1, vTexCoords);
	vec3 normal = CalcNormal(texture2D(uTexture2, vTexCoords).rgb);
	vec3 dirReflect = reflect(normalize(vPosition - uCameraPosition), normal);
	vec3 reflectColor = textureCube(uEnvMap, dirReflect).rgb;
	float shininess = uShininess.x * uShininess.y + (1.0 - uShininess.y) * specular.a;
	diffuse.rgb = diffuse.rgb * (1.0 - shininess) + reflectColor * shininess;
	
	vec3 lightColor = CalcLight(diffuse.rgb, specular.rgb, normal, uTexture3);
	// TODO: alpha handling that works when the diffuse texture is not used for color, but bound from the previous object
	vec4 finalColor = vec4(lightColor, CalcAlphaFromAfterFog()); // alpha = CalcAlphaFromAfterFog() * diffuse.a
	// if(finalColor.a < 0.1)
	// 	discard;
	gl_FragColor = finalColor;
}
