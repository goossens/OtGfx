//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <string>

#include "glm/glm.hpp"
#include "SDL3/SDL_surface.h"


//
//	OtImage
//

class OtImage {
public:
	// image types
	static constexpr int rgba8Image = SDL_PIXELFORMAT_RGBA8888;
	static constexpr int rgbaFloat32Image = SDL_PIXELFORMAT_RGBA128_FLOAT;

	// constructors/destructor
	OtImage() = default;
	OtImage(const std::string& path, bool powerof2=false, bool square=false);
	~OtImage();

	// clear the resources
	void clear();

	// create/update an image
	void update(int width, int height, int format);

	// load image
	void load(const std::string& address, bool powerof2=false, bool square=false);
	void load(int width, int height, int format, void* pixels);
	void load(void* data, size_t size);

	// save the image to disk
	void saveToPNG(const std::string& path);

	// see if image is valid
	inline bool isValid() { return image != nullptr; }

	// get information about image
	inline int getFormat() { return image->format; }
	inline int getWidth() { return image->w; }
	inline int getHeight() { return image->h; }
	inline void* getPixels() { return image->pixels; }
	inline int getPitch() { return image->pitch; }

	// get pixel values
	glm::vec4 getPixelRgba(int x, int y);
	inline float getPixelGray(int x, int y) { return getPixelRgba(x, y).x; }

	glm::vec4 sampleValueRgba(float u, float v);
	float sampleValueGray(float u, float v);

	// version management
	inline void setVersion(int v) { version = v; }
	inline int getVersion() { return version; }
	inline void incrementVersion() { version++; }

	// see if images are identical
	inline bool operator==(OtImage& rhs) {
		return image == rhs.image && version == rhs.version;
	}

	inline bool operator!=(OtImage& rhs) {
		return !operator==(rhs);
	}

private:
	// the actual image
	SDL_Surface* image;
	int version = 0;
};
