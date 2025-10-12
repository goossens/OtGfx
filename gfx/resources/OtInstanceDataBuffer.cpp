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

#include "OtGpu.h"
#include "OtInstanceDataBuffer.h"


//
//	OtInstanceDataBuffer::clear
//

void OtInstanceDataBuffer::clear() {
	dataBuffer = nullptr;
	transferBuffer = nullptr;
	dataSize = 0;
	dataCount = 0;
	currentBufferCount = 0;
}


//
//	OtInstanceDataBuffer::set
//

void OtInstanceDataBuffer::set(void* data, size_t size, size_t count, bool dynamic) {
	// remember the data details
	dataSize = size;
	dataCount = count;

	// create/resize data buffer (if required)
	auto bufferSize = count * size;
	auto& gpu = OtGpu::instance();

	if (!dynamic || count > currentBufferCount) {
		SDL_GPUBufferCreateInfo bufferInfo{
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = static_cast<Uint32>(bufferSize),
			.props = 0
		};

		SDL_GPUBuffer* vbuffer = SDL_CreateGPUBuffer(gpu.device, &bufferInfo);

		if (!vbuffer) {
			OtLogFatal("Error in SDL_CreateGPUBuffer: {}", SDL_GetError());
		}

		assignDataBuffer(vbuffer);

		// create a transfer buffer
		SDL_GPUTransferBufferCreateInfo transferInfo{
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = static_cast<Uint32>(bufferSize),
			.props = 0
		};

		SDL_GPUTransferBuffer* tbuffer = SDL_CreateGPUTransferBuffer(gpu.device, &transferInfo);

		if (!tbuffer) {
			OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
		}

		assignTransferBuffer(tbuffer);
	}

	// put instance data in transfer buffer
	void* bufferData = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer.get(), dynamic);
	std::memcpy(bufferData, data, bufferSize);
	SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer.get());

	// upload instance buffer to GPU
	SDL_GPUTransferBufferLocation location{
		.transfer_buffer = transferBuffer.get(),
		.offset = 0
	};

	SDL_GPUBufferRegion region{
		.buffer = dataBuffer.get(),
		.offset = 0,
		.size = static_cast<Uint32>(bufferSize)
	};

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);
	SDL_UploadToGPUBuffer(copyPass, &location, &region, dynamic);
	SDL_EndGPUCopyPass(copyPass);

	if (dynamic) {
		currentBufferCount = count;

	} else {
		transferBuffer = nullptr;
		currentBufferCount = 0;
	}
}


//
//	OtInstanceDataBuffer::assignDataBuffer
//

void OtInstanceDataBuffer::assignDataBuffer(SDL_GPUBuffer* newBuffer) {
	dataBuffer = std::shared_ptr<SDL_GPUBuffer>(
		newBuffer,
		[](SDL_GPUBuffer* oldBuffer) {
			SDL_ReleaseGPUBuffer(OtGpu::instance().device, oldBuffer);
		});
}


//
//	OtInstanceDataBuffer::assignTransferBuffer
//

void OtInstanceDataBuffer::assignTransferBuffer(SDL_GPUTransferBuffer* newBuffer) {
	transferBuffer = std::shared_ptr<SDL_GPUTransferBuffer>(
		newBuffer,
		[](SDL_GPUTransferBuffer* oldBuffer) {
			SDL_ReleaseGPUTransferBuffer(OtGpu::instance().device, oldBuffer);
		});
}
