#ifndef _LIGHTING_GLSL
#define _LIGHTING_GLSL
#define NUM_LIGHTS 4
#define PCF_SAMPLE_SIZE 1.0
#define PCF_EPSILON -0.01
#define GAMMA 2.2

struct DirectionalLight
{
	vec4 direction;
	vec4 color;
};

struct PointLight
{
	vec4 position;
	vec4 color;
	vec4 attenuation;
};

struct SpotLight
{
	vec4 position;
	vec4 direction;
	vec4 color;
	vec4 attenuation;
	float angleCosMin;
	float angleCosMax;
	float unused1;
	float unused2;
};

float SimpleDiffuse(vec3 normal, vec3 lightDir)
{
	return max(dot(normal, lightDir), 0.0);
}

float SimpleSpecular(vec3 normal, vec3 viewDir, vec3 lightDir, float shininess)
{
	vec3 h = normalize(viewDir + lightDir);
	float angle = max(dot(h, normal), 0.0);
	return pow(angle, shininess);
}

float CalcAttenuation(vec3 attenuation, float dist)
{
	return 1.0 / (dist * dist * attenuation.x + dist * attenuation.y + attenuation.z);
}

vec2 SimplePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shininess)
{
	vec3 lightDir = normalize(light.position.xyz - fragPos);
	float attenuation = CalcAttenuation(light.attenuation.xyz, distance(light.position.xyz, fragPos));
	return vec2(
		SimpleDiffuse(normal, lightDir) * attenuation,
		SimpleSpecular(normal, viewDir, lightDir, shininess) * attenuation);
}

vec2 SimpleDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shininess)
{
	return vec2(
		SimpleDiffuse(normal, -light.direction.xyz),
		SimpleSpecular(normal, viewDir, -light.direction.xyz, shininess));
}

vec2 SimpleSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shininess)
{
	vec3 lightDir = normalize(light.position.xyz - fragPos);
	float attenuation = CalcAttenuation(light.attenuation.xyz, distance(light.position.xyz, fragPos));
	float intensity = (dot(lightDir, -light.direction.xyz) - light.angleCosMin) / (light.angleCosMin - light.angleCosMax);
	intensity = clamp(intensity, 0.0, 1.0);
	intensity = intensity * intensity * (3.0 - 2.0 * intensity);
	attenuation *= intensity;
	return vec2(
		SimpleDiffuse(normal, lightDir) * attenuation,
		SimpleSpecular(normal, viewDir, lightDir,shininess) * attenuation);
}

vec3 GammaCorrectTexture(vec3 color)
{
	return color;
	return pow(color, vec3(1.0 / GAMMA));
}

vec3 GammaCorrectFinal(vec3 color)
{
	return color;
	return pow(color, vec3(GAMMA));
}

vec4 GammaCorrectTexture(vec4 color)
{
	return color;
	return vec4(pow(color.rgb, vec3(1.0 / GAMMA)), color.a);
}

vec4 GammaCorrectFinal(vec4 color)
{
	return color;
	return vec4(pow(color.rgb, vec3(GAMMA)), color.a);
}

float luma(vec3 color) {
  return dot(color, vec3(0.299, 0.587, 0.114));
}

float luma(vec4 color) {
  return dot(color.rgb, vec3(0.299, 0.587, 0.114));
}

#ifndef NO_UNIFORMS_OR_VARYING

uniform vec3 uCameraPosition;

uniform mat4 uShadowVP;
uniform vec2 uShadowTexelSize;

uniform vec4 uAmbientLight;
uniform DirectionalLight uDirLight;
uniform PointLight uPointLights[NUM_LIGHTS];
uniform SpotLight uSpotLights[NUM_LIGHTS];
uniform int uNumPointLights;
uniform int uNumSpotLights;

uniform float uFogStartRadius;
uniform float uFogEndRadius;

uniform vec4 uDiffuseColor;
uniform vec4 uSpecularColor;
uniform vec4 uDefaultNormal;
uniform float uSpecularPower;
uniform float uDiffuseIntensity;
uniform float uSpecularIntensity;

varying vec3 vPosition;
varying vec2 vTexCoords;
varying mat3 vTBN;

