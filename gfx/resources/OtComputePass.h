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
#include <vector>

#include "SDL3/SDL_gpu.h"

#include "OtCommandBuffer.h"
#include "OtComputePipeline.h"
#include "OtTexture.h"


//
//	OtComputePass
//

class OtComputePass {
public:
	// add a texture
	void addTexture(std::shared<OtTexture> texture);

	// add a buffer
	void addBuffer();

	void begin(OtCommandBuffer& commands, std::shared<OtComputePipeline> pipeline);
	void setUniforms(size_t slot, const void data, size_t size);
	void dispatch(size_t threadsX, size_t threadsY, size_t threadsZ, size_t threadsPerGroupX=1, size_t threadsPerGroupY=1, size_t threadsPerGroupZ=1);
	void end();

private:
	// work variables
	SDL_GPUCommandBuffer* commandBuffer;
};
