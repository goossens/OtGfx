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

#include "OtComputePipeline.h"
#include "OtGpu.h"
#include "OtTexture.h"


//
//	OtComputePass
//

class OtComputePass {
public:
	// add an output texture
	inline void addOutputTexture(OtTexture& texture) {
			SDL_GPUStorageTextureReadWriteBinding binding{
			.texture = texture.getTexture(),
			.mip_level = 0,
			.layer = 0,
			.cycle = false
		};

		textures.emplace_back(binding);
	}

	// add a buffer (must be called before begin)
	inline void addBuffer() {
	}

	// start a compute shader pass
	inline void begin(OtComputePipeline& pipeline) {
		pass = SDL_BeginGPUComputePass(
			OtGpu::instance().commandBuffer,
			textures.data(),
			textures.size(),
			nullptr,
			0);

		SDL_BindGPUComputePipeline(pass, pipeline.pipeline.get());
	}

	// set uniform for the compute shader pass (must be called between begin and end calls)
	void setUniforms(size_t slot, const void* data, size_t size);

	// run the compute shader (must be called between begin and end calls)
	inline void dispatch(size_t groupCountX, size_t groupCountY, size_t groupCountZ) {
		SDL_DispatchGPUCompute(pass, groupCountX, groupCountY, groupCountZ);
	}

	// end the compute shader pass
	inline void end() {
		SDL_EndGPUComputePass(pass);
		textures.clear();
	}

private:
	// local data
	SDL_GPUComputePass* pass;
	std::vector<SDL_GPUStorageTextureReadWriteBinding> textures;
};
