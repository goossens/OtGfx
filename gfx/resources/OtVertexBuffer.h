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
	inline void clear() { buffer = nullptr; }

	// see if buffer is valid
	inline bool isValid() { return buffer != nullptr; }

	// set vertices
	inline void set(void* data, size_t count, OtVertexDescription* description) {
		// remember the vertex description and count
		vertexDescription = description;
		vertexCount = count;

		// create the vertex buffer
		auto size = count * description->size;

		SDL_GPUBufferCreateInfo bufferInfo{
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = static_cast<Uint32>(size),
			.props = 0
		};

		auto& gpu = OtGpu::instance();
		SDL_GPUBuffer* vertexBuffer = SDL_CreateGPUBuffer(gpu.device, &bufferInfo);

		if (!vertexBuffer) {
			OtLogFatal("Error in SDL_CreateGPUBuffer: {}", SDL_GetError());
		}

		assign(vertexBuffer);

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

		// put vertex data in transfer buffer
		void* bufferData = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer, false);
		std::memcpy(bufferData, data, size);
		SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer);

		// upload vertex buffer to GPU
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

	// get vertex count
	inline size_t getCount() { return vertexCount; }

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

	// vertex description and count
	OtVertexDescription* vertexDescription = nullptr;
	size_t vertexCount = 0;

	// get accesss to the raw buffer handle
	friend class OtRenderPass;
	SDL_GPUBuffer* getBuffer() { return buffer.get(); }
};