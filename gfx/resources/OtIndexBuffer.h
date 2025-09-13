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
	inline void clear() {
		indexBuffer = nullptr;
		transferBuffer = nullptr;
		currentBufferSize = 0;
	}

	// see if buffer is valid
	inline bool isValid() { return indexBuffer != nullptr; }

	// set indices
	inline void set(uint32_t* data, size_t count, bool dynamic=false) {
		// remember index count
		indexCount = count;

		// create/resize index buffer (if required)
		auto size = count * sizeof(uint32_t);
		auto& gpu = OtGpu::instance();

		if (!dynamic || size > currentBufferSize) {
			SDL_GPUBufferCreateInfo bufferInfo{
				.usage = SDL_GPU_BUFFERUSAGE_INDEX,
				.size = static_cast<Uint32>(size),
				.props = 0
			};

			SDL_GPUBuffer* ibuffer = SDL_CreateGPUBuffer(gpu.device, &bufferInfo);

			if (!ibuffer) {
				OtLogFatal("Error in SDL_CreateGPUBuffer: {}", SDL_GetError());
			}

			assignIndexBuffer(ibuffer);

			// create a transfer buffer
			SDL_GPUTransferBufferCreateInfo transferInfo{
				.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
				.size = static_cast<Uint32>(size),
				.props = 0
			};

			SDL_GPUTransferBuffer* tbuffer = SDL_CreateGPUTransferBuffer(gpu.device, &transferInfo);

			if (!tbuffer) {
				OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
			}

			assignTransferBuffer(tbuffer);
		}

		// put index data in transfer buffer
		void* bufferData = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer.get(), dynamic);
		std::memcpy(bufferData, data, size);
		SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer.get());

		// upload index buffer to GPU
		SDL_GPUTransferBufferLocation location{
			.transfer_buffer = transferBuffer.get(),
			.offset = 0
		};

		SDL_GPUBufferRegion region{
			.buffer = indexBuffer.get(),
			.offset = 0,
			.size = static_cast<Uint32>(size)
		};

		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);
		SDL_UploadToGPUBuffer(copyPass, &location, &region, dynamic);
		SDL_EndGPUCopyPass(copyPass);

		if (dynamic) {
			currentBufferSize = size;

		} else {
			transferBuffer = nullptr;
			currentBufferSize = 0;
		}
	}

	// get index count
	inline size_t getCount() { return indexCount; }

private:
	// the GPU resource
	std::shared_ptr<SDL_GPUBuffer> indexBuffer;
	std::shared_ptr<SDL_GPUTransferBuffer> transferBuffer;

	// memory manage SDL resource
	inline void assignIndexBuffer(SDL_GPUBuffer* newBuffer) {
		indexBuffer = std::shared_ptr<SDL_GPUBuffer>(
			newBuffer,
			[](SDL_GPUBuffer* oldBuffer) {
				SDL_ReleaseGPUBuffer(OtGpu::instance().device, oldBuffer);
			});
	}

	inline void assignTransferBuffer(SDL_GPUTransferBuffer* newBuffer) {
		transferBuffer = std::shared_ptr<SDL_GPUTransferBuffer>(
			newBuffer,
			[](SDL_GPUTransferBuffer* oldBuffer) {
				SDL_ReleaseGPUTransferBuffer(OtGpu::instance().device, oldBuffer);
			});
	}

	// number of indices
	size_t indexCount = 0;

	// current dynamic buffer size
	size_t currentBufferSize = 0;

	// get accesss to the raw buffer handle
	friend class OtRenderPass;
	SDL_GPUBuffer* getBuffer() { return indexBuffer.get(); }
};
