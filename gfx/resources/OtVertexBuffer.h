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
#include "OtVertex.h"


//
//	OtVertexBuffer
//

class OtVertexBuffer {
public:
	// clear the object
	inline void clear() {
		vertexBuffer = nullptr;
		transferBuffer = nullptr;
		currentBufferSize = 0;
	}

	// see if buffer is valid
	inline bool isValid() { return vertexBuffer != nullptr; }

	// set vertices
	inline void set(void* data, size_t count, OtVertexDescription* description, bool dynamic=false) {
		// remember the vertex description and vertex count
		vertexDescription = description;
		vertexCount = count;

		// create/resize vertex buffer (if required)
		auto size = count * description->size;
		auto& gpu = OtGpu::instance();

		if (!dynamic || size > currentBufferSize) {
			SDL_GPUBufferCreateInfo bufferInfo{
				.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
				.size = static_cast<Uint32>(size),
				.props = 0
			};

			SDL_GPUBuffer* vbuffer = SDL_CreateGPUBuffer(gpu.device, &bufferInfo);

			if (!vbuffer) {
				OtLogFatal("Error in SDL_CreateGPUBuffer: {}", SDL_GetError());
			}

			assignVertexBuffer(vbuffer);

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

		// put vertex data in transfer buffer
		void* bufferData = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer.get(), dynamic);
		std::memcpy(bufferData, data, size);
		SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer.get());

		// upload vertex buffer to GPU
		SDL_GPUTransferBufferLocation location{
			.transfer_buffer = transferBuffer.get(),
			.offset = 0
		};

		SDL_GPUBufferRegion region{
			.buffer = vertexBuffer.get(),
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

	// get vertex count
	inline size_t getCount() { return vertexCount; }

private:
	// the GPU resources
	std::shared_ptr<SDL_GPUBuffer> vertexBuffer;
	std::shared_ptr<SDL_GPUTransferBuffer> transferBuffer;

	// memory manage SDL resource
	inline void assignVertexBuffer(SDL_GPUBuffer* newBuffer) {
		vertexBuffer = std::shared_ptr<SDL_GPUBuffer>(
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

	// vertex description and count
	OtVertexDescription* vertexDescription = nullptr;
	size_t vertexCount = 0;

	// current dynamic buffer size
	size_t currentBufferSize = 0;

	// get accesss to the raw buffer handle
	friend class OtRenderPass;
	SDL_GPUBuffer* getBuffer() { return vertexBuffer.get(); }
};
