//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <cstring>

#include "OtLog.h"
#include "OtMeasure.h"

#include "OtFramework.h"
#include "OtGpu.h"


//
//	createDummyTexture
//

static SDL_GPUTexture* createDummyTexture(SDL_GPUCopyPass* pass, SDL_GPUTransferBuffer* buffer, SDL_Color pixel) {
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
//	OtFramework::initSDL
//

void OtFramework::initSDL() {
	// initialize SDL library
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		OtLogFatal("Can't initialize SDL library: {}", SDL_GetError());
	}

	// initialize GPU data singleton
	auto& gpu = OtGpu::instance();

	// determine best default window size
	int numberOfDisplays;
	SDL_DisplayID* displays = SDL_GetDisplays(&numberOfDisplays);

	if (numberOfDisplays == 0) {
		OtLogFatal("SDL library can't find any displays");
	}

	SDL_Rect rect;
	SDL_GetDisplayBounds(displays[0], &rect);
	SDL_free(displays);

	if (rect.w >= 1600 && rect.h >= 900) {
		gpu.width = 1600;
		gpu.height = 900;

	} else if (rect.w >= 1440 && rect.h >= 810) {
		gpu.width = 1440;
		gpu.height = 810;

	} else if (rect.w >= 1280 && rect.h >= 720) {
		gpu.width = 1280;
		gpu.height = 720;

	} else if (rect.w >= 1024 && rect.h >= 576) {
		gpu.width = 1024;
		gpu.height = 576;

	} else if (rect.w >= 800 && rect.h >= 450) {
		gpu.width = 800;
		gpu.height = 450;

	} else {
		gpu.width = 640;
		gpu.height = 360;
	}

	// create a new window
	gpu.window = SDL_CreateWindow("ObjectTalk", gpu.width, gpu.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

	if (!gpu.window) {
		OtLogFatal("Error in SDL_CreateWindow: {}", SDL_GetError());
	}

	if (!SDL_SetWindowAspectRatio(gpu.window, 16.0f / 9.0f, 16.0f / 9.0f)) {
		OtLogFatal("Error in SDL_SetWindowAspectRatio: {}", SDL_GetError());
	}

	SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");

	// create GPU device
#if OT_DEBUG
	static constexpr bool debug = true;
#else
	static constexpr bool debug = false;
#endif

	gpu.device = SDL_CreateGPUDevice(
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
		debug,
		nullptr);

	if (gpu.device == nullptr) {
		OtLogFatal("Error in SDL_CreateGPUDevice: {}", SDL_GetError());
	}

	// claim window for GPU device
	if (!SDL_ClaimWindowForGPUDevice(gpu.device, gpu.window)) {
		OtLogFatal("Error in SDL_ClaimWindowForGPUDevice: {}", SDL_GetError());
	}

	// start loop timer
	lastTime = std::chrono::high_resolution_clock::now();

#if __APPLE__
	fixMenus();
#endif
}


//
//	OtFramework::startSetupSDL
//

void OtFramework::startSetupSDL() {
	// start a new command cycle
	startCommandCycle();

	// create a transfer buffer to create the dummy textures
	SDL_GPUTransferBufferCreateInfo bufferInfo {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = sizeof(Uint32),
		.props = 0
	};

	auto& gpu = OtGpu::instance();
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu.device, &bufferInfo);

	if (!transferBuffer) {
		OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
	}

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);

	// create dummy textures
	SDL_Color transparent{ .r = 0, .g = 0, .b = 0, .a = 0 };
	SDL_Color black{ .r = 0, .g = 0, .b = 0, .a = 255 };
	SDL_Color white{ .r = 255, .g = 255, .b = 255, .a = 255 };
	gpu.transparentDummyTexture = createDummyTexture(copyPass, transferBuffer, transparent);
	gpu.blackDummyTexture = createDummyTexture(copyPass, transferBuffer, black);
	gpu.whiteDummyTexture = createDummyTexture(copyPass, transferBuffer, white);

	SDL_EndGPUCopyPass(copyPass);
	SDL_ReleaseGPUTransferBuffer(gpu.device, transferBuffer);
}


//
//	OtFramework::endSetupSDL
//

void OtFramework::endSetupSDL() {
	// execute the command buffers
	endCommandCycle();
}


//
//	OtFramework::eventsSDL
//

