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

#include "OtRenderTarget.h"
#include "OtTexture.h"


//
//	OtGbuffer
//

class OtGbuffer : public OtRenderTarget {
public:
	// clear all resources
	inline void clear() {
		// release resources (if required)
		albedoTexture.clear();
		normalTexture.clear();
		pbrTexture.clear();
		emissiveTexture.clear();
		depthTexture.clear();

		// clear other fields
		width = -1;
		height = -1;
		valid = false;


	}

	// see if gbuffer is valid
	inline bool isValid() { return valid; }

	// update gbuffer
	inline bool update(int w, int h) {
	// update framebuffer if required
		if (!valid || w != width || h != height) {
			// clear old resources
			clear();

			// create new textures
			auto w32 = static_cast<Uint32>(w);
			auto h32 = static_cast<Uint32>(h);
			auto colorUsage = OtTexture::Usage(OtTexture::Usage::colorTarget | OtTexture::Usage::sampler);
			auto depthUsage = OtTexture::Usage(OtTexture::Usage::depthStencilTarget | OtTexture::Usage::sampler);

			albedoTexture.update(w32, h32, OtTexture::Format::rgba32, colorUsage);
			normalTexture.update(w32, h32, OtTexture::Format::rgba8, colorUsage);
			pbrTexture.update(w32, h32, OtTexture::Format::rgba8, colorUsage);
			emissiveTexture.update(w32, h32, OtTexture::Format::rgba8, colorUsage);
			depthTexture.update(w32, h32, OtTexture::Format::d32, depthUsage);

			// create/update render target information
			colorTargetInfo[0].texture = albedoTexture.getTexture();
			colorTargetInfo[0].clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
			colorTargetInfo[0].load_op = SDL_GPU_LOADOP_CLEAR;
			colorTargetInfo[0].store_op = SDL_GPU_STOREOP_STORE;

			colorTargetInfo[1].texture = normalTexture.getTexture();
			colorTargetInfo[1].clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
			colorTargetInfo[1].load_op = SDL_GPU_LOADOP_CLEAR;
			colorTargetInfo[1].store_op = SDL_GPU_STOREOP_STORE;

			colorTargetInfo[2].texture = pbrTexture.getTexture();
			colorTargetInfo[2].clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
			colorTargetInfo[2].load_op = SDL_GPU_LOADOP_CLEAR;
			colorTargetInfo[2].store_op = SDL_GPU_STOREOP_STORE;

			colorTargetInfo[3].texture = emissiveTexture.getTexture();
			colorTargetInfo[3].clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
			colorTargetInfo[3].load_op = SDL_GPU_LOADOP_CLEAR;
			colorTargetInfo[3].store_op = SDL_GPU_STOREOP_STORE;

			depthStencilTargetInfo.texture = depthTexture.getTexture();
			depthStencilTargetInfo.clear_depth = 1.0f;
			depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

			info.colorTargetInfo = colorTargetInfo;
			info.numColorTargets = 4;
			info.depthStencilTargetInfo = &depthStencilTargetInfo;

			// remember dimensions and set state
			width = w;
			height = h;
			valid = true;
			return true;

		} else {
			return false;
		}
	}

	// get framebuffer dimensions
	inline int getWidth() { return width; }
	inline int getHeight() { return height; }

		// get textures
	inline OtTexture& getAlbedoTexture() { return albedoTexture; }
	inline OtTexture& getNormalTexture() { return normalTexture; }
	inline OtTexture& getPbrTexture() { return pbrTexture; }
	inline OtTexture& getEmissiveTexture() { return emissiveTexture; }
	inline OtTexture& getDepthTexture() { return depthTexture; }

	// get texture IDs (for Dear ImGUI use)
	inline ImTextureID getAlbedoTextureID() { return albedoTexture.getTextureID(); }
	inline ImTextureID getNormalTextureID() { return normalTexture.getTextureID(); }
	inline ImTextureID getPbrTextureID() { return pbrTexture.getTextureID(); }
	inline ImTextureID getEmissiveTextureID() { return emissiveTexture.getTextureID(); }
	inline ImTextureID getDepthTextureID() { return depthTexture.getTextureID(); }

private:
	// dimensions of gbuffer
	int width = -1;
	int height = -1;

	// state
	bool valid = false;

	// resources
	OtTexture albedoTexture;
	OtTexture normalTexture;
	OtTexture pbrTexture;
	OtTexture emissiveTexture;
	OtTexture depthTexture;

	// render target description
	SDL_GPUColorTargetInfo colorTargetInfo[4];
	SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo;
	OtRenderTargetInfo info;

	// get render target information
	friend class OtRenderPass;
	OtRenderTargetInfo* getRenderTargetInfo() override { return &info; }
};
