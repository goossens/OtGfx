//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "SDL3/SDL_gpu.h"

#include "OtGpu.h"
#include "OtTexture.h"


//
//	OtBlitPass
//

class OtBlitPass {
public:
	// blit entire texture to other texture
	static void blit(OtTexture& src, OtTexture& dst) {
		SDL_GPUBlitRegion srcRegion{
			.texture = src.getTexture(),
			.mip_level = 0,
			.layer_or_depth_plane = 0,
			.x = 0,
			.y = 0,
			.w = static_cast<Uint32>(src.getWidth()),
			.h = static_cast<Uint32>(src.getHeight())
		};

		SDL_GPUBlitRegion dstRegion{
			.texture = dst.getTexture(),
			.mip_level = 0,
			.layer_or_depth_plane = 0,
			.x = 0,
			.y = 0,
			.w = static_cast<Uint32>(dst.getWidth()),
			.h = static_cast<Uint32>(dst.getHeight())
		};

		SDL_GPUBlitInfo info{
			.source = srcRegion,
			.destination = dstRegion,
			.load_op = SDL_GPU_LOADOP_LOAD,
			.flip_mode = SDL_FLIP_NONE,
			.filter = SDL_GPU_FILTER_NEAREST,
			.cycle = false
		};

		SDL_BlitGPUTexture(OtGpu::instance().pipelineCommandBuffer, &info);
	}
};
