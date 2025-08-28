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
	// flags
	static constexpr int nearestSampling = 1 << 0;
	static constexpr int linearSampling = 1 << 1;
	static constexpr int anisotropicSampling = 1 << 2;
	static constexpr int repeatSampling = 1 << 3;
	static constexpr int mirrorSampling = 1 << 4;
	static constexpr int clampSampling = 1 << 5;

	// constructors
	OtSampler() = default;
	OtSampler(int flags) : requestedFlags(flags) {}

	// clear the object
	inline void clear() { sampler = nullptr; }

	// see if sampler is valid
	inline bool isValid() { return sampler != nullptr; }

	// access flags
	inline void setFlags(int flags) { requestedFlags = flags; }
	inline int getFlags() { return requestedFlags; };

private:
	// sampler
	std::shared_ptr<SDL_GPUSampler> sampler;

	// properties
	int requestedFlags = linearSampling | repeatSampling;
	int createdFlags = 0;

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

	inline SDL_GPUSampler* getSampler() {
		if (!sampler || requestedFlags != createdFlags) {
			SDL_GPUFilter filter =
				(requestedFlags & nearestSampling)
					? SDL_GPU_FILTER_NEAREST
					: SDL_GPU_FILTER_LINEAR;

			SDL_GPUSamplerAddressMode addressMode =
				(requestedFlags & repeatSampling)
					? SDL_GPU_SAMPLERADDRESSMODE_REPEAT
					: (requestedFlags & mirrorSampling)
						? SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT
						: SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

			SDL_GPUSamplerCreateInfo info{
				.min_filter = filter,
				.mag_filter = filter,
				.address_mode_u = addressMode,
				.address_mode_v = addressMode,
				.address_mode_w = addressMode,
				.mip_lod_bias = 0,
				.max_anisotropy = (requestedFlags | anisotropicSampling) ? 8.0f : 0.0f,
				.compare_op = SDL_GPU_COMPAREOP_ALWAYS,
				.min_lod = 0,
				.max_lod = 0,
				.enable_anisotropy = (requestedFlags | anisotropicSampling) != 0,
				.enable_compare = false
			};

			assign(SDL_CreateGPUSampler(OtGpu::instance().device, &info));

			if (sampler == nullptr) {
				OtLogFatal("Error in SDL_CreateGPUSampler: {}", SDL_GetError());
			}

			createdFlags = requestedFlags;
		}

		return sampler.get();
	}
};
