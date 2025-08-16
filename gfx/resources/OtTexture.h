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

#include "OtLibuv.h"

#include "OtImage.h"


//
//	OtTexture
//

class OtTexture {
public:
	// texture types
	static constexpr int noTexture = 0;
	static constexpr int r8Texture = SDL_GPU_TEXTUREFORMAT_R8_UINT;
	static constexpr int rFloat32Texture = SDL_GPU_TEXTUREFORMAT_R32_FLOAT;
	static constexpr int rgFloat16Texture = SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT;
	static constexpr int rgba8Texture = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UINT;
	static constexpr int rgbaFloat16Texture = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	static constexpr int rgbaFloat32Texture = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
	static constexpr int dFloatTexture = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	static constexpr int d24s8Texture = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;

	// constructors
	OtTexture() = default;
	OtTexture(int w, int h, int f) { create(w, h, f); }

	// clear the resources
	void clear();

	// see if texture is valid
	inline bool isValid() { return texture.isValid(); }

	// create an empty texture
	void create(int width, int height, int format);

	// load texture
	void load(OtImage& image);
	void load(const std::string& address, bool async=false);
	void load(int width, int height, int format, void* pixels, bool async=false);
	void load(void* data, size_t size, bool async=false);

	// update (part of) texture
	void update(int x, int y, int width, int height, void* pixels);

	// return texture ID (for Dear ImGUI use)
	inline ImTextureID getTextureID() { return isValid() ? static_cast<ImTextureID>(texture.getIndex()) : ImTextureID_Invalid; }

	// get texture properties
	inline int getWidth() { return width; }
	inline int getHeight() { return height; }
	inline int getFormat() { return format; }

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

	// see if format has depth component
	static inline bool hasDepth(int format) {
		if (format) {
			bimg::ImageBlockInfo info = bimg::getBlockInfo(bimg::TextureFormat::Enum(format));
			return info.depthBits > 0;

		} else {
			return false;
		}
	}

	// see if format has stencil component
	static inline bool hasStencil(int format) {
		if (format) {
			bimg::ImageBlockInfo info = bimg::getBlockInfo(bimg::TextureFormat::Enum(format));
			return info.stencilBits > 0;

		} else {
			return false;
		}
	}

private:
	// texture
	OtBgfxHandle<bgfx::TextureHandle> texture;
	int width = 1;
	int height = 1;
	int format = noTexture;
	int version = 0;

	// support for async loading
	std::shared_ptr<OtImage> asyncImage;
	uv_async_t* asyncHandle = nullptr;
	void loadAsync();
};
