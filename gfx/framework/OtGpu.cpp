//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtLog.h"

#include "OtGpu.h"


//
//	OtGpu::init
//

void OtGpu::init(SDL_Window* win, int w, int h) {
	// remember window information
	window = win;
	width = w;
	height = h;

	// create GPU device
#if OT_DEBUG
	static constexpr bool debug = true;
#else
	static constexpr bool debug = false;
#endif

	device = SDL_CreateGPUDevice(
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
		debug,
		nullptr);

	if (device == nullptr) {
		OtLogFatal("Error in SDL_CreateGPUDevice: {}", SDL_GetError());
	}

	// claim window for GPU device
	if (!SDL_ClaimWindowForGPUDevice(device, window)) {
		OtLogFatal("Error in SDL_ClaimWindowForGPUDevice: {}", SDL_GetError());
	}

	// acquire a command buffer
	auto commandBuffer = SDL_AcquireGPUCommandBuffer(device);

	if (!commandBuffer) {
		OtLogFatal("Error in SDL_AcquireGPUCommandBuffer: {}", SDL_GetError());
	}

	// create a transfer buffer to create the dummy textures
	SDL_GPUTransferBufferCreateInfo bufferInfo {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = sizeof(Uint32),
		.props = 0
	};

	auto transferBuffer = SDL_CreateGPUTransferBuffer(device, &bufferInfo);

	if (!transferBuffer) {
		OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
	}

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

	// create dummy textures
	SDL_Color transparent{ .r = 0, .g = 0, .b = 0, .a = 0 };
	SDL_Color black{ .r = 0, .g = 0, .b = 0, .a = 255 };
	SDL_Color white{ .r = 255, .g = 255, .b = 255, .a = 255 };

	transparentDummyTexture = createDummyTexture(copyPass, transferBuffer, transparent);
	blackDummyTexture = createDummyTexture(copyPass, transferBuffer, black);
	whiteDummyTexture = createDummyTexture(copyPass, transferBuffer, white);
	dummyCubeMap = createDummyCubeMap();

	SDL_EndGPUCopyPass(copyPass);
	SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

	// submit the command buffer
	auto fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);

	if (!fence) {
		OtLogFatal("Error in SDL_SubmitGPUCommandBufferAndAcquireFence: {}", SDL_GetError());
	}

	if (!SDL_WaitForGPUFences(device, true, &fence, 1)) {
		OtLogFatal("Error in SDL_WaitForGPUFences: {}", SDL_GetError());
	}

	SDL_ReleaseGPUFence(device, fence);
}


//
//	OtGpu::release
//

void OtGpu::release() {
	// release dummy textures
	SDL_ReleaseGPUTexture(device, transparentDummyTexture);
	SDL_ReleaseGPUTexture(device, blackDummyTexture);
	SDL_ReleaseGPUTexture(device, whiteDummyTexture);
	SDL_ReleaseGPUTexture(device, dummyCubeMap);

	// release GPU device
	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyGPUDevice(device);
}


//
//	OtGpu::setWindowSize
//

void OtGpu::setWindowSize(int w, int h) {
	width = w;
	height = h;
}


//
//	OtGpu::startFrame
//

void OtGpu::startFrame() {
	// acquire new command buffers
	acquireCommandBuffers();

	// get the swapchain texture
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(pipelineCommandBuffer, window, &swapchainTexture, nullptr, nullptr)) {
		OtLogFatal("Error in SDL_WaitAndAcquireGPUSwapchainTexture: {}", SDL_GetError());
	}
}


//
//	OtGpu::endFrame
//

void OtGpu::endFrame() {
	// execute command captured in this frame
	executeCommandBuffer();
}


//
//	OtGpu::flushAndRestartFrame
//

void OtGpu::flushAndRestartFrame() {
	// execute commands captured sofar in current frame and restart frame
	executeCommandBuffer();
	acquireCommandBuffers();
}


