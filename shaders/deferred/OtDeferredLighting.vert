//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#version 450

layout (location = 0) out vec2 vUv;

const vec2 vertices[3] = vec2[](
	vec2(-1.0f, -1.0f),
	vec2(3.0f, -1.0f),
	vec2(-1.0f, 3.0f));

void main() {
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
	vUv = vec2(0.5f + 0.5f * gl_Position.x, 0.5f - 0.5f * gl_Position.y);
}
