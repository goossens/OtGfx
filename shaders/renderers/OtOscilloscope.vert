//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#version 450

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aUv;
layout (location = 2) in vec4 aColor;

layout (location = 0) out vec2 vUv;
layout (location = 1) out vec4 vColor;

layout(std140, set=1, binding=0) uniform UBO {
	mat4 matrix;
};

void main() {
	gl_Position = matrix * vec4(aPosition, 0.0f, 1.0f);
	vUv = aUv;
	vColor = aColor;
}
