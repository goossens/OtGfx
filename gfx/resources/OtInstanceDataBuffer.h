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
#include <vector>

#include "SDL3/SDL_gpu.h"


//
//	OtInstanceDataBuffer
//

class OtInstanceDataBuffer {
public:
	enum class ElementFormat {
		mat4,
		vec4
	};

	// clear the object
	void clear();

	// see if buffer is valid
	inline bool isValid() { return dataBuffer != nullptr; }

	// specify element formats
	void  addElement(ElementFormat format);

	// set instance data
	void set(void* data, size_t count, bool dynamic=false);

	// get instance data details
	inline size_t getSize() { return dataSize; }
	inline size_t getCount() { return dataCount; }

private:
	// the GPU resources
	std::shared_ptr<SDL_GPUBuffer> dataBuffer;
	std::shared_ptr<SDL_GPUTransferBuffer> transferBuffer;

	// memory manage SDL resource
	void assignDataBuffer(SDL_GPUBuffer* newBuffer);
	void assignTransferBuffer(SDL_GPUTransferBuffer* newBuffer);

	// instance data information
	std::vector<SDL_GPUVertexAttribute> elements;

	size_t dataSize = 0;
	size_t dataCount = 0;
	size_t currentBufferCount = 0;

	// get accesss to the raw buffer handle
	friend class OtRenderPass;
	inline SDL_GPUBuffer* getBuffer() { return dataBuffer.get(); }
};
