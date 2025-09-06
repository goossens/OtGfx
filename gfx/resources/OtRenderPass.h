//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <vector>

#include "glm/glm.hpp"
#include "SDL3/SDL.h"

#include "OtFrameBuffer.h"
// #include "OtGbuffer.h"
#include "OtGpu.h"


//
//	OtRenderPass
//

class OtRenderPass {
public:
	// start a render pass
	void start(OtFrameBuffer& framebuffer) {
		// sanity check
		if (!framebuffer.isValid()) {
			OtLogFatal("Can't use invalid framebuffer in render pass");
		}

		// start rendering pass
		auto info = framebuffer.getRenderTargetInfo();

		pass = SDL_BeginGPURenderPass(
			OtGpu::instance().pipelineCommandBuffer,
			info->colorTargetInfo,
			info->numColorTargets,
			info->depthStencilTargetInfo);

		if (!pass) {
			OtLogFatal("Error in SDL_BeginGPURenderPass: {}", SDL_GetError());
		}
	}

	// void start(OtGbuffer& gbuffer) {

	// }

	// end a render pass
	void end() {
		SDL_EndGPURenderPass(pass);
	}

	private:
	// the GPU resource
	SDL_GPURenderPass* pass;
};
