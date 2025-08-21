//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "SDL3_shadercross/SDL_shadercross.h"

#include "OtLog.h"

#include "OtComputePipeline.h"
#include "OtGpu.h"


//
//	OtComputePipeline::load
//

void OtComputePipeline::load(const uint32_t* code, size_t size) {
	// figure out shader metadata
	SDL_ShaderCross_ComputePipelineMetadata* metadata = SDL_ShaderCross_ReflectComputeSPIRV((Uint8*) code, size, 0);

	if (metadata == nullptr) {
		OtLogFatal("Error in SDL_ShaderCross_ReflectComputeSPIRV: {}", SDL_GetError());
	}

	// cross compile to the appropriate shader format and create a shader object
	auto device = OtGpu::instance().device;

	SDL_ShaderCross_SPIRV_Info info{};
	info.bytecode = (Uint8*) code;
	info.bytecode_size = size;
	info.entrypoint = "main";
	info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;

	pipeline = std::shared_ptr<SDL_GPUComputePipeline>(
		SDL_ShaderCross_CompileComputePipelineFromSPIRV(device, &info, metadata, 0),
		[device](SDL_GPUComputePipeline* p) {
			SDL_ReleaseGPUComputePipeline(device, p);
		});

	if (pipeline == nullptr) {
		OtLogFatal("Error in SDL_ShaderCross_CompileComputePipelineFromSPIRV: {}", SDL_GetError());
	}

	// cleanup
	SDL_free(metadata);
}
