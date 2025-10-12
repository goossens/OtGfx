//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtLog.h"

#include "OtGpu.h"
#include "OtSampler.h"


//
//	OtSampler::clear
//

void OtSampler::clear() {
	sampler = nullptr;
	requestedFilter = Filter::linear;
	currentFilter = Filter::none;
	requestedAddressingX = Addressing::repeat;
	requestedAddressingY = Addressing::repeat;
	currentAddressingX = Addressing::none;
	currentAddressingY = Addressing::none;
}


//
//	OtSampler::assign
//

void OtSampler::assign(SDL_GPUSampler* newSampler) {
	sampler = std::shared_ptr<SDL_GPUSampler>(
		newSampler,
		[](SDL_GPUSampler* oldSampler) {
			SDL_ReleaseGPUSampler(OtGpu::instance().device, oldSampler);
		});
}


//
//	OtSampler::getSampler
//

SDL_GPUSampler* OtSampler::getSampler() {
	if (!sampler || requestedFilter != currentFilter || requestedAddressingX != currentAddressingX || requestedAddressingY != currentAddressingY) {
		if (requestedFilter == Filter::none) {
			OtLogFatal("Invalid filter type for sampler");
		}

		if (requestedAddressingX == Addressing::none) {
			OtLogFatal("Invalid addressing type for sampler");
		}

		if (requestedAddressingY == Addressing::none) {
			OtLogFatal("Invalid addressing type for sampler");
		}

		SDL_GPUFilter filter =
			(requestedFilter == Filter::nearest)
				? SDL_GPU_FILTER_NEAREST
				: SDL_GPU_FILTER_LINEAR;

		SDL_GPUSamplerAddressMode addressModeX =
			(requestedAddressingX == Addressing::repeat)
				? SDL_GPU_SAMPLERADDRESSMODE_REPEAT
				: (requestedAddressingX == Addressing::mirror)
					? SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT
					: SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

		SDL_GPUSamplerAddressMode addressModeY =
			(requestedAddressingY == Addressing::repeat)
				? SDL_GPU_SAMPLERADDRESSMODE_REPEAT
				: (requestedAddressingY == Addressing::mirror)
					? SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT
					: SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

		SDL_GPUSamplerCreateInfo info{
			.min_filter = filter,
			.mag_filter = filter,
			.mipmap_mode = requestedFilter == Filter::nearest ? SDL_GPU_SAMPLERMIPMAPMODE_NEAREST : SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
			.address_mode_u = addressModeX,
			.address_mode_v = addressModeY,
			.address_mode_w = addressModeX,
			.mip_lod_bias = 0,
			.max_anisotropy = requestedFilter == Filter::anisotropic ? 4.0f : 0.0f,
			.compare_op = SDL_GPU_COMPAREOP_INVALID,
			.min_lod = 0.0f,
			.max_lod = 0.0f,
			.enable_anisotropy = requestedFilter == Filter::anisotropic,
			.enable_compare = false,
			.padding1 = 0,
			.padding2 = 0,
			.props = 0
		};

		auto sdlSampler =SDL_CreateGPUSampler(OtGpu::instance().device, &info);

		if (sdlSampler == nullptr) {
			OtLogFatal("Error in SDL_CreateGPUSampler: {}", SDL_GetError());
		}

		assign(sdlSampler);
		currentFilter = requestedFilter;
		currentAddressingX = requestedAddressingX;
		currentAddressingY = requestedAddressingY;
	}

	return sampler.get();
}