//
//	OtGpu::createDummyTexture
//

SDL_GPUTexture* OtGpu::createDummyTexture(SDL_GPUCopyPass* pass, SDL_GPUTransferBuffer* buffer, SDL_Color pixel) {
	// create new texture
	SDL_GPUTextureCreateInfo textureInfo{
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ,
		.width = 1,
		.height = 1,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
		.props = 0
	};

	auto& gpu = OtGpu::instance();
	SDL_GPUTexture* texture = SDL_CreateGPUTexture(gpu.device, &textureInfo);

	if (!texture) {
		OtLogFatal("Error in SDL_CreateGPUTexture: {}", SDL_GetError());
	}

	// put pixel in transfer buffer
	void* data = SDL_MapGPUTransferBuffer(gpu.device, buffer, true);
	std::memcpy(data, &pixel, sizeof(pixel));
	SDL_UnmapGPUTransferBuffer(gpu.device, buffer);

	// transfer buffer to GPU
	SDL_GPUTextureTransferInfo transferInfo {
		.transfer_buffer = buffer,
		.offset = 0,
		.pixels_per_row = 0,
		.rows_per_layer = 0
	};

	SDL_GPUTextureRegion region {
		.texture = texture,
		.mip_level = 0,
		.layer = 0,
		.x = 0,
		.y = 0,
		.z = 0,
		.w = 1,
		.h = 1,
		.d = 1
	};

	SDL_UploadToGPUTexture(pass, &transferInfo, &region, true);
	return texture;
}


//
//	OtGpu::createDummyCubeMap
//

SDL_GPUTexture* OtGpu::createDummyCubeMap() {
	// create new cubemap
	SDL_GPUTextureCreateInfo info{
		.type = SDL_GPU_TEXTURETYPE_CUBE,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ,
		.width = 1,
		.height = 1,
		.layer_count_or_depth = 6,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
		.props = 0
	};

	SDL_GPUTexture* cubemap = SDL_CreateGPUTexture(OtGpu::instance().device, &info);

	if (!cubemap) {
		OtLogFatal("Error in SDL_CreateGPUTexture: {}", SDL_GetError());
	}

	return cubemap;
}


//
//	OtGpu::acquireCommandBuffers
//

void OtGpu::acquireCommandBuffers() {
	// acquire a copy command buffer
	copyCommandBuffer = SDL_AcquireGPUCommandBuffer(device);

	if (!copyCommandBuffer) {
		OtLogFatal("Error in SDL_AcquireGPUCommandBuffer: {}", SDL_GetError());
	}

	// acquire a pipeline command buffer
	pipelineCommandBuffer = SDL_AcquireGPUCommandBuffer(device);

	if (!pipelineCommandBuffer) {
		OtLogFatal("Error in SDL_AcquireGPUCommandBuffer: {}", SDL_GetError());
	}
}


//
//	OtGpu::executeCommandBuffer
//

void OtGpu::executeCommandBuffer() {
	// submit the copy command buffer
	SDL_GPUFence* fences[2];
	fences[0] = SDL_SubmitGPUCommandBufferAndAcquireFence(copyCommandBuffer);

	if (!fences[0]) {
		OtLogFatal("Error in SDL_SubmitGPUCommandBufferAndAcquireFence: {}", SDL_GetError());
	}

	// submit the pipeline command buffer
	fences[1] = SDL_SubmitGPUCommandBufferAndAcquireFence(pipelineCommandBuffer);

	if (!fences[1]) {
		OtLogFatal("Error in SDL_SubmitGPUCommandBufferAndAcquireFence: {}", SDL_GetError());
	}

	if (!SDL_WaitForGPUFences(device, true, fences, 2)) {
		OtLogFatal("Error in SDL_WaitForGPUFences: {}", SDL_GetError());
	}

	SDL_ReleaseGPUFence(device, fences[0]);
	SDL_ReleaseGPUFence(device, fences[1]);
}
