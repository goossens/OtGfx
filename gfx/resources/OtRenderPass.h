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

		// describe color target
		SDL_GPUColorTargetInfo colorTargetInfo{
			.texture = framebuffer.getColorTexture().getTexture(),
			.load_op = SDL_GPU_LOADOP_CLEAR,
			.store_op = SDL_GPU_STOREOP_STORE
		};

		// describe depth/stencil target
		SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo {
			.texture = framebuffer.getDepthTexture().getTexture(),
			.clear_depth = 1.0f,
			.load_op = SDL_GPU_LOADOP_CLEAR,
			.store_op = SDL_GPU_STOREOP_STORE
		};

		// start rendering pass
		pass = SDL_BeginGPURenderPass(
			OtGpu::instance().pipelineCommandBuffer,
			colorTargetInfo.texture != nullptr ? &colorTargetInfo : nullptr,
			colorTargetInfo.texture != nullptr ? 1 : 0,
			depthStencilTargetInfo.texture != nullptr ? &depthStencilTargetInfo : nullptr);

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
