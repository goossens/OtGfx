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

#include "SDL3/SDL.h"

#include "OtGpu.h"


//
//	OtSampler
//

class OtSampler {
public:
	// filtering options
	enum class Filter {
		none,
		nearest,
		linear,
		anisotropic
	};

	enum class Addressing {
		none,
		repeat,
		mirror,
		clamp
	};

	// constructors
	OtSampler() = default;
	OtSampler(Filter filter, Addressing addressing) : requestedFilter(filter), requestedAddressingX(addressing), requestedAddressingY(addressing) {}
	OtSampler(Filter filter, Addressing addressingX, Addressing addressingY) : requestedFilter(filter), requestedAddressingX(addressingX), requestedAddressingY(addressingY) {}

	// clear the object
	inline void clear() { sampler = nullptr; }

	// see if sampler is valid
	inline bool isValid() { return sampler != nullptr; }

	// access options
	inline void setFilter(Filter filter) { requestedFilter = filter; }
	inline Filter getFilter() { return requestedFilter; };

	inline void setAddressing(Addressing addressing) {
		requestedAddressingX = addressing;
		requestedAddressingY = addressing;
	}

	inline void setAddressingX(Addressing addressing) { requestedAddressingX = addressing; }
	inline void setAddressingY(Addressing addressing) { requestedAddressingY = addressing; }
	inline Addressing getAddressingX() { return requestedAddressingX; };
	inline Addressing getAddressingY() { return requestedAddressingY; };

private:
	// sampler
	std::shared_ptr<SDL_GPUSampler> sampler;

	// properties
	Filter requestedFilter = Filter::linear;
	Filter currentFilter = Filter::none;
	Addressing requestedAddressingX = Addressing::repeat;
	Addressing requestedAddressingY = Addressing::repeat;
	Addressing currentAddressingX = Addressing::none;
	Addressing currentAddressingY = Addressing::none;

	// memory manage SDL resource
	inline void assign(SDL_GPUSampler* newSampler) {
		sampler = std::shared_ptr<SDL_GPUSampler>(
			newSampler,
			[](SDL_GPUSampler* oldSampler) {
				SDL_ReleaseGPUSampler(OtGpu::instance().device, oldSampler);
			});
	}

	// get the raw sampler object
	friend class OtComputePass;
	friend class OtRenderPass;

	inline SDL_GPUSampler* getSampler() {
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
				.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
				.address_mode_u = addressModeX,
				.address_mode_v = addressModeY,
				.address_mode_w = addressModeX,
				.mip_lod_bias = 0,
				.max_anisotropy = (requestedFilter == Filter::anisotropic) ? 8.0f : 0.0f,
				.compare_op = SDL_GPU_COMPAREOP_ALWAYS,
				.min_lod = 0,
				.max_lod = 0,
				.enable_anisotropy = (requestedFilter == Filter::anisotropic) != 0,
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
};
