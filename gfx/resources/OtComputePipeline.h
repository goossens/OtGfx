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
	OtComputePipeline(const uint32_t* code, size_t size) : computeShaderCode(code), computeShaderSize(size) {}

	// associate shader with pipeline
	inline void setShader(const uint32_t* code, size_t size) {
		computeShaderCode = code;
		computeShaderSize = size;
		pipeline = nullptr;
	}

	// clear the object
	inline void clear() {
		computeShaderCode = nullptr;
		computeShaderSize = 0;
		pipeline = nullptr;
	}

	// see if pipeline is valid
	inline bool isValid() { return pipeline != nullptr; }

private:
	// shader definition
	const uint32_t* computeShaderCode = nullptr;
	size_t computeShaderSize = 0;

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
		// create pipeline (if required)
		if (!pipeline) {
			// ensure shader is provided
			if (!computeShaderCode || !computeShaderSize) {
				OtLogFatal("Compute pipeline is missing shader");
			}

			// figure out shader metadata
			SDL_ShaderCross_ComputePipelineMetadata* metadata = SDL_ShaderCross_ReflectComputeSPIRV((Uint8*) computeShaderCode, computeShaderSize, 0);

			if (metadata == nullptr) {
				OtLogFatal("Error in SDL_ShaderCross_ReflectComputeSPIRV: {}", SDL_GetError());
			}

			// cross compile to the appropriate shader format and create a pipeline object
			SDL_ShaderCross_SPIRV_Info info{
				.bytecode = (Uint8*) computeShaderCode,
				.bytecode_size = computeShaderSize,
				.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE,
				.enable_debug = false,
				.entrypoint = "main",
				.name = "",
				.props = 0
			};

			auto computePipeline = SDL_ShaderCross_CompileComputePipelineFromSPIRV(OtGpu::instance().device, &info, metadata, 0);

			if (computePipeline == nullptr) {
				OtLogFatal("Error in SDL_ShaderCross_CompileComputePipelineFromSPIRV: {}", SDL_GetError());
			}

			assign(computePipeline);

			// cleanup
			SDL_free(metadata);
		}

		// return the raw pipeline object
		return pipeline.get();
	}
};
