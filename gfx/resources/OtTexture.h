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
#include <memory>
#include <string>

#include "imgui.h"
#include "SDL3/SDL.h"

#include "OtLibuv.h"

#include "OtGpu.h"
#include "OtImage.h"


//
//	OtTexture
//

class OtTexture {
public:
	// texture formats
	enum class Format {
		none = SDL_GPU_TEXTUREFORMAT_INVALID,
		r8 = SDL_GPU_TEXTUREFORMAT_R8_UNORM,
		rFloat32 = SDL_GPU_TEXTUREFORMAT_R32_FLOAT,
		rgFloat16 = SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT,
		rgba8 = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		rgbaFloat16 = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
		rgbaFloat32 = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
		dFloat = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		d24s8 = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT
	};

	// texture usage
	enum Usage {
		none = 0,
		sampler = SDL_GPU_TEXTUREUSAGE_SAMPLER,
		colorTarget = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
		depthStencilTarget = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
		graphicsStorageRead = SDL_GPU_TEXTUREUSAGE_GRAPHICS_STORAGE_READ,
		computeStorageRead = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ,
		computeStorageWrite = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE,
		computeStorageReadWrite = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_SIMULTANEOUS_READ_WRITE
	};

	// constructors
	OtTexture() = default;
	OtTexture(int width, int height, Format format, Usage usage) { update(width, height, format, usage); }

	// clear the texture
	void clear();

	// see if texture is valid
	inline bool isValid() { return texture != nullptr; }

	// update a texture (returns true if update was require, false if nothing needed to be done)
	bool update(int width, int height, Format format, Usage usage);

	// load texture
	void load(OtImage& image);
	void load(const std::string& address, bool async=false);
	void load(int width, int height, int format, void* pixels, bool async=false);
	void load(void* data, size_t size, bool async=false);

	// return texture ID (for Dear ImGUI use)
	inline ImTextureID getTextureID() {
		return isValid()
			? (ImTextureID)(intptr_t) texture.get()
			: ImTextureID_Invalid;
	}

	// get texture properties
	inline int getWidth() { return width; }
	inline int getHeight() { return height; }
	inline Format getFormat() { return format; }
	inline Usage getUsage() { return usage; }

	// version management
	inline void setVersion(int v) { version = v; }
	inline int getVersion() { return version; }
	inline void incrementVersion() { version++; }

	// see if textures are identical
	inline bool operator==(OtTexture& rhs) {
		return texture == rhs.texture && width == rhs.width && height == rhs.height && version == rhs.version;
	}

	inline bool operator!=(OtTexture& rhs) {
		return !operator==(rhs);
	}

	// see if texture has depth component
	inline bool hasDepth() {
		return (usage & depthStencilTarget) != 0;
	}

	// see if texture has stencil component
	inline bool hasStencil() {
		return (usage & depthStencilTarget) != 0 && format == Format::d24s8;
	}

private:
	// texture
	std::shared_ptr<SDL_GPUTexture> texture;

	// memory manage SDL resource
	inline void assign(SDL_GPUTexture* newTexture) {
		texture = std::shared_ptr<SDL_GPUTexture>(
			newTexture,
			[](SDL_GPUTexture* oldTexture) {
				SDL_ReleaseGPUTexture(OtGpu::instance().device, oldTexture);
			});

		incrementVersion();
	}

	// properties
	int width = 1;
	int height = 1;
	Format format = Format::none;
	Usage usage = Usage::none;
	int version = 0;

	// support for async loading
	std::shared_ptr<OtImage> asyncImage;
	uv_async_t* asyncHandle = nullptr;
	void loadAsync();

	// get the raw texture object
	friend class OtComputePass;
	friend class OtFrameBuffer;
	friend class OtGbuffer;

	inline SDL_GPUTexture* getTexture() {
		return isValid()
			? texture.get()
			: OtGpu::instance().transparentDummyTexture;
	}
};