vec3 CalcNormal(vec3 normalTexture)
{
	vec3 normal = (normalTexture * 2.0 - 1.0) * (1.0 - uDefaultNormal.a) + uDefaultNormal.rgb * uDefaultNormal.a;
	return normalize(vTBN * normal);
}

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i) {
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float myrand(vec3 seed, int i) {
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

vec3 CalcLight(vec3 diffuseColorTexture, vec3 specularColorTexture, vec3 normal, sampler2D shadowMap)
{
	vec3 viewDir = normalize(uCameraPosition - vPosition);
	vec3 finalDiffuse = uAmbientLight.rgb * uAmbientLight.a;
	vec3 finalSpecular = vec3(0, 0, 0);
	vec2 dirLightAmounts = SimpleDirectionalLight(uDirLight, normal, viewDir, uSpecularPower);

	float dirLightIntensity = 1.0;
	vec4 shadowPos = uShadowVP * vec4(vPosition, 1.0);
	float cosTheta = clamp(dot(normal, -uDirLight.direction.xyz), 0.0, 1.0);
	float bias = PCF_EPSILON;// *tan(acos(cosTheta));
	vec3 shadowRes = vec3(shadowPos.xy / shadowPos.w, (shadowPos.z + bias) / shadowPos.w);
	shadowRes = shadowRes * 0.5 + 0.5;
	if (shadowRes.x - PCF_SAMPLE_SIZE * uShadowTexelSize.x >= 0.0 &&
		shadowRes.x + PCF_SAMPLE_SIZE * uShadowTexelSize.x <= 1.0 &&
		shadowRes.y - PCF_SAMPLE_SIZE * uShadowTexelSize.y >= 0.0 &&
		shadowRes.y + PCF_SAMPLE_SIZE * uShadowTexelSize.y <= 1.0)
	{
		float shadowAmount = 0.0;
		for (float x = -PCF_SAMPLE_SIZE; x <= PCF_SAMPLE_SIZE; x++)
		{
			for (float y = -PCF_SAMPLE_SIZE; y <= PCF_SAMPLE_SIZE; y++)
			{
				vec2 newCoords = shadowRes.xy + vec2(uShadowTexelSize.x * x, uShadowTexelSize.y * y);
				if (newCoords.x < 0.0 || newCoords.x > 1.0 || newCoords.y < 0.0 || newCoords.y > 1.0)
					continue;
				float zFromShadowmap = texture2D(shadowMap, newCoords).r;
				if (zFromShadowmap > shadowRes.z)
					shadowAmount++;
			}
		}
		dirLightIntensity = shadowAmount / ((PCF_SAMPLE_SIZE * 2.0 + 1.0) * (PCF_SAMPLE_SIZE * 2.0 + 1.0));
	}
	
	/*float visibility = 1.0;
	for (int i = 0; i < 4; i++)
	{
		//int index = int(16.0*myrand(floor(vPosition*1000.0), i));
		//index -= (index / 16) * 16;
		float closestZ = texture2D(shadowMap, shadowRes.xy + poissonDisk[i] / 700.0).r;
		if (closestZ < shadowRes.z)
			visibility -= 0.25;
	}
	dirLightIntensity = visibility;
	*/
	finalDiffuse += dirLightAmounts.x * uDirLight.color.rgb * dirLightIntensity;
	finalSpecular += dirLightAmounts.y * uDirLight.color.rgb * dirLightIntensity;
	for(int i = 0; i < NUM_LIGHTS; i++)
	{
		if (i >= uNumPointLights)
			break;
		vec2 pointLightAmounts = SimplePointLight(uPointLights[i], normal, vPosition, viewDir, uSpecularPower);
		finalDiffuse += pointLightAmounts.x * uPointLights[i].color.rgb;
		finalSpecular += pointLightAmounts.y * uPointLights[i].color.rgb;
	}
	for(int i = 0; i < NUM_LIGHTS; i++)
	{
		if (i >= uNumSpotLights)
			break;
		vec2 spotLightAmounts = SimpleSpotLight(uSpotLights[i], normal, vPosition, viewDir, uSpecularPower);
		finalDiffuse += spotLightAmounts.x * uSpotLights[i].color.rgb;
		finalSpecular += spotLightAmounts.y * uSpotLights[i].color.rgb;
	}
	vec3 diffuseColor = GammaCorrectTexture(diffuseColorTexture) * (1.0 - uDiffuseColor.a) + uDiffuseColor.rgb * uDiffuseColor.a;
	vec3 specularColor = GammaCorrectTexture(specularColorTexture) * (1.0 - uSpecularColor.a) + uSpecularColor.rgb * uSpecularColor.a;
	finalDiffuse *= diffuseColor * uDiffuseIntensity;
	finalSpecular *= specularColor * uSpecularIntensity;
	vec3 finalColor = finalDiffuse + finalSpecular;
	//return vec3(dirLightIntensity);
	return GammaCorrectFinal(finalColor);
}

float CalcAlphaFromAfterFog()
{
	float dist = distance(vPosition, uCameraPosition);
	float alpha = clamp(1.0 - (dist - uFogStartRadius) / (uFogEndRadius - uFogStartRadius), 0.0, 1.0);
	return alpha;
}
#endif

#endif