//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef OT_UTILITIES_GLSL
#define OT_UTILITIES_GLSL

// pre multiply color
vec4 preMultiplyAlpha(vec4 color) {
	return vec4(color.rgb * color.a, color.a);
}

// un pre multiply color
vec4 unPreMultiplyAlpha(vec4 color)  {
	if (color.a == 0.0f) {
		return vec4(0.0f);
	}

	return vec4(color.rgb / color.a, color.a);
}

// support functions
float inverseLerp(float value, float minValue, float maxValue) {
  return (value - minValue) / (maxValue - minValue);
}

float remap(float value, float inMin, float inMax, float outMin, float outMax) {
  return mix(outMin, outMax, inverseLerp(value, inMin, inMax));
}

float saturate(float x) {
  return clamp(x, 0.0f, 1.0f);
}

// create matrices
mat3 rotateY(float theta) {
	float c = cos(theta);
	float s = sin(theta);
	return mat3(vec3(c, 0.0f, s), vec3(0.0f, 1.0f, 0.0f), vec3(-s, 0.0f, c));
}

mat3 rotateAxis(vec3 axis, float angle) {
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0f - c;

	return mat3(
		oc * axis.x * axis.x + c,          oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
		oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z - axis.x * s,
		oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c);
}

#endif
