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
#include <memory>

#include "OtLog.h"

#include "SDL3/SDL_gpu.h"
#include "SDL3_shadercross/SDL_shadercross.h"

#include "OtGpu.h"


//
//	OtComputePipeline
//

class OtComputePipeline {
public:
	// constructor
	OtComputePipeline() = default;
	OtComputePipeline(const uint32_t* c, size_t s) : code(c), size(s) {}

	// initialize the pipeline
	inline void initialize(const uint32_t* c, size_t s) {
		code = c;
		size = s;
		pipeline = nullptr;
	}

	// clear the object
	inline void clear() {
		code = nullptr;
		size = 0;
		pipeline = nullptr;
	}

	// see if pipeline is valid
	inline bool isValid() { return pipeline != nullptr; }

private:
	// shader definition
	const uint32_t* code = nullptr;
	size_t size = 0;

	// the GPU resource
	std::shared_ptr<SDL_GPUComputePipeline> pipeline;

	// memory manage SDL resource
	inline void assign(SDL_GPUComputePipeline* newPipeline) {
		pipeline = std::shared_ptr<SDL_GPUComputePipeline>(
			newPipeline,
			[](SDL_GPUComputePipeline* oldPipeline) {
				SDL_ReleaseGPUComputePipeline(OtGpu::instance().device, oldPipeline);
			});
	}

	// get the raw pipeline object
	friend class OtComputePass;

	inline SDL_GPUComputePipeline* getPipeline() {
		// ensure pipeline is initialized
		if (!code || !size) {
			OtLogFatal("Uninitialize compute pipeline");
		}

		// create pipeline (if required)
		if (!pipeline) {
			// figure out shader metadata
			SDL_ShaderCross_ComputePipelineMetadata* metadata = SDL_ShaderCross_ReflectComputeSPIRV((Uint8*) code, size, 0);

			if (metadata == nullptr) {
				OtLogFatal("Error in SDL_ShaderCross_ReflectComputeSPIRV: {}", SDL_GetError());
			}

			// cross compile to the appropriate shader format and create a pipeline object
			SDL_ShaderCross_SPIRV_Info info{
				.bytecode = (Uint8*) code,
				.bytecode_size = size,
				.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE,
				.enable_debug = false,
				.entrypoint = "main",
				.name = "",
				.props = 0
			};

			assign(SDL_ShaderCross_CompileComputePipelineFromSPIRV(OtGpu::instance().device, &info, metadata, 0));

			if (pipeline == nullptr) {
				OtLogFatal("Error in SDL_ShaderCross_CompileComputePipelineFromSPIRV: {}", SDL_GetError());
			}

			// cleanup
			SDL_free(metadata);
		}

		// return the raw pipeline object
		return pipeline.get();
	}
};
