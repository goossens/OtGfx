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

#include "SDL3/SDL_gpu.h"

#include "OtGpu.h"


//
//	OtIndexBuffer
//

class OtIndexBuffer {
public:
	// clear the object
	inline void clear() { buffer = nullptr; }

	// see if buffer is valid
	inline bool isValid() { return buffer != nullptr; }

	// set indices
	inline void set(uint32_t* data, size_t count) {
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

		// remember index count
		indexCount = count;
	}

	// get index count
	inline size_t getCount() { return indexCount; }

private:
	// the GPU resource
	std::shared_ptr<SDL_GPUBuffer> buffer;

	// memory manage SDL resource
	inline void assign(SDL_GPUBuffer* newBuffer) {
		buffer = std::shared_ptr<SDL_GPUBuffer>(
			newBuffer,
			[](SDL_GPUBuffer* oldBuffer) {
				SDL_ReleaseGPUBuffer(OtGpu::instance().device, oldBuffer);
			});
	}

	// number of indices
	size_t indexCount = 0;

	// get accesss to the raw buffer handle
	friend class OtRenderPass;
	SDL_GPUBuffer* getBuffer() { return buffer.get(); }
};
