//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "SDL3/SDL.h"


//
//	OtRenderTargetInfo
//

class OtRenderTargetInfo {
public:
	SDL_GPUColorTargetInfo *colorTargetInfo;
	Uint32 numColorTargets;
	SDL_GPUDepthStencilTargetInfo *depthStencilTargetInfo;
};


//
//	OtRenderTarget
//

class OtRenderTarget {
public:
	// destructor
	virtual ~OtRenderTarget() {}

	// get render target information
	virtual OtRenderTargetInfo* getRenderTargetInfo() = 0;
};
