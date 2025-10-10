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
#include <vector>

#include "glm/glm.hpp"
#include "SDL3/SDL.h"

#include "OtAssert.h"
#include "OtLog.h"

#include "OtCubeMap.h"
#include "OtFrameBuffer.h"
#include "OtGbuffer.h"
#include "OtGeometry.h"
#include "OtGpu.h"
#include "OtIndexBuffer.h"
#include "OtMesh.h"
#include "OtRenderPipeline.h"
#include "OtSampler.h"
#include "OtTexture.h"
#include "OtVertexBuffer.h"


//
//	OtRenderPass
//

class OtRenderPass {
public:
	// destructor
	~OtRenderPass() {
		OtAssert(!open);
	}

	// start a render pass
	inline void start(OtTexture& texture, bool clear) {
		// sanity checks
		OtAssert(!open);

		if (!texture.isValid()) {
			OtLogFatal("Can't use invalid texture in render pass");
		}

		// start rendering pass
		SDL_GPUColorTargetInfo info{};
		info.texture = texture.getTexture();

		if (clear) {
			info.load_op = SDL_GPU_LOADOP_CLEAR;
			info.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
		}

		pass = SDL_BeginGPURenderPass(OtGpu::instance().pipelineCommandBuffer, &info, 1, nullptr);

		if (!pass) {
			OtLogFatal("Error in SDL_BeginGPURenderPass: {}", SDL_GetError());
		}

		open = true;
	}

	inline void start(OtFrameBuffer& framebuffer) {
		// sanity checks
		OtAssert(!open);

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

		open = true;
	}

	inline void start(OtGbuffer& gbuffer) {
		// sanity checks
		OtAssert(!open);

		if (!gbuffer.isValid()) {
			OtLogFatal("Can't use invalid gbuffer in render pass");
		}

		// start rendering pass
		auto info = gbuffer.getRenderTargetInfo();

		pass = SDL_BeginGPURenderPass(
			OtGpu::instance().pipelineCommandBuffer,
			info->colorTargetInfo,
			info->numColorTargets,
			info->depthStencilTargetInfo);

		if (!pass) {
			OtLogFatal("Error in SDL_BeginGPURenderPass: {}", SDL_GetError());
		}

		open = true;
	}

	// bind a render pipeline
	inline void bindPipeline(OtRenderPipeline& pipeline) {
		OtAssert(open);
		SDL_BindGPUGraphicsPipeline(pass, pipeline.getPipeline());
	}

	// bind a vertex sampler for a texture
	inline void bindVertexSampler(size_t slot, OtSampler& sampler, OtTexture& texture) {
		OtAssert(open);

		SDL_GPUTextureSamplerBinding binding{
			.texture = texture.getTexture(),
			.sampler = sampler.getSampler()
		};

		SDL_BindGPUVertexSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
	}

	// bind a vertex sampler for a cubemap
	inline void bindVertexSampler(size_t slot, OtSampler& sampler, OtCubeMap& cubemap) {
		OtAssert(open);

		SDL_GPUTextureSamplerBinding binding{
			.texture = cubemap.getTexture(),
			.sampler = sampler.getSampler()
		};

		SDL_BindGPUVertexSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
	}

	// bind a fragment sampler for a texture
	inline void bindFragmentSampler(size_t slot, OtSampler& sampler, OtTexture& texture) {
		OtAssert(open);

		SDL_GPUTextureSamplerBinding binding{
			.texture = texture.getTexture(),
			.sampler = sampler.getSampler()
		};

		SDL_BindGPUFragmentSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
	}

	// bind a vertex sampler for a cubemap
	inline void bindFragmentSampler(size_t slot, OtSampler& sampler, OtCubeMap& cubemap) {
		OtAssert(open);

		SDL_GPUTextureSamplerBinding binding{
			.texture = cubemap.getTexture(),
			.sampler = sampler.getSampler()
		};

		SDL_BindGPUFragmentSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
	}

	// set uniforms
	inline void setVertexUniforms(size_t slot, const void* data, size_t size) {
		OtAssert(open);

		SDL_PushGPUVertexUniformData(
			OtGpu::instance().pipelineCommandBuffer,
			static_cast<Uint32>(slot),
			data,
			static_cast<Uint32>(size));
	}

	inline void setFragmentUniforms(size_t slot, const void* data, size_t size) {
		OtAssert(open);

		SDL_PushGPUFragmentUniformData(
			OtGpu::instance().pipelineCommandBuffer,
			static_cast<Uint32>(slot),
			data,
			static_cast<Uint32>(size));
	}

	// set the stencil reference
	inline void setStencilReference(uint8_t reference) {
		OtAssert(open);
		SDL_SetGPUStencilReference(pass, static_cast<Uint8>(reference));
	}

	// execute a rendering command
	inline void render(size_t vertices, size_t instances=1) {
		OtAssert(open);
		SDL_DrawGPUPrimitives(pass, static_cast<Uint32>(vertices), static_cast<Uint32>(instances), 0, 0);
	}

	inline void render(OtVertexBuffer& buffer) {
		OtAssert(open);

		// bind the vertex buffer to the pass
		SDL_GPUBufferBinding bufferBindings{
			.buffer = buffer.getBuffer(),
			.offset = 0
		};

		SDL_BindGPUVertexBuffers(pass, 0, &bufferBindings, 1);

		// render the triangles
		SDL_DrawGPUPrimitives(pass, static_cast<Uint32>(buffer.getCount()), 1, 0, 0);
	}

	inline void render(OtVertexBuffer& vertexBuffer, OtIndexBuffer& indexBuffer, size_t offset=0, size_t count=0) {
		OtAssert(open);

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
		Uint32 numIndices = static_cast<Uint32>(count == 0 ? indexBuffer.getCount() : count);
		Uint32 firstIndex = static_cast<Uint32>(offset);
		SDL_DrawGPUIndexedPrimitives(pass, numIndices, 1, firstIndex, 0, 0);
	}

	inline void render(OtMesh& mesh) {
		render(mesh.getVertexBuffer(), mesh.getIndexBuffer());
	}

	inline void render(OtGeometry& geometry) {
		auto& mesh = geometry.getMesh();
		render(mesh.getVertexBuffer(), mesh.getIndexBuffer());
	}

	// end a render pass
	inline void end() {
		OtAssert(open);
		SDL_EndGPURenderPass(pass);
		open = false;
	}

	private:
	// the GPU resource
	SDL_GPURenderPass* pass;

	// state
	bool open = false;
};
