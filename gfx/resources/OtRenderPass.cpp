//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtAssert.h"
#include "OtLog.h"

#include "OtGpu.h"
#include "OtRenderPass.h"


//
//	OtRenderPass::~OtRenderPass
//

OtRenderPass::~OtRenderPass() {
	OtAssert(!open);
}


//
//	OtRenderPass::start
//

void OtRenderPass::start(OtTexture& texture, bool clear) {
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

void OtRenderPass::start(OtFrameBuffer& framebuffer) {
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

void OtRenderPass::start(OtGbuffer& gbuffer) {
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


//
//	OtRenderPass::bindPipeline
//

void OtRenderPass::bindPipeline(OtRenderPipeline& pipeline) {
	OtAssert(open);
	SDL_BindGPUGraphicsPipeline(pass, pipeline.getPipeline());
}


//
//	OtRenderPass::bindVertexSampler
//

void OtRenderPass::bindVertexSampler(size_t slot, OtSampler& sampler, SDL_GPUTexture* texture) {
	OtAssert(open);

	SDL_GPUTextureSamplerBinding binding{
		.texture = texture,
		.sampler = sampler.getSampler()
	};

	SDL_BindGPUVertexSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
}

void OtRenderPass::bindVertexSampler(size_t slot, OtSampler& sampler, OtTexture& texture) {
	OtAssert(open);

	SDL_GPUTextureSamplerBinding binding{
		.texture = texture.getTexture(),
		.sampler = sampler.getSampler()
	};

	SDL_BindGPUVertexSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
}

void OtRenderPass::bindVertexSampler(size_t slot, OtSampler& sampler, OtCubeMap& cubemap) {
	OtAssert(open);

	SDL_GPUTextureSamplerBinding binding{
		.texture = cubemap.getTexture(),
		.sampler = sampler.getSampler()
	};

	SDL_BindGPUVertexSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
}


//
//	OtRenderPass::bindFragmentSampler
//

void OtRenderPass::bindFragmentSampler(size_t slot, OtSampler& sampler, SDL_GPUTexture* texture) {
	OtAssert(open);

	SDL_GPUTextureSamplerBinding binding{
		.texture = texture,
		.sampler = sampler.getSampler()
	};

	SDL_BindGPUFragmentSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
}

void OtRenderPass::bindFragmentSampler(size_t slot, OtSampler& sampler, OtTexture& texture) {
	OtAssert(open);

	SDL_GPUTextureSamplerBinding binding{
		.texture = texture.getTexture(),
		.sampler = sampler.getSampler()
	};

	SDL_BindGPUFragmentSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
}

void OtRenderPass::bindFragmentSampler(size_t slot, OtSampler& sampler, OtCubeMap& cubemap) {
	OtAssert(open);

	SDL_GPUTextureSamplerBinding binding{
		.texture = cubemap.getTexture(),
		.sampler = sampler.getSampler()
	};

	SDL_BindGPUFragmentSamplers(pass, static_cast<Uint32> (slot), &binding, 1);
}


//
//	OtRenderPass::setVertexUniforms
//

void OtRenderPass::setVertexUniforms(size_t slot, const void* data, size_t size) {
	OtAssert(open);

	SDL_PushGPUVertexUniformData(
		OtGpu::instance().pipelineCommandBuffer,
		static_cast<Uint32>(slot),
		data,
		static_cast<Uint32>(size));
}


//
//	OtRenderPass::setFragmentUniforms
//

void OtRenderPass::setFragmentUniforms(size_t slot, const void* data, size_t size) {
	OtAssert(open);

	SDL_PushGPUFragmentUniformData(
		OtGpu::instance().pipelineCommandBuffer,
		static_cast<Uint32>(slot),
		data,
		static_cast<Uint32>(size));
}


//
//	OtRenderPass::setStencilReference
//

void OtRenderPass::setStencilReference(uint8_t reference) {
	OtAssert(open);
	SDL_SetGPUStencilReference(pass, static_cast<Uint8>(reference));
}


//
//	OtRenderPass::render
//

void OtRenderPass::render(size_t vertices, size_t instances) {
	OtAssert(open);
	SDL_DrawGPUPrimitives(pass, static_cast<Uint32>(vertices), static_cast<Uint32>(instances), 0, 0);
}

void OtRenderPass::render(OtVertexBuffer& buffer) {
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

void OtRenderPass::render(OtVertexBuffer& vertexBuffer, OtIndexBuffer& indexBuffer, size_t offset, size_t count) {
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


//
//	OtRenderPass::end
//

void OtRenderPass::end() {
	OtAssert(open);
	SDL_EndGPURenderPass(pass);
	open = false;
}
