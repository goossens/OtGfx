//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef OT_CAMERA_GLSL
#define OT_CAMERA_GLSL

layout(std140, set=1, binding=CAMERA_UNIFORMS) uniform CameraUniforms {
	mat4 viewProjectionMatrix;
	mat4 inverseViewProjectionMatrix;
	mat4 projectionMatrix;
	mat4 inverseProjectionMatrix;
	mat4 viewMatrix;
	mat4 inverseViewMatrix;
};

#endif
