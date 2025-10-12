//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

//	Inspired by Joey de Vries' Learn OpenGL:
//	https://learnopengl.com/PBR/Lighting

#ifndef OT_SHADOW_GLSL
#define OT_SHADOW_GLSL

#include "utilities.glsl"

// uniforms
layout(std140, set=3, binding=SHADOW_UNIFORMS) uniform ShadowUniforms {
	mat4 viewTransform;
	mat4 shadowViewProjTransform[4];
	float cascade0Distance;
	float cascade1Distance;
	float cascade2Distance;
	float cascade3Distance;
	float shadowTexelSize;
	bool shadowEnabled;
};

layout(set=2, binding=SHADOW_SAMPLERS) uniform sampler2D shadowMap0;
layout(set=2, binding=SHADOW_SAMPLERS + 1) uniform sampler2D shadowMap1;
layout(set=2, binding=SHADOW_SAMPLERS + 2) uniform sampler2D shadowMap2;
layout(set=2, binding=SHADOW_SAMPLERS + 3) uniform sampler2D shadowMap3;

// get hard shadow (on/off) at specified point
float hardShadow(sampler2D shadowmap, vec2 shadowCoord, float depth, float bias) {
	return step(depth, texture(shadowmap, shadowCoord).r + bias);
}

// get shadow value using Percentage-Closer Filtering (PCF)
float getCascadeShadow(sampler2D shadowmap, vec2 shadowCoord, float depth, float bias) {
	float shadow = 0.0f;
	float offset = shadowTexelSize;

	shadow += hardShadow(shadowmap, shadowCoord + vec2(-1.5f, -1.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(-1.5f, -0.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(-1.5f,  0.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(-1.5f,  1.5f) * offset, depth, bias);

	shadow += hardShadow(shadowmap, shadowCoord + vec2(-0.5f, -1.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(-0.5f, -0.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(-0.5f,  0.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(-0.5f,  1.5f) * offset, depth, bias);

	shadow += hardShadow(shadowmap, shadowCoord + vec2(0.5f, -1.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(0.5f, -0.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(0.5f,  0.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(0.5f,  1.5f) * offset, depth, bias);

	shadow += hardShadow(shadowmap, shadowCoord + vec2(1.5f, -1.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(1.5f, -0.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(1.5f,  0.5f) * offset, depth, bias);
	shadow += hardShadow(shadowmap, shadowCoord + vec2(1.5f,  1.5f) * offset, depth, bias);

	return shadow / 16.0;
}

// get shadow value at specified position and distance from camera
float getShadow(vec3 wordPosition, vec3 viewPosition, float NdotL) {
	// see if we have shadowmaps
	if (shadowEnabled) {
		// determine shadow bias based on slope (shadowmap resolution is applied below)
		float bias = max(0.05f * (1.0f - NdotL), 0.005f);

		// determine cascade and calculate shadow
		if (-viewPosition.z < cascade0Distance) {
			vec4 pos = shadowViewProjTransform[0] * vec4(wordPosition, 1.0f);
			pos = pos / pos.w;
			return getCascadeShadow(shadowMap0, clipToUvSpace(pos.xyz), pos.z, bias * (1.0f / (cascade0Distance * 0.5f)));

		} else if (-viewPosition.z < cascade1Distance) {
			vec4 pos = shadowViewProjTransform[1] * vec4(wordPosition, 1.0f);
			pos = pos / pos.w;
			return getCascadeShadow(shadowMap1, clipToUvSpace(pos.xyz), pos.z, bias * (1.0f / (cascade1Distance * 0.5f)));

		} else if (-viewPosition.z < cascade2Distance) {
			vec4 pos = shadowViewProjTransform[2] * vec4(wordPosition, 1.0f);
			pos = pos / pos.w;
			return getCascadeShadow(shadowMap2, clipToUvSpace(pos.xyz), pos.z, bias * (1.0f / (cascade2Distance * 0.5f)));

		} else if (-viewPosition.z < cascade3Distance) {
			vec4 pos = shadowViewProjTransform[3] * vec4(wordPosition, 1.0f);
			pos = pos / pos.w;
			return getCascadeShadow(shadowMap3, clipToUvSpace(pos.xyz), pos.z, bias * (1.0f / (cascade3Distance * 0.5f)));

		} else {
			return 1.0f;
		}

	} else {
		return 1.0f;
	}
}

#endif
