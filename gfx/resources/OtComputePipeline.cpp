//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtLog.h"

#include "OtComputePipeline.h"


//
//	OtComputePipeline::OtComputePipeline
//

OtComputePipeline::OtComputePipeline(SDL_GPUDevice* d, const uint32_t* code, size_t size) {
	// remember graphics device for destructor
	device = d;

	// figure out shader metadata
	metadata = SDL_ShaderCross_ReflectComputeSPIRV((Uint8*) code, size, 0);

	if (metadata == nullptr) {
		OtLogFatal("Error in SDL_ShaderCross_ReflectComputeSPIRV: {}", SDL_GetError());
	}

	// cross compile to the appropriate shader format and create a shader object
	SDL_ShaderCross_SPIRV_Info info{};
	info.bytecode = (Uint8*) code;
	info.bytecode_size = size;
	info.entrypoint = "main";
	info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
	pipeline = SDL_ShaderCross_CompileComputePipelineFromSPIRV(device, &info, metadata, 0);

	if (pipeline == nullptr) {
		OtLogFatal("Error in SDL_ShaderCross_CompileComputePipelineFromSPIRV: {}", SDL_GetError());
	}
}


//
//	OtComputePipeline::~OtComputePipeline
//

OtComputePipeline::~OtComputePipeline() {
	if (pipeline) {
		SDL_ReleaseGPUComputePipeline(device, pipeline);
	}

	if (metadata) {
		SDL_free(metadata);
	}
}
