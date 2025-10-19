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
	visibleInstances = std::make_shared<std::vector<glm::mat4>>();
}


//
//	OtInstances::clear
//

void OtInstances::clear() {
	instances = std::make_shared<std::vector<glm::mat4>>();
	visibleInstances = std::make_shared<std::vector<glm::mat4>>();
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
//	OtInstances::determineVisibility
//

bool OtInstances::determineVisibility(OtCamera& camera, OtAABB& aabb) {
	if (instances->size()) {
		// filter instances based on visibility
		struct InstanceReference {
			InstanceReference(size_t i, float d) : index(i), distance(d) {}
			size_t index;
			glm::mat4 matrix;
			float distance;
		};

		std::vector<InstanceReference> instanceReferences;

		for (size_t i = 0; i < instances->size(); i++) {
			auto instanceAabb = aabb.transform(instances->at(i));

			if (camera.isVisibleAABB(instanceAabb)) {
				instanceReferences.emplace_back(i, glm::distance(camera.position, instanceAabb.getCenter()));
			}
		}

		// clear list of visible instance
		visibleInstances->clear();

		if (instanceReferences.size()) {
			// sort instances by distance to camera
			std::sort(instanceReferences.begin(), instanceReferences.end(), [&](const InstanceReference& i1, const InstanceReference& i2) {
				return i1.distance < i2.distance;
			});

			// extract list of matrices
			for (auto& instanceReference : instanceReferences) {
				visibleInstances->emplace_back(instances->at(instanceReference.index));
			}

			return true;

		} else {
			return false;
		}

	} else {
		return false;
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
	// update GPU buffer
	auto bufferSize = sizeof(glm::mat4) * visibleInstances->size();

	SDL_GPUBufferCreateInfo bufferInfo{};
	bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	bufferInfo.size = static_cast<Uint32>(bufferSize);

	auto& gpu = OtGpu::instance();
	SDL_GPUBuffer* vbuffer = SDL_CreateGPUBuffer(gpu.device, &bufferInfo);

	if (!vbuffer) {
		OtLogFatal("Error in SDL_CreateGPUBuffer: {}", SDL_GetError());
	}

	assignVertexBuffer(vbuffer);

	// create a transfer buffer
	SDL_GPUTransferBufferCreateInfo transferInfo{};
	transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transferInfo.size = static_cast<Uint32>(bufferSize);
	SDL_GPUTransferBuffer* tbuffer = SDL_CreateGPUTransferBuffer(gpu.device, &transferInfo);

	if (!tbuffer) {
		OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
	}

	assignTransferBuffer(tbuffer);

	// put vertex data in transfer buffer
	void* bufferData = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer.get(), false);
	std::memcpy(bufferData, visibleInstances->data(), bufferSize);
	SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer.get());

	// upload vertex buffer to GPU
	SDL_GPUTransferBufferLocation location{};
	location.transfer_buffer = transferBuffer.get();

	SDL_GPUBufferRegion region{};
	region.buffer = vertexBuffer.get();
	region.size = static_cast<Uint32>(bufferSize);

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);
	SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
	SDL_EndGPUCopyPass(copyPass);

	// return raw buffer pointer
	return vertexBuffer.get();
}
