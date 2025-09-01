//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <memory>

#include "SDL3/SDL_gpu.h"

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
	void set(void* data, size_t count, OtVertexDescription* description);

	// submit to GPU
	void submit();

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

	// vertex description
	OtVertexDescription* vertexDescription = nullptr;
};