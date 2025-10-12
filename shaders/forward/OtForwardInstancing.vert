//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#version 450
#extension GL_GOOGLE_include_directive : require

#define CAMERA_UNIFORMS 1
#include "camera.glsl"

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in vec2 aUv;
layout (location = 5) in vec4 data0;
layout (location = 6) in vec4 data1;
layout (location = 7) in vec4 data2;
layout (location = 8) in vec4 data3;

layout (location = 0) out vec3 vPosition;
layout (location = 1) out vec3 vNormal;
layout (location = 2) out vec3 vTangent;
layout (location = 3) out vec3 vBitangent;
layout (location = 4) out vec3 vUv;

layout(std140, set=1, binding=0) uniform UBO {
	mat4 modelMatrix;
};

void main() {
	mat4 instanceMatrix = modelMatrix * mat4(data0, data1, data2, data3);

	vUv = vUv;
	vPosition = (instanceMatrix * vec4(aPosition, 1.0f)).xyz;
	vNormal = (instanceMatrix * vec4(aNormal, 0.0f)).xyz;
	vTangent = (instanceMatrix * vec4(aTangent, 0.0f)).xyz;
	vBitangent = (instanceMatrix * vec4(aBitangent, 0.0f)).xyz;
	gl_Position = viewProjectionMatrix * vec4(vPosition, 1.0f);
}
