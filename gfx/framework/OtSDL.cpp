//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtLog.h"
#include "OtMeasure.h"

#include "OtFramework.h"


//
//	OtFramework::initSDL
//

void OtFramework::initSDL() {
	// initialize SDL library
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		OtLogFatal("Can't initialize SDL library: {}", SDL_GetError());
	}

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
		width = 1600;
		height = 900;

	} else if (rect.w >= 1440 && rect.h >= 810) {
		width = 1440;
		height = 810;

	} else if (rect.w >= 1280 && rect.h >= 720) {
		width = 1280;
		height = 720;

	} else if (rect.w >= 1024 && rect.h >= 576) {
		width = 1024;
		height = 576;

	} else if (rect.w >= 800 && rect.h >= 450) {
		width = 800;
		height = 450;

	} else {
		width = 640;
		height = 360;
	}

	// create a new window
	window = SDL_CreateWindow("ObjectTalk", width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

	if (!window) {
		OtLogFatal("SDL library can't create window: {}", SDL_GetError());
	}

	SDL_SetWindowAspectRatio(window, 16.0f / 9.0f, 16.0f / 9.0f);

	// create GPU device
	gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB, true, nullptr);

	if (gpuDevice == nullptr) {
		OtLogFatal("Error in  SDL_CreateGPUDevice(): {}", SDL_GetError());
	}

	// claim window for GPU device
	if (!SDL_ClaimWindowForGPUDevice(gpuDevice, window)) {
		OtLogFatal("Error in  SDL_ClaimWindowForGPUDevice(): {}", SDL_GetError());
	}

	// start loop timer
	lastTime = std::chrono::high_resolution_clock::now();

#if __APPLE__
	fixMenus();
#endif
}


//
//	OtFramework::eventsSDL
//

void OtFramework::eventsSDL() {
	// process available events
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		eventIMGUI(event);

		if (event.type == SDL_EVENT_WINDOW_RESIZED) {
			width = event.window.data1;
			height = event.window.data2;

		} else if (event.type == SDL_EVENT_QUIT) {
			if (canQuit()) {
				stop();
			}

		} else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) {
			if (canQuit()) {
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

	// acquire the command buffer
	commandBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);

	if(!commandBuffer) {
		OtLogFatal("Error in  SDL_AcquireGPUCommandBuffer(): {}", SDL_GetError());
	}

	// get the swapchain texture
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, nullptr, nullptr)) {
		OtLogFatal("Error in  SDL_WaitAndAcquireGPUSwapchainTexture(): {}", SDL_GetError());
	}

	gpuWaitTime = stopwatch.elapsed();
}


//
//	OtFramework::endFrameSDL
//

void OtFramework::endFrameSDL() {
	// use stopwatch to see how long the GPU takes toprocess the command buffer
	OtMeasureStopWatch stopwatch;

	// submit the command buffer
	SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);

	if (!fence) {
		OtLogFatal("Error in  SDL_SubmitGPUCommandBufferAndAcquireFence(): {}", SDL_GetError());
	}

	if (!SDL_WaitForGPUFences(gpuDevice, true, &fence, 1)) {
		OtLogFatal("Error in  SDL_WaitForGPUFences(): {}", SDL_GetError());
	}

	// record time and cleanup
	gpuTime = stopwatch.elapsed();
	SDL_ReleaseGPUFence(gpuDevice, fence);
}


//
//	OtFramework::endSDL
//

void OtFramework::endSDL() {
	// terminate SDL
	SDL_DestroyGPUDevice(gpuDevice);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


//
//	OtFramework::openURL
//

void OtFramework::openURL(const std::string& url) {
	SDL_OpenURL(url.c_str());
}
