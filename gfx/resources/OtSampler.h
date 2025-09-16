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
	OtSampler(Filter filter, Addressing address) : requestedFilter(filter), requestedAddressing(address) {}

	// clear the object
	inline void clear() { sampler = nullptr; }

	// see if sampler is valid
	inline bool isValid() { return sampler != nullptr; }

	// access options
	inline void setFilter(Filter filter) { requestedFilter = filter; }
	inline Filter getFilter() { return requestedFilter; };
	inline void setAddressing(Addressing address) { requestedAddressing = address; }
	inline Addressing getAddressing() { return requestedAddressing; };

private:
	// sampler
	std::shared_ptr<SDL_GPUSampler> sampler;

	// properties
	Filter requestedFilter = Filter::linear;
	Addressing requestedAddressing = Addressing::repeat;
	Filter currentFilter = Filter::none;
	Addressing currentAddressing = Addressing::none;

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
		if (!sampler || requestedFilter != currentFilter || requestedAddressing != currentAddressing) {
			if (requestedFilter == Filter::none) {
				OtLogFatal("Invalid filter type for sampler");
			}

			if (requestedAddressing == Addressing::none) {
				OtLogFatal("Invalid addressing type for sampler");
			}

			SDL_GPUFilter filter =
				(requestedFilter == Filter::nearest)
					? SDL_GPU_FILTER_NEAREST
					: SDL_GPU_FILTER_LINEAR;

			SDL_GPUSamplerAddressMode addressMode =
				(requestedAddressing == Addressing::repeat)
					? SDL_GPU_SAMPLERADDRESSMODE_REPEAT
					: (requestedAddressing == Addressing::mirror)
						? SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT
						: SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

			SDL_GPUSamplerCreateInfo info{
				.min_filter = filter,
				.mag_filter = filter,
				.address_mode_u = addressMode,
				.address_mode_v = addressMode,
				.address_mode_w = addressMode,
				.mip_lod_bias = 0,
				.max_anisotropy = (requestedFilter == Filter::anisotropic) ? 8.0f : 0.0f,
				.compare_op = SDL_GPU_COMPAREOP_ALWAYS,
				.min_lod = 0,
				.max_lod = 0,
				.enable_anisotropy = (requestedFilter == Filter::anisotropic) != 0,
				.enable_compare = false
			};

			auto sdlSampler =SDL_CreateGPUSampler(OtGpu::instance().device, &info);

			if (sdlSampler == nullptr) {
				OtLogFatal("Error in SDL_CreateGPUSampler: {}", SDL_GetError());
			}

			assign(sdlSampler);
			currentFilter = requestedFilter;
			currentAddressing = requestedAddressing;
		}

		return sampler.get();
	}
};
