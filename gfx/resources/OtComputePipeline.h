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
	// load shader
	inline void load(const uint32_t* code, size_t size) {
		// figure out shader metadata
		SDL_ShaderCross_ComputePipelineMetadata* metadata = SDL_ShaderCross_ReflectComputeSPIRV((Uint8*) code, size, 0);

		if (metadata == nullptr) {
			OtLogFatal("Error in SDL_ShaderCross_ReflectComputeSPIRV: {}", SDL_GetError());
		}

		// cross compile to the appropriate shader format and create a shader object
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

	// clear the object
	inline void clear() { pipeline = nullptr; }

	// see if pipeline is valid
	inline bool isValid() { return pipeline != nullptr; }

private:
	// the GPU resource
	friend class OtComputePass;
	std::shared_ptr<SDL_GPUComputePipeline> pipeline;

	// memory manage SDL resource
	inline void assign(SDL_GPUComputePipeline* newPipeline) {
		pipeline = std::shared_ptr<SDL_GPUComputePipeline>(
			newPipeline,
			[](SDL_GPUComputePipeline* oldPipeline) {
				SDL_ReleaseGPUComputePipeline(OtGpu::instance().device, oldPipeline);
			});
	}
};
