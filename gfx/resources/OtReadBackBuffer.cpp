//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <algorithm>

#include "SDL3/SDL.h"

#include "OtGpu.h"
#include "OtReadBackBuffer.h"


//
//	OtReadBackBuffer::clear
//

void OtReadBackBuffer::clear() {
	// clear the image
	image.clear();
}


//
//	OtReadBackBuffer::readback
//

OtImage& OtReadBackBuffer::readback(OtTexture& texture) {
	return readback(texture, 0, 0, texture.getWidth(), texture.getHeight());
}


//
//	OtReadBackBuffer::readback
//

OtImage& OtReadBackBuffer::readback(OtTexture& texture, int x, int y, int w, int h) {
	// create a transfer buffer
	SDL_GPUTransferBufferCreateInfo bufferInfo{
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD,
		.size = static_cast<Uint32>(texture.getBpp() * w * h),
		.props = 0
	};

	// transfer GPU texture back to CPU
	auto& gpu = OtGpu::instance();
	auto transferBuffer = SDL_CreateGPUTransferBuffer(gpu.device, &bufferInfo);

	SDL_GPUTextureRegion region{
		.texture = texture.getTexture(),
		.mip_level = 0,
		.layer = 0,
		.x = static_cast<Uint32>(x),
		.y = static_cast<Uint32>(y),
		.z = 0,
		.w = static_cast<Uint32>(w),
		.h = static_cast<Uint32>(h),
		.d = 1
	};

	SDL_GPUTextureTransferInfo transferInfo{
		.transfer_buffer = transferBuffer,
		.offset = 0,
		.pixels_per_row = 0,
		.rows_per_layer = 0
	};

	auto commandBuffer = SDL_AcquireGPUCommandBuffer(gpu.device);
	auto copyPass = SDL_BeginGPUCopyPass(commandBuffer);
	SDL_DownloadFromGPUTexture(copyPass, &region, &transferInfo);
	SDL_EndGPUCopyPass(copyPass);

	// wait for download to complete
	auto fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer);
	SDL_WaitForGPUFences(gpu.device, true, &fence, 1);
	SDL_ReleaseGPUFence(gpu.device, fence);

	// transfer data to image
	void* data = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer, false);
	image.load(w, h, texture.getBestImageFormat(), data);
	SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer);
	SDL_ReleaseGPUTransferBuffer(gpu.device, transferBuffer);

	// return reference to image
	return image;
}
