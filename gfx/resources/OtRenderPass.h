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
#include "OtIndexBuffer.h"
#include "OtRenderPipeline.h"
#include "OtVertexBuffer.h"


//
//	OtRenderPass
//

class OtRenderPass {
public:
	// start a render pass
	inline void start(OtFrameBuffer& framebuffer) {
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

	// inline void start(OtGbuffer& gbuffer) {

	// }

	// bind a render pipeline
	inline void bindPipeline(OtRenderPipeline& pipeline) {
		SDL_BindGPUGraphicsPipeline(pass, pipeline.getPipeline());
	}

	// bind a vertex buffer
	inline void bindVertexBuffer(OtVertexBuffer& buffer) {
		SDL_GPUBufferBinding bufferBindings{
			.buffer = buffer.getBuffer(),
			.offset = 0
		};

		SDL_BindGPUVertexBuffers(pass, 0, &bufferBindings, 1);
	}

	// set uniforms
	inline void setVertexUniforms(size_t slot, const void* data, size_t size) {
		SDL_PushGPUVertexUniformData(
			OtGpu::instance().pipelineCommandBuffer,
			static_cast<Uint32>(slot),
			data,
			static_cast<Uint32>(size));
	}

	inline void setFragmentUniforms(size_t slot, const void* data, size_t size) {
		SDL_PushGPUFragmentUniformData(
			OtGpu::instance().pipelineCommandBuffer,
			static_cast<Uint32>(slot),
			data,
			static_cast<Uint32>(size));
	}

	// render triangles
	inline void render(OtVertexBuffer& buffer) {
		// bind the vertex buffer to the pass
		SDL_GPUBufferBinding bufferBindings{
			.buffer = buffer.getBuffer(),
			.offset = 0
		};

		SDL_BindGPUVertexBuffers(pass, 0, &bufferBindings, 1);

		// render the triangles
		SDL_DrawGPUPrimitives(pass, static_cast<Uint32>(buffer.getCount()), 1, 0, 0);
	}

	inline void render(OtVertexBuffer& vertexBuffer, OtIndexBuffer& indexBuffer) {
		// bind the vertex buffer to the pass
		SDL_GPUBufferBinding vertexBufferBindings{
			.buffer = vertexBuffer.getBuffer(),
			.offset = 0
		};

		SDL_GPUBufferBinding indexBufferBinding = {
			.buffer = indexBuffer.getBuffer(),
			.offset = 0
		};

		SDL_BindGPUVertexBuffers(pass, 0, &vertexBufferBindings, 1);
		SDL_BindGPUIndexBuffer(pass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		// render the triangles
		SDL_DrawGPUIndexedPrimitives(pass, static_cast<Uint32>(indexBuffer.getCount()), 1, 0, 0, 0);
	}

	// end a render pass
	inline void end() {
		SDL_EndGPURenderPass(pass);
	}

	private:
	// the GPU resource
	SDL_GPURenderPass* pass;
};
