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

	// get vertex description
	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(OtVertex, position)},
			{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(OtVertex, normal)},
			{2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(OtVertex, tangent)},
			{3, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(OtVertex, bitangent)},
			{4, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(OtVertex, uv)}
		};

		static OtVertexDescription description{sizeof(OtVertex), sizeof(attributes) / sizeof(attributes[0]), attributes};
		return &description;
	}
};


//
//	OtVertexPosCol2D
//

struct OtVertexPosCol2D {
	// vertex elements
	glm::vec2 position;
	glm::vec4 color;

	// constructors
	OtVertexPosCol2D() = default;
	inline OtVertexPosCol2D(const glm::vec2& p, const glm::vec4& c=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) : position(p), color(c) {}

	// get vertex description
	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(OtVertexPosCol2D, position)},
			{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(OtVertexPosCol2D, color)}
		};

		static OtVertexDescription description{sizeof(OtVertexPosCol2D), sizeof(attributes) / sizeof(attributes[0]), attributes};
		return &description;
	}
};


//
//	OtVertexPosUv2D
//

struct OtVertexPosUv2D {
	// vertex elements
	glm::vec2 position;
	glm::vec2 uv;

	// constructors
	OtVertexPosUv2D() = default;
	inline OtVertexPosUv2D(const glm::vec2& p, const glm::vec2& u=glm::vec2(0.0f)) : position(p), uv(u) {}

	// get vertex description
	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(OtVertexPosUv2D, position)},
			{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(OtVertexPosUv2D, uv)}
		};

		static OtVertexDescription description{sizeof(OtVertexPosUv2D), sizeof(attributes) / sizeof(attributes[0]), attributes};
		return &description;
	}
};


//
//	OtVertexPosColor
//

struct OtVertexPosColor {
	// vertex elements
	glm::vec3 position;
	glm::vec4 color;

	// constructors
	OtVertexPosColor() = default;

	inline OtVertexPosColor(const glm::vec3& p, const glm::vec4& c) : position(p), color(c) {}

	// get vertex description
	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(OtVertexPosColor, position)},
			{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(OtVertexPosColor, color)}
		};

		static OtVertexDescription description{sizeof(OtVertexPosColor), sizeof(attributes) / sizeof(attributes[0]), attributes};
		return &description;
	}
};


//
//	OtVertexPos
//

struct OtVertexPos {
	// vertex elements
	glm::vec3 position;

	// constructors
	OtVertexPos() = default;
	inline OtVertexPos(const glm::vec3& p) : position(p) {}

	// get vertex description
	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(OtVertexPos, position)}
		};

		static OtVertexDescription description{sizeof(OtVertexPos), sizeof(attributes) / sizeof(attributes[0]), attributes};
		return &description;
	}
};


//
//	OtVertexPosUvCol2D
//

struct OtVertexPosUvCol2D {
	// vertex elements
	glm::vec2 position;
	glm::vec2 uv;
	glm::vec4 color;

	// constructors
	OtVertexPosUvCol2D() = default;
	inline OtVertexPosUvCol2D(const glm::vec2& p, const glm::vec2& u=glm::vec2(0.0f), const glm::vec4& c=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) : position(p), uv(u), color(c) {}

	// get vertex description
	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(OtVertexPosUvCol2D, position)},
			{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(OtVertexPosUvCol2D, uv)},
			{2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(OtVertexPosUvCol2D, color)}
		};

		static OtVertexDescription description{sizeof(OtVertexPosUvCol2D), sizeof(attributes) / sizeof(attributes[0]), attributes};
		return &description;
	}
};


//
//	OtVertexBones
//

struct OtVertexBones {
	// vertex elements
	glm::vec4 indices;
	glm::vec4 weights;

	// constructors
	OtVertexBones() = default;
	inline OtVertexBones(const glm::vec4& i, const glm::vec4& w) : indices(i), weights(w) {}

	// get vertex description
	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(OtVertexBones, indices)},
			{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(OtVertexBones, weights)}
		};

		static OtVertexDescription description{sizeof(OtVertexBones), sizeof(attributes) / sizeof(attributes[0]), attributes};
		return &description;
	}
};
