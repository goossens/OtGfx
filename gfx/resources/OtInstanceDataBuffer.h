//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <cstring>
#include <memory>

#include "SDL3/SDL_gpu.h"

#include "OtLog.h"

#include "OtGpu.h"


//
//	OtInstanceDataBuffer
//

class OtInstanceDataBuffer {
public:
	// clear the object
	inline void clear() {
		dataBuffer = nullptr;
		transferBuffer = nullptr;
		dataSize = 0;
		dataCount = 0;
		currentBufferCount = 0;
	}

	// see if buffer is valid
	inline bool isValid() { return dataBuffer != nullptr; }

	// set instance data
	inline void set(void* data, size_t size, size_t count, bool dynamic=false) {
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

	// get instance data details
	inline size_t getSize() { return dataSize; }
	inline size_t getCount() { return dataCount; }

private:
	// the GPU resources
	std::shared_ptr<SDL_GPUBuffer> dataBuffer;
	std::shared_ptr<SDL_GPUTransferBuffer> transferBuffer;

	// memory manage SDL resource
	inline void assignDataBuffer(SDL_GPUBuffer* newBuffer) {
		dataBuffer = std::shared_ptr<SDL_GPUBuffer>(
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

	// instance data information
	size_t dataSize = 0;
	size_t dataCount = 0;
	size_t currentBufferCount = 0;

	// get accesss to the raw buffer handle
	friend class OtRenderPass;
	SDL_GPUBuffer* getBuffer() { return dataBuffer.get(); }
};
