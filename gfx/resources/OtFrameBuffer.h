//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "imgui.h"
#include "SDL3/SDL.h"

#include "OtLog.h"

#include "OtTexture.h"


//
//	OtFrameBuffer
//

class OtFrameBuffer {
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
	}

	// see if framebuffer is valid
	inline bool isValid() { return colorTexture.isValid() || depthTexture.isValid(); }

	// update frame buffer
	inline  void update(int w, int h) {
		// update framebuffer if required
		if (!isValid() || w != width || h != height) {
			// clear old resources
			clear();

			if (colorTextureType == OtTexture::Format::none && depthTextureType == OtTexture::Format::none) {
				OtLogFatal("Internal error: you can't have a FrameBuffer without Textures");
			}

			// create new textures
			if (colorTextureType != OtTexture::Format::none) {
				depthTexture.update(
					w, h,
					colorTextureType,
					OtTexture::Usage(OtTexture::Usage::colorTarget | OtTexture::Usage::sampler));
			}

			if (depthTextureType != OtTexture::Format::none) {
				depthTexture.update(
					w, h,
					depthTextureType,
					OtTexture::Usage(OtTexture::Usage::colorTarget | OtTexture::Usage::sampler));
			}

			// remember dimensions
			width = w;
			height = h;
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

	// dimensions of framebuffer;
	int width = -1;
	int height = -1;

	// resource handles
	OtTexture colorTexture;
	OtTexture depthTexture;
};
