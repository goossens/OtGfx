//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 0) out vec4 vColor;

void main() {
	gl_Position = vec4(aPosition, 1.0f);
	vColor = aColor;
}
