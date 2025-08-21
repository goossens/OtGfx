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

#include "SDL3/SDL_gpu.h"


//
//	OtComputePipeline
//

class OtComputePipeline {
public:
	// load shader
	void load(const uint32_t* code, size_t size);

	// clear the object
	void clear() { pipeline = nullptr; }
private:
	std::shared_ptr<SDL_GPUComputePipeline> pipeline;
};
