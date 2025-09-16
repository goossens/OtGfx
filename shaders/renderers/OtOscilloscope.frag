//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#version 450

layout (location = 0) in vec2 vUv;
layout (location = 1) in vec4 vColor;

layout (location = 0) out vec4 fragColor;

layout(std140, set=2, binding=0) uniform UBO {
	float brightness;
	float alpha;
};

layout(set=0, binding=0) uniform sampler2D brush;

void main() {
	fragColor = vColor * texture(brush, vUv) * vec4(brightness, brightness, brightness, alpha);
}