void OtFramework::eventsSDL() {
	// process available events
	SDL_Event event;
	auto& gpu = OtGpu::instance();

	while (SDL_PollEvent(&event)) {
		eventIMGUI(event);

		if (event.type == SDL_EVENT_WINDOW_RESIZED) {
			gpu.width = event.window.data1;
			gpu.height = event.window.data2;

		} else if (event.type == SDL_EVENT_QUIT) {
			if (canQuit()) {
				stop();
			}

		} else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
			if (canClose()) {
				stop();
			}
		}
	}
}


//
//	OtFramework::startFrameSDL
//

void OtFramework::startFrameSDL() {
	// calculate loop speed
	loopTime = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(loopTime - lastTime).count();
	loopDuration = static_cast<float>(microseconds) / 1000.0f;
	lastTime = loopTime;

	// use stopwatch to see how long we have to wait for GPU
	OtMeasureStopWatch stopwatch;

	// start a new GPU command cycle
	auto& gpu = OtGpu::instance();
	startCommandCycle();

	// get the swapchain texture
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(gpu.pipelineCommandBuffer, gpu.window, &gpu.swapchainTexture, nullptr, nullptr)) {
		OtLogFatal("Error in SDL_WaitAndAcquireGPUSwapchainTexture: {}", SDL_GetError());
	}

	gpuWaitTime = stopwatch.elapsed();
}


//
//	OtFramework::endFrameSDL
//

void OtFramework::endFrameSDL() {
	// use stopwatch to see how long the GPU takes toprocess the command buffer
	OtMeasureStopWatch stopwatch;

	// execute command buffers
	endCommandCycle();

	// record time and cleanup
	gpuTime = stopwatch.elapsed();
}


//
//	OtFramework::endSDL
//

void OtFramework::endSDL() {
	// cleanup
	auto& gpu = OtGpu::instance();
	SDL_ReleaseGPUTexture(gpu.device, gpu.transparentDummyTexture);
	SDL_ReleaseGPUTexture(gpu.device, gpu.blackDummyTexture);
	SDL_ReleaseGPUTexture(gpu.device, gpu.whiteDummyTexture);

	// terminate SDL
	SDL_ReleaseWindowFromGPUDevice(gpu.device, gpu.window);
	SDL_DestroyGPUDevice(gpu.device);
	SDL_DestroyWindow(gpu.window);
	SDL_Quit();
}


//
//	OtFramework::openURL
//

void OtFramework::openURL(const std::string& url) {
	SDL_OpenURL(url.c_str());
}


//
//	OtFramework::startCommandCycle
//

void OtFramework::startCommandCycle() {
	// start a command cycle
	auto& gpu = OtGpu::instance();
	gpu.copyCommandBuffer = SDL_AcquireGPUCommandBuffer(gpu.device);

	if (!gpu.copyCommandBuffer) {
		OtLogFatal("Error in SDL_AcquireGPUCommandBuffer: {}", SDL_GetError());
	}

	// acquire a pipeline command buffer
	gpu.pipelineCommandBuffer = SDL_AcquireGPUCommandBuffer(gpu.device);

	if (!gpu.pipelineCommandBuffer) {
		OtLogFatal("Error in SDL_AcquireGPUCommandBuffer: {}", SDL_GetError());
	}
}


//
//	OtFramework::endCommandCycle
//

void OtFramework::endCommandCycle() {
	// end a command cycle
	auto& gpu = OtGpu::instance();
	SDL_GPUFence* fences[2];

	// submit the copy command buffer
	fences[0] = SDL_SubmitGPUCommandBufferAndAcquireFence(gpu.copyCommandBuffer);

	if (!fences[0]) {
		OtLogFatal("Error in SDL_SubmitGPUCommandBufferAndAcquireFence: {}", SDL_GetError());
	}

	// submit the pipeline command buffer
	fences[1] = SDL_SubmitGPUCommandBufferAndAcquireFence(gpu.pipelineCommandBuffer);

	if (!fences[1]) {
		OtLogFatal("Error in SDL_SubmitGPUCommandBufferAndAcquireFence: {}", SDL_GetError());
	}

	if (!SDL_WaitForGPUFences(gpu.device, true, fences, 2)) {
		OtLogFatal("Error in SDL_WaitForGPUFences: {}", SDL_GetError());
	}

	SDL_ReleaseGPUFence(gpu.device, fences[0]);
	SDL_ReleaseGPUFence(gpu.device, fences[1]);
}
