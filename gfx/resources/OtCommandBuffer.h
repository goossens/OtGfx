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


//
//	OtCommandBuffer
//

class OtCommandBuffer {
public:
	// constructor
	OtCommandBuffer(SDL_GPUCommandBuffer* cb) : commandBuffer(cb) {}

	// the actual command buffer
	SDL_GPUCommandBuffer* commandBuffer;
};
