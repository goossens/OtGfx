//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "SDL3/SDL.h"

#include "OtLog.h"
#include "OtSingleton.h"


//
//	OtGpu
//

class OtGpu : public OtSingleton<OtGpu> {
public:
	// rendering properties
	SDL_Window* window;
	SDL_GPUDevice* device;
	SDL_GPUCommandBuffer* copyCommandBuffer;
	SDL_GPUCommandBuffer* pipelineCommandBuffer;
	SDL_GPUTexture* swapchainTexture;
	int width;
	int height;

	// dummy textures
	SDL_GPUTexture* transparentDummyTexture;
	SDL_GPUTexture* blackDummyTexture;
	SDL_GPUTexture* whiteDummyTexture;
};
