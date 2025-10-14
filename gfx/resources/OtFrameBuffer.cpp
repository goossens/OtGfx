//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtFrameBuffer.h"


//
//	OtFrameBuffer::initialize
//

void OtFrameBuffer::initialize(OtTexture::Format c, OtTexture::Format d) {
	if (colorTextureType != c || depthTextureType != d) {
		clear();
		colorTextureType = c;
		depthTextureType = d;
	}
}


//
//	OtFrameBuffer::clear
//

void OtFrameBuffer::clear() {
	// release resources (if required)
	colorTexture.clear();
	depthTexture.clear();

	// clear other fields
	width = -1;
	height = -1;
	valid = false;
}


//
//	OtFrameBuffer::setClearColor
//

void OtFrameBuffer::setClearColor(bool flag, const glm::vec4& value) {
	clearColorTexture = flag;
	clearColorValue = value;
	valid = false;
}


//
//	OtFrameBuffer::setClearDepth
//

void OtFrameBuffer::setClearDepth(bool flag, float value) {
	clearDepthTexture = flag;
	clearDepthValue = value;
	valid = false;
}


//
//	OtFrameBuffer::setClearStencil
//

void OtFrameBuffer::setClearStencil(bool flag, uint8_t value) {
	clearStencilTexture = flag;
	clearStencilValue = value;
	valid = false;
}


//
//	OtFrameBuffer::update
//

bool OtFrameBuffer::update(int w, int h) {
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
				OtTexture::Usage(OtTexture::Usage::depthStencilTarget | OtTexture::Usage::sampler));
		}

		// remember dimensions
		width = w;
		height = h;

		// create/update render target information
		colorTargetInfo = SDL_GPUColorTargetInfo{};
		colorTargetInfo.texture = colorTexture.getTexture();

		colorTargetInfo.clear_color = SDL_FColor{
			.r = clearColorValue.r,
			.g = clearColorValue.g,
			.b = clearColorValue.b,
			.a = clearColorValue.a
		};

		colorTargetInfo.load_op = hasColorTexture() && clearColorTexture ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		depthStencilTargetInfo = SDL_GPUDepthStencilTargetInfo{};
		depthStencilTargetInfo.texture = depthTexture.getTexture();
		depthStencilTargetInfo.clear_depth = clearDepthValue;
		depthStencilTargetInfo.load_op = hasDepthTexture() && clearDepthTexture ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		depthStencilTargetInfo.stencil_load_op = hasStencilTexture() && clearStencilTexture ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;
		depthStencilTargetInfo.clear_stencil = static_cast<Uint8>(clearStencilValue);

		info.colorTargetInfo = hasColorTexture() ? &colorTargetInfo : nullptr;
		info.numColorTargets = hasColorTexture() ? 1 : 0;
		info.depthStencilTargetInfo = hasDepthTexture() ? &depthStencilTargetInfo : nullptr;

		// set state
		valid = true;
		return true;

	} else {
		return false;
	}
}
