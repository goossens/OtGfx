//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtLog.h"

#include "OtIndexBuffer.h"


//
//	OtIndexBuffer::setVertices
//

void OtIndexBuffer::set(uint32_t* data, size_t count) {
	// create the index buffer
	auto size = count * sizeof(uint32_t);

	SDL_GPUBufferCreateInfo bufferInfo{
		.usage = SDL_GPU_BUFFERUSAGE_INDEX,
		.size = static_cast<Uint32>(size),
		.props = 0
	};

	auto& gpu = OtGpu::instance();
	SDL_GPUBuffer* indexBuffer = SDL_CreateGPUBuffer(gpu.device, &bufferInfo);

	if (!indexBuffer) {
		OtLogFatal("Error in SDL_CreateGPUBuffer: {}", SDL_GetError());
	}

	assign(indexBuffer);

	// create a transfer buffer
	SDL_GPUTransferBufferCreateInfo transferInfo{
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = static_cast<Uint32>(size),
		.props = 0
	};

	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu.device, &transferInfo);

	if (!transferBuffer) {
		OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
	}

	// put index data in transfer buffer
	void* bufferData = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer, false);
	std::memcpy(bufferData, data, size);
	SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer);

	// upload index buffer to GPU
	SDL_GPUTransferBufferLocation location{
		.transfer_buffer = transferBuffer,
		.offset = 0
	};

	SDL_GPUBufferRegion region{
		.buffer = buffer.get(),
		.offset = 0,
		.size = static_cast<Uint32>(size)
	};

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);
	SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
	SDL_EndGPUCopyPass(copyPass);
	SDL_ReleaseGPUTransferBuffer(gpu.device, transferBuffer);
}


//
//	OtIndexBuffer::submit
//

void OtIndexBuffer::submit() {
}
