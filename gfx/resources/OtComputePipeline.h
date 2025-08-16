//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "SDL3/SDL_gpu.h"
#include "SDL3_shadercross/SDL_shadercross.h"


//
//	OtComputePipeline
//

class OtComputePipeline {
public:
	// constructor/destructor
	OtComputePipeline(SDL_GPUDevice* d, const uint32_t* code, size_t size);
	~OtComputePipeline();

private:
	SDL_GPUDevice* device = nullptr;
	SDL_ShaderCross_ComputePipelineMetadata* metadata = nullptr;
	SDL_GPUComputePipeline* pipeline = nullptr;
};
