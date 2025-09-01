//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <algorithm>
#include <fstream>

#include "SDL3/SDL_iostream.h"
#include "SDL3_image/SDL_image.h"

#include "OtAssert.h"
#include "OtException.h"
#include "OtLog.h"
#include "OtNumbers.h"
#include "OtText.h"

#include "OtImage.h"
#include "OtFrameworkAtExit.h"
#include "OtUrl.h"


//
//	OtImage::OtImage
//

OtImage::OtImage(const std::string& path, bool powerof2, bool square) {
	load(path, powerof2, square);
}


//
//	OtImage::clear
//

void OtImage::clear() {
	if (surface) {
		surface = nullptr;
		incrementVersion();
	}
}


//
//	OtImage::update
//

void OtImage::update(int width, int height, int format) {
	if (!surface || surface->w != width || surface->h != height || surface->format != format) {
		// create new image
		auto sdlSurface = SDL_CreateSurface(width, height, static_cast<SDL_PixelFormat>(format));

		if (!sdlSurface) {
			OtLogFatal("Error in SDL_CreateSurface: {}", SDL_GetError());
		}

		assign(sdlSurface);
	}
}


//
//	isPowerOfTwo
//

static bool isPowerOfTwo(int n) {
	// handle the special case of 0, which is not a power of two
	if (n <= 0) {
		return false;

	} else {
	    // check if n has only one bit set using the bitwise AND trick
	    return (n & (n - 1)) == 0;
	}
}

//
//	OtImage::load
//

void OtImage::load(const std::string& address, bool powerof2, bool square) {
	// do we have a URL?
	if (OtText::startsWith(address, "http:") || OtText::startsWith(address, "https:")) {
		OtUrl url(address);
		url.download();
		load(url.getDownloadedData(), url.getDownloadedSize());

	} else {
		// load image from file
		auto sdlSurface = IMG_Load(address.c_str());

		if (!sdlSurface) {
			OtLogFatal("Error in IMG_Load: {}", SDL_GetError());
		}

		assign(sdlSurface);
		normalize();
	}

	// validate sides are power of 2 (if required)
	if (powerof2 && !(isPowerOfTwo(surface->w))) {
		clear();
		OtLogError("Image width {} is not a power of 2", surface->w);
	}

	if (powerof2 && !(isPowerOfTwo(surface->h))) {
		clear();
		OtLogError("Image height {} is not a power of 2", surface->h);
	}

	// validate squareness (if required)
	if (square && surface->w != surface->h) {
		clear();
		OtLogError("Image must be square not {} by {}", surface->w, surface->h);
	}
}


//
//	OtImage::load
//

void OtImage::load(int width, int height, int format, void* pixels) {
	// load new image
	auto sdlSurface = SDL_CreateSurfaceFrom(
		width,
		height,
		static_cast<SDL_PixelFormat>(format),
		pixels,
		SDL_BYTESPERPIXEL(format) * width);

	if (!sdlSurface) {
		OtLogFatal("Error in IMG_Load_IO: {}", SDL_GetError());
	}

	assign(sdlSurface);
	normalize();
}


//
//	OtImage::load
//

void OtImage::load(void* data, size_t size) {
	// create the image
	SDL_IOStream* io = SDL_IOFromMem(data, size);

	if (!io) {
		OtLogFatal("Error in SDL_IOFromMem: {}", SDL_GetError());
	}

	auto sdlSurface = IMG_Load_IO(io, true);

	if (!sdlSurface) {
		OtLogFatal("Error in IMG_Load_IO: {}", SDL_GetError());
	}

	assign(sdlSurface);
	normalize();
}


//
//	OtImage::saveToPNG
//

void OtImage::saveToPNG(const std::string& path) {
	// sanity check
	OtAssert(isValid());

	// write image to file
	if (!IMG_SavePNG(surface.get(), path.c_str())){
		OtLogFatal("Error in IMG_SavePNG: {}", SDL_GetError());
	}
}


//
//	OtImage::getPixelRgba
//

glm::vec4 OtImage::getPixelRgba(int x, int y) {
	// sanity check
	OtAssert(isValid());

	x = std::clamp(x, 0, surface->w - 1);
	y = std::clamp(y, 0, surface->h - 1);

	if (surface->format == SDL_PIXELFORMAT_RGBA8888) {
		auto value = &((uint8_t*) surface->pixels)[y * surface->w + x];
		return glm::vec4(value[0] / 255.0f, value[1] / 255.0f, value[2] / 255.0f, value[3] / 255.0f);

	} else if (surface->format == SDL_PIXELFORMAT_RGBA128_FLOAT) {
		auto value = &((float*) surface->pixels)[y * surface->w + x];
		return glm::vec4(value[0], value[1], value[2], value[3]);

	} else {
		OtLogFatal("Internal error: invalid pixel format");
		return glm::vec4();
	}
}


//
//	OtImage::sampleValueRgba
//

glm::vec4 OtImage::sampleValueRgba(float x, float y) {
	// sanity check
	OtAssert(isValid());

	x *= surface->w - 1;
	y *= surface->h - 1;

	int x1 = static_cast<int>(std::floor(x));
	int y1 = static_cast<int>(std::floor(y));
	int x2 = x1 + 1;
	int y2 = y1 + 1;

	auto h11 = getPixelRgba(x1, y1);
	auto h21 = getPixelRgba(x2, y1);
	auto h12 = getPixelRgba(x1, y2);
	auto h22 = getPixelRgba(x2, y2);

	auto hx1 = glm::mix(h11, h21, x - x1);
	auto hx2 = glm::mix(h12, h22, x - x1);
	return glm::mix(hx1, hx2, y - y1);
}


//
//	OtImage::sampleValueGray
//

float OtImage::sampleValueGray(float x, float y) {
	// sanity check
	OtAssert(isValid());

	x *= surface->w - 1;
	y *= surface->h - 1;

	int x1 = static_cast<int>(std::floor(x));
	int y1 = static_cast<int>(std::floor(y));
	int x2 = x1 + 1;
	int y2 = y1 + 1;

	auto h11 = getPixelGray(x1, y1);
	auto h21 = getPixelGray(x2, y1);
	auto h12 = getPixelGray(x1, y2);
	auto h22 = getPixelGray(x2, y2);

	auto hx1 = std::lerp(h11, h21, x - x1);
	auto hx2 = std::lerp(h12, h22, x - x1);
	return std::lerp(hx1, hx2, y - y1);
}


//
//	OtImage::normalize
//

void OtImage::normalize() {
	if (SDL_ISPIXELFORMAT_FLOAT(surface->format)) {
		if (surface->format != SDL_PIXELFORMAT_RGBA128_FLOAT) {
			auto sdlSurface = SDL_ConvertSurface(surface.get(), SDL_PIXELFORMAT_RGBA128_FLOAT);

			if (!sdlSurface) {
				OtLogFatal("Error in SDL_ConvertSurface: {}", SDL_GetError());
			}

			assign(sdlSurface);
		}

	} else {
		if (surface->format != SDL_PIXELFORMAT_ABGR8888) {
			auto sdlSurface = SDL_ConvertSurface(surface.get(), SDL_PIXELFORMAT_ABGR8888);

			if (!sdlSurface) {
				OtLogFatal("Error in SDL_ConvertSurface: {}", SDL_GetError());
			}

			assign(sdlSurface);
		}
	}
}
