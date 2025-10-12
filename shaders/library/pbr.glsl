//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

//	Inspired by Joey de Vries' Learn OpenGL:
//	https://learnopengl.com/PBR/Lighting

#ifndef OT_PBR_GLSL
#define OT_PBR_GLSL

#include "constants.glsl"
#include "lighting.glsl"

// material data
struct Material {
	vec3 albedo;
	vec3 normal;
	float metallic;
	float roughness;
	float ao;
};

// convert color to linear colorspace
vec3 toLinear(vec3 color) {
	return pow(color, vec3(2.2f));
}

vec4 toLinear(vec4 color) {
	return vec4(toLinear(color.rgb), color.a);
}

// fresnel equations
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

// Normal Distribution Function (NDF)
float distributionGGX(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;
	float denominator = max(NdotH2 * (a2 - 1.0f) + 1.0f, 0.0001f);
	return a2 / (PI * denominator * denominator);
}

// geometry function
float geometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0f;
	return NdotV / (NdotV * (1.0f - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);
	float ggx2 = geometrySchlickGGX(NdotV, roughness);
	float ggx1 = geometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}

//	PBR calculation for directional light
vec3 directionalLightPBR(Material material, DirectionalLight light, vec3 V) {
	// calculate surface reflection at zero incidence
	vec3 F0 = mix(vec3(0.04f), material.albedo, material.metallic);

	// calculate halfway vector between view direction and light direction
	vec3 H = normalize(V + light.L);

	// calculate the ratio between specular and diffuse reflection
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0); // Schlick Fresnel function

	// Cook-Torrance Microfacet Bidirectional Reflective Distribution (BRDF)
	float NDF = distributionGGX(material.normal, H, material.roughness); // GGX normal distribution function
	float G = geometrySmith(material.normal, V, light.L, material.roughness); // Smith Schlick geometry function

	vec3 specular =
		(NDF * G * F) /
		(4.0f * max(dot(material.normal, V), 0.0f) * max(dot(material.normal, light.L), 0.0f) + 0.0001f);

	// calculate contribution to the reflectance equation
	vec3 kS = F;
	vec3 kD = (vec3(1.0f) - kS) * (1.0f - material.metallic);

	// determine outgoing radiance
	float NdotL = max(dot(material.normal, light.L), 0.0f);
	vec3 color = (kD * material.albedo / PI + specular) * light.color * NdotL;

	// apply shadow
	color *= light.shadow;

	// add ambient light
	color += light.ambient * material.albedo * material.ao;

	// return result
	return color;
}

//	PBR calculation for point light
vec3 pointLightPBR(Material material, PointLight light, vec3 V) {
	// calculate surface reflection at zero incidence
	vec3 F0 = mix(vec3(0.04f), material.albedo, material.metallic);

	// calculate halfway vector between view direction and light direction
	vec3 H = normalize(V + light.L);

	// calculate the ratio between specular and diffuse reflection
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0); // Schlick Fresnel function

	// Cook-Torrance Microfacet Bidirectional Reflective Distribution (BRDF)
	float NDF = distributionGGX(material.normal, H, material.roughness); // GGX normal distribution function
	float G = geometrySmith(material.normal, V, light.L, material.roughness); // Smith Schlick geometry function

	vec3 specular =
		(NDF * G * F) /
		(4.0f * max(dot(material.normal, V), 0.0f) * max(dot(material.normal, light.L), 0.0f) + 0.0001f);

	// calculate contribution to the reflectance equation
	vec3 kS = F;
	vec3 kD = (vec3(1.0f) - kS) * (1.0f - material.metallic);

	// determine outgoing radiance
	float NdotL = clamp(dot(material.normal, light.L), 0.0f, 1.0f);
	vec3 color = (kD / PI + specular) * light.color * NdotL;

	// return result
	return (kD / PI + specular) * light.color * NdotL * light.attenuation;
}

//	PBR calculation for image based lighting
vec3 imageBasedLightingPBR(Material material, vec3 V, int envLevels, sampler2D brdfLUT, samplerCube irradianceMap, samplerCube envMap) {
	// calculate surface reflection at zero incidence
	vec3 F0 = mix(vec3(0.04f), material.albedo, material.metallic);

	// calculate the ratio between specular and diffuse reflection
	vec3 F = fresnelSchlickRoughness(max(dot(material.normal, V), 0.0f), F0, material.roughness);

	// calculate contribution to the reflectance equation
	vec3 kS = F;
	vec3 kD = (vec3(1.0f) - kS) * (1.0f - material.metallic);

	// get diffuse part
    vec3 diffuse = kD * texture(irradianceMap, material.normal).rgb * material.albedo;

	// get specular part
	vec3 R = reflect(-V, material.normal);
	vec3 prefilteredColor = textureLod(envMap, R, material.roughness * envLevels).rgb;
	vec2 brdf = texture(brdfLUT, vec2(max(dot(material.normal, V), 0.0), material.roughness)).rg;
	vec3 specular = prefilteredColor * (kS * brdf.r + brdf.g);

	// determine outgoing radiance
	return (diffuse + specular) * material.ao;
}


#endif
