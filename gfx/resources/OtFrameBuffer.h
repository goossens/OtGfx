//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <cstdint>

#include "imgui.h"
#include "SDL3/SDL.h"

#include "OtLog.h"

#include "OtRenderTarget.h"
#include "OtTexture.h"


//
//	OtFrameBuffer
//

class OtFrameBuffer : OtRenderTarget {
public:
	// constructors
	OtFrameBuffer() = default;

	OtFrameBuffer(OtTexture::Format colorTextureType, OtTexture::Format depthTextureType=OtTexture::Format::none) {
		initialize(colorTextureType, depthTextureType);
	}

	// initialize framebuffer
	inline void initialize(OtTexture::Format c, OtTexture::Format d=OtTexture::Format::none) {
		if (colorTextureType != c || depthTextureType != d) {
			clear();
			colorTextureType = c;
			depthTextureType = d;
		}
	}

	// clear all resources
	inline void clear() {
		// release resources (if required)
		colorTexture.clear();
		depthTexture.clear();

		// clear other fields
		width = -1;
		height = -1;
		valid = false;
	}

	// see if framebuffer is valid
	inline bool isValid() { return valid; }

	// set clear properties
	inline void setClearColor(bool flag, const glm::vec4& value=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
		clearColorTexture = flag;
		clearColorValue = value;
		valid = false;
	}

	inline void setClearDepth(bool flag, float value=1.0f) {
		clearDepthTexture = flag;
		clearDepthValue = value;
		valid = false;
	}

	inline void setClearStencil(bool flag, uint8_t value=0) {
		clearStencilTexture = flag;
		clearStencilValue = value;
		valid = false;
	}

	// update frame buffer
	inline bool update(int w, int h) {
		// update framebuffer if required
		if (!valid || w != width || h != height) {
			// clear old resources
			clear();

			if (colorTextureType == OtTexture::Format::none && depthTextureType == OtTexture::Format::none) {
				OtLogFatal("Internal error: you can't have a FrameBuffer without Textures");
			}

			// create new textures (if required)
			if (colorTextureType != OtTexture::Format::none) {
				colorTexture.update(
					w, h,
					colorTextureType,
					OtTexture::OtTexture::Usage::rwDefault);
			}

			if (depthTextureType != OtTexture::Format::none) {
				depthTexture.update(
					w, h,
					depthTextureType,
					OtTexture::Usage(OtTexture::Usage::depthStencilTarget));
			}

			// remember dimensions
			width = w;
			height = h;

			// create/update render target information
			colorTargetInfo.texture = colorTexture.getTexture();

			colorTargetInfo.clear_color = SDL_FColor{
				.r = clearColorValue.r,
				.g = clearColorValue.g,
				.b = clearColorValue.b,
				.a = clearColorValue.a
			};

			colorTargetInfo.load_op = hasColorTexture() && clearColorTexture ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
			colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

			depthStencilTargetInfo.texture = depthTexture.getTexture();
			depthStencilTargetInfo.clear_depth = clearDepthValue;
			depthStencilTargetInfo.load_op = hasDepthTexture() && clearDepthTexture ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
			depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
			depthStencilTargetInfo.stencil_load_op = hasStencilTexture() && clearStencilTexture ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
			depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;
			depthStencilTargetInfo.clear_stencil = static_cast<Uint8>(clearStencilValue);

			info.colorTargetInfo = &colorTargetInfo;
			info.numColorTargets = 1;
			info.depthStencilTargetInfo = hasDepthTexture() ? &depthStencilTargetInfo : nullptr;

			// set state
			valid = true;
			return true;

		} else {
			return false;
		}
	}

	// get framebuffer dimensions
	inline int getWidth() { return width; }
	inline int getHeight() { return height; }

	// see if textures are available
	inline bool hasColorTexture() { return colorTextureType != OtTexture::Format::none; }
	inline bool hasDepthTexture() { return depthTexture.hasDepth(); }
	inline bool hasStencilTexture() { return depthTexture.hasStencil(); }

	// get texture types
	inline OtTexture::Format getColorTextureType() { return colorTextureType; }
	inline OtTexture::Format getDepthTextureType() { return depthTextureType; }

	// get textures
	inline OtTexture& getColorTexture() { return colorTexture; }
	inline OtTexture& getDepthTexture() { return depthTexture; }

	// get texture IDs (for Dear ImGUI use)
	inline ImTextureID getColorTextureID() { return colorTexture.getTextureID(); }
	inline ImTextureID getDepthTextureID() { return depthTexture.getTextureID(); }

private:
	// properties
	OtTexture::Format colorTextureType = OtTexture::Format::none;
	OtTexture::Format depthTextureType = OtTexture::Format::none;

	// dimensions of framebuffer
	int width = -1;
	int height = -1;

	// state
	bool valid = false;

	// resources
	OtTexture colorTexture;
	OtTexture depthTexture;

	// clearing flags
	bool clearColorTexture = true;
	bool clearDepthTexture = true;
	bool clearStencilTexture = true;
	glm::vec4 clearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
	float clearDepthValue = 1.0f;
	std::uint8_t clearStencilValue = 0;

	// render target description
	SDL_GPUColorTargetInfo colorTargetInfo{};
	SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{};
	OtRenderTargetInfo info;

	// get render target information
	friend class OtRenderPass;
	OtRenderTargetInfo* getRenderTargetInfo() override { return &info; }
};
