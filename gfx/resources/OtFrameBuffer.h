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
	void initialize(OtTexture::Format c, OtTexture::Format d=OtTexture::Format::none);

	// clear all resources
	void clear();

	// see if framebuffer is valid
	inline bool isValid() { return valid; }

	// set clear properties
	void setClearColor(bool flag, const glm::vec4& value=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	void setClearDepth(bool flag, float value=1.0f);
	void setClearStencil(bool flag, uint8_t value=0);

	// update frame buffer
	bool update(int w, int h);

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
	SDL_GPUColorTargetInfo colorTargetInfo;
	SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo;
	OtRenderTargetInfo info;

	// get render target information
	friend class OtRenderPass;
	inline OtRenderTargetInfo* getRenderTargetInfo() override { return &info; }
};
