//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#version 450

layout (location = 0) in vec4 vColor;
layout (location = 0) out vec4 fragColor;

layout(std140, set = 3, binding = 0) uniform UBO {
	float time;
};

void main() {
	float pulse = sin(time * 2.0f) * 0.5f + 0.5f;
    fragColor = vec4(vColor.rgb * (0.5f + pulse * 0.5f), vColor.a);
}