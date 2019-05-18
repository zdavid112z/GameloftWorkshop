precision highp float;

#include "lighting.glsl"

// Blend texture
uniform sampler2D uTexture0;
// RGB Textures
uniform sampler2D uTexture1;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
// Shadowmap
uniform sampler2D uTexture4;

uniform samplerCube uEnvMap;

uniform vec2 uShininess;
uniform mat4 uShadowMVP;

varying vec2 vNormUV;

void main()
{
	vec3 blendmapValue = vec3(texture2D(uTexture0, vNormUV));
	vec3 tex1 = vec3(texture2D(uTexture1, vTexCoords));
	vec3 tex2 = vec3(texture2D(uTexture2, vTexCoords));
	vec3 tex3 = vec3(texture2D(uTexture3, vTexCoords));
	vec3 color = blendmapValue.r * tex1 + blendmapValue.g * tex2 + blendmapValue.b * tex3;

	vec3 normal = CalcNormal(vec3(0, 0, 1));
	vec3 dirReflect = reflect(normalize(vPosition - uCameraPosition), normal);
	vec3 reflectColor = textureCube(uEnvMap, dirReflect).rgb;
	float shininess = uShininess.x * uShininess.y;
	color.rgb = color.rgb * (1.0 - shininess) + reflectColor * shininess;

	//vec3 shadowPos = CalcShadowmapUVAndZ();
	//float dirLightIntensity = CalcLightIntensity(texture2D(uTexture4, shadowPos.xy).r, shadowPos);
	//float dirLightIntensity = CalcLightIntensity(uTexture4);

	vec3 lightColor = CalcLight(color, vec3(1, 1, 1), normal, uTexture4);
	vec4 finalColor = vec4(lightColor, CalcAlphaFromAfterFog());
	if(finalColor.a < 0.1)
		discard;
	gl_FragColor = finalColor;
}
