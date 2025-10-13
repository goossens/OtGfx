//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "nlohmann/json.hpp"

#include "OtLog.h"
#include "OtText.h"

#include "OtGlm.h"
#include "OtGpu.h"
#include "OtInstances.h"


//
//	OtInstances::OtInstances
//

OtInstances::OtInstances() {
	instances = std::make_shared<std::vector<glm::mat4>>();
}


//
//	OtInstances::clear
//

void OtInstances::clear() {
	instances = std::make_shared<std::vector<glm::mat4>>();
	incrementVersion();
}


//
//	OtInstances::load
//

void OtInstances::load(const std::string& path) {
	// load instances from file
	std::string text;
	OtText::load(path, text);
	auto data = nlohmann::json::parse(text);

	// clear list and add instances
	instances = std::make_shared<std::vector<glm::mat4>>();

	for (auto element : data.items()) {
		glm::mat4 instance;
		glm::from_json(element.value(), instance);
		instances->emplace_back(instance);
	}

	// increment version number
	incrementVersion();
}


//
//	OtInstances::save
//

void OtInstances::save(const std::string& path) {
	// create json
	auto data = nlohmann::json::array();

	for (auto& instance : *instances) {
		data.emplace_back(instance);
	}

	// write instances to file
	OtText::save(path, data.dump(1, '\t'));
}


//
//	OtInstances::add
//

void OtInstances::add(const glm::mat4 &instance, bool updateVersion) {
	instances->emplace_back(instance);

	if (updateVersion) {
		incrementVersion();
	}
}


//
//	OtInstances::assignVertexBuffer
//

void OtInstances::assignVertexBuffer(SDL_GPUBuffer* newBuffer) {
	vertexBuffer = std::shared_ptr<SDL_GPUBuffer>(
		newBuffer,
		[](SDL_GPUBuffer* oldBuffer) {
			SDL_ReleaseGPUBuffer(OtGpu::instance().device, oldBuffer);
		});
}


//
//	OtInstances::assignTransferBuffer
//

void OtInstances::assignTransferBuffer(SDL_GPUTransferBuffer* newBuffer) {
	transferBuffer = std::shared_ptr<SDL_GPUTransferBuffer>(
		newBuffer,
		[](SDL_GPUTransferBuffer* oldBuffer) {
			SDL_ReleaseGPUTransferBuffer(OtGpu::instance().device, oldBuffer);
		});
}


//
//	OtInstances::getBuffer
//

SDL_GPUBuffer* OtInstances::getBuffer() {
	// update GPU buffer (if required)
	if (gpuVersion != version) {
		auto bufferSize = sizeof(glm::mat4) * instances->size();

		SDL_GPUBufferCreateInfo bufferInfo{
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = static_cast<Uint32>(bufferSize),
			.props = 0
		};

		auto& gpu = OtGpu::instance();
		SDL_GPUBuffer* vbuffer = SDL_CreateGPUBuffer(gpu.device, &bufferInfo);

		if (!vbuffer) {
			OtLogFatal("Error in SDL_CreateGPUBuffer: {}", SDL_GetError());
		}

		assignVertexBuffer(vbuffer);

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

		// put vertex data in transfer buffer
		void* bufferData = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer.get(), false);
		std::memcpy(bufferData, instances->data(), bufferSize);
		SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer.get());

		// upload vertex buffer to GPU
		SDL_GPUTransferBufferLocation location{
			.transfer_buffer = transferBuffer.get(),
			.offset = 0
		};

		SDL_GPUBufferRegion region{
			.buffer = vertexBuffer.get(),
			.offset = 0,
			.size = static_cast<Uint32>(bufferSize)
		};

		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);
		SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
		SDL_EndGPUCopyPass(copyPass);
		gpuVersion = version;
	}

	return vertexBuffer.get();
}
