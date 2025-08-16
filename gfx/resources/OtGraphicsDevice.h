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

#include "OtComputePipeline.h"
#include "OtGraphicsPipeline.h"
#include "OtGraphicsShader.h"


//
//	OtGraphicsDevice
//

class OtGraphicsDevice {
public:
	// constructor
	OtGraphicsDevice(SDL_GPUDevice* d) : device(d) {}

	// create shaders
	std::shared_ptr<OtGraphicsShader> createGraphicsShaderFromSpirv(const uint32_t* code, size_t size, OtGraphicsShader::Stage stage) {
		return std::make_shared<OtGraphicsShader>(device, code, size, stage);
	}

	// create pipelines
	std::shared_ptr<OtComputePipeline> createComputePipelineFromSpirv(const uint32_t* code, size_t size) {
		return std::make_shared<OtComputePipeline>(device, code, size);
	}

	// access device size
	inline void setSize(int w, int h) {
		width = w;
		height = h;
	}

	inline int getWidth() { return width; }
	inline int getHeight() { return height; }

	private:
	// device information
	SDL_GPUDevice* device;
	int width;
	int height;
};
