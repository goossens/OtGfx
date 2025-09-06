//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <cstdint>

#include "glm/glm.hpp"
#include "SDL3/SDL_gpu.h"


//
//	OtVertexDescription
//

struct OtVertexDescription {
	size_t size;
	size_t members;
	SDL_GPUVertexAttribute* attributes;
};


//
//	OtVertex
//

struct OtVertex {
	// vertex elements
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec2 uv;

	// constructors
	OtVertex() = default;

	inline OtVertex(const glm::vec3& p, const glm::vec3& n=glm::vec3(0.0f), const glm::vec2& u=glm::vec2(0.0f), const glm::vec3 t=glm::vec3(0.0f), const glm::vec3 b=glm::vec3(0.0f)) :
		position(p), normal(n), tangent(t), bitangent(b), uv(u) {
	}

	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			{
				.buffer_slot = 0,
				.location = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.offset = offsetof(OtVertex, position),
			},
			{
				.buffer_slot = 0,
				.location = 1,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.offset = offsetof(OtVertex, normal),
			},
			{
				.buffer_slot = 0,
				.location = 2,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.offset = offsetof(OtVertex, tangent),
			},
			{
				.buffer_slot = 0,
				.location = 3,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.offset = offsetof(OtVertex, bitangent),
			},
			{
				.buffer_slot = 0,
				.location = 4,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
				.offset = offsetof(OtVertex, uv),
			},
		};

		static OtVertexDescription description{
			.size = sizeof(OtVertex),
			.members = sizeof(attributes) / sizeof(attributes[0]),
			.attributes = attributes
		};

		return &description;
	}
};

