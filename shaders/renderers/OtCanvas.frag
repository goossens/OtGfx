//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#version 450

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec2 vUv;

layout (location = 0) out vec4 fragColor;

layout(std140, set=2, binding=0) uniform UBO {
	mat3 scissorMat;
	mat3 paintMat;
	vec4 innerCol;
	vec4 outerCol;
	vec2 scissorExt;
	vec2 scissorScale;
	vec2 extent;
	float radius;
	float feather;
	float strokeMult;
	float strokeThr;
	int texType;
	int type;
};

layout(set=0, binding=0) uniform sampler2D tex;

float sdroundrect(vec2 pt, vec2 ext, float rad) {
	vec2 ext2 = ext - vec2(rad, rad);
	vec2 d = abs(pt) - ext2;
	return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f)) - rad;
}

float scissorMask(vec2 p) {
	vec2 sc = (abs((scissorMat * vec3(p, 1.0f)).xy) - scissorExt);
	sc = vec2(0.5f, 0.5f) - sc * scissorScale;
	return clamp(sc.x, 0.0f, 1.0f) * clamp(sc.y, 0.0f, 1.0f);
}

float strokeMask() {
	return min(1.0f, (1.0f - abs(vUv.x * 2.0f - 1.0f)) * strokeMult) * vUv.y;
}

void main(void) {
	float scissor = scissorMask(vPosition);
	float strokeAlpha = strokeMask();
	if (strokeAlpha < strokeThr) discard;

	if (type == 0) { // gradient
		// calculate gradient color using box gradient
		vec2 pt = (paintMat * vec3(vPosition, 1.0f)).xy;
		float d = clamp((sdroundrect(pt, extent, radius) + feather * 0.5f) / feather, 0.0f, 1.0f);
		vec4 color = mix(innerCol, outerCol, d);

		// combine alpha
		color.w *= strokeAlpha * scissor;
		fragColor = color;

	} else if (type == 1) { // image
		// calculate color from texture
		vec2 pt = (paintMat * vec3(vPosition, 1.0f)).xy / extent;
		vec4 color = texture(tex, pt);
		color = texType == 0 ? color : vec4(1.0f, 1.0f, 1.0f, color.x);

		// combine alpha
		color.w *= strokeAlpha * scissor;
		fragColor = color;

	} else if (type == 2) { // stencil fill
		fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	} else if (type == 3) { // textured triangles
		vec4 color = texture(tex, vUv);
		if (texType == 1.0) color = vec4(color.xyz * color.w, color.w);
		if (texType == 2.0) color = vec4(color.x);
		color *= scissor;
		fragColor = color * innerCol;
	}
}
