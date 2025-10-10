//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>

#include "SDL3/SDL_gpu.h"

#include "OtLog.h"

#include "OtComputePipeline.h"
#include "OtCubeMap.h"
#include "OtGpu.h"
#include "OtSampler.h"
#include "OtTexture.h"


//
//	OtComputePass
//

class OtComputePass {
public:
	// add an input sampler for a texture
	inline void addInputSampler(OtSampler& sampler, OtTexture& texture) {
		if ((texture.getUsage() & OtTexture::sampler) == 0) {
			OtLogFatal("Can't add texture without [sampler] usage to compute pass");
		}

		SDL_GPUTextureSamplerBinding binding{
			.texture = texture.getTexture(),
			.sampler = sampler.getSampler()
		};

		samplers.emplace_back(binding);
	}

	// add an input sampler for a cubemap
	inline void addInputSampler(OtSampler& sampler, OtCubeMap& cubemap) {
		SDL_GPUTextureSamplerBinding binding{
			.texture = cubemap.getTexture(),
			.sampler = sampler.getSampler()
		};

		samplers.emplace_back(binding);
	}

	// add an output texture
	inline void addOutputTexture(OtTexture& texture) {
		if ((texture.getUsage() & (OtTexture::Usage::computeStorageWrite | OtTexture::Usage::computeStorageReadWrite)) == 0) {
			OtLogFatal("Can't add output texture to compute pass without [write] usage");
		}

		SDL_GPUStorageTextureReadWriteBinding binding{
			.texture = texture.getTexture(),
			.mip_level = 0,
			.layer = 0,
			.cycle = false,
			.padding1 = 0,
			.padding2 = 0,
			.padding3 = 0
		};

		textures.emplace_back(binding);
	}

	// add an output cubemap
	inline void addOutputCubeMap(OtCubeMap& cubemap) {
		SDL_GPUStorageTextureReadWriteBinding binding{
			.texture = cubemap.getTexture(),
			.mip_level = 0,
			.layer = 0,
			.cycle = false,
			.padding1 = 0,
			.padding2 = 0,
			.padding3 = 0
		};

		textures.emplace_back(binding);
	}

	// add an output buffer
	inline void addOutputBuffer() {
	}

	// add a set of uniforms
	inline void addUniforms(const void* data, size_t size) {
		auto& uniform = uniforms.emplace_back();
		uniform.resize(size);
		std::memcpy(uniform.data(), data, size);
	}

	// execute a compute shader pass
	inline void execute(OtComputePipeline& pipeline, size_t groupCountX, size_t groupCountY, size_t groupCountZ) {
		SDL_GPUComputePass* pass = SDL_BeginGPUComputePass(
			OtGpu::instance().pipelineCommandBuffer,
			textures.data(),
			static_cast<Uint32>(textures.size()),
			nullptr,
			0);

		if (samplers.size()) {
			SDL_BindGPUComputeSamplers(pass, 0, samplers.data(), static_cast<Uint32>(samplers.size()));
		}

		SDL_BindGPUComputePipeline(pass, pipeline.getPipeline());

		for (size_t i = 0; i < uniforms.size(); i++) {
			SDL_PushGPUComputeUniformData(
				OtGpu::instance().pipelineCommandBuffer,
				static_cast<Uint32>(i),
				uniforms[i].data(),
				static_cast<Uint32>(uniforms[i].size()));
		}

		// run the pipeline
		SDL_DispatchGPUCompute(
			pass,
			static_cast<Uint32>(groupCountX),
			static_cast<Uint32>(groupCountY),
			static_cast<Uint32>(groupCountZ));

		// cleanup
		SDL_EndGPUComputePass(pass);
		samplers.clear();
		textures.clear();
		uniforms.clear();
	}

private:
	// pass specific data
	std::vector<SDL_GPUTextureSamplerBinding> samplers;
	std::vector<SDL_GPUStorageTextureReadWriteBinding> textures;
	std::vector<std::vector<std::byte>> uniforms;
};
