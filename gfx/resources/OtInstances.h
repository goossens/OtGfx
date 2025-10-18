//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "SDL3/SDL_gpu.h"

#include "OtAABB.h"
#include "OtCamera.h"
#include "OtVertex.h"


//
//	OtInstances
//

class OtInstances {
public:
	// constructor
	OtInstances();

	// clear list of instances
	void clear();

	// see if we have any instances (i.e. is this a valid list)
	inline bool isValid() { return instances->size() != 0; }

	// instance I/O
	void load(const std::string& path);
	void save(const std::string& path);

	// add an instance
	void add(const glm::mat4& instance, bool updateVersion=true);

	// access individual instances
	inline glm::mat4& operator[](size_t i) { return instances->operator[](i); }
	inline size_t size() { return instances->size(); }
	inline glm::mat4* data() { return instances->data(); }

	// version management
	inline void setVersion(int v) { version = v; }
	inline int getVersion() { return version; }
	inline void incrementVersion() { version++; }

	// see if instances are identical
	inline bool operator==(OtInstances& rhs) {
		return instances == rhs.instances && version == rhs.version;
	}

	inline bool operator!=(OtInstances& rhs) {
		return !operator==(rhs);
	}

	// get vertex description
	static inline OtVertexDescription* getDescription() {
		static SDL_GPUVertexAttribute attributes[] = {
			SDL_GPUVertexAttribute{0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, 0},
			{1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, sizeof(glm::vec4)},
			{2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, sizeof(glm::vec4) * 2},
			{3, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, sizeof(glm::vec4) * 3}
		};

		static OtVertexDescription description{sizeof(glm::mat4), sizeof(attributes) / sizeof(attributes[0]), attributes};

		return &description;
	}

private:
	// list of transformations (for the instances)
	std::shared_ptr<std::vector<glm::mat4>> instances;
	int version = 0;

	// the GPU resources
	std::shared_ptr<SDL_GPUBuffer> vertexBuffer;
	std::shared_ptr<SDL_GPUTransferBuffer> transferBuffer;
	int gpuVersion = -1;

	// memory manage SDL resource
	void assignVertexBuffer(SDL_GPUBuffer* newBuffer);
	void assignTransferBuffer(SDL_GPUTransferBuffer* newBuffer);

	// get accesss to the raw buffer handle
	friend class OtRenderPass;
	SDL_GPUBuffer* getBuffer();
};
