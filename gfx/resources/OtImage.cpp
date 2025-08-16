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
//	OtImage::~OtImage
//

OtImage::~OtImage() {
	if (image) {
		SDL_DestroySurface(image);
		image = nullptr;
	}
}


//
//	OtImage::clear
//

void OtImage::clear() {
	if (image) {
		SDL_DestroySurface(image);
		image = nullptr;
		incrementVersion();
	}
}


//
//	OtImage::update
//

void OtImage::update(int width, int height, int format) {
	if (!image || image->w != width || image->h != height || image->format != format) {
		// remove previous image (if required)
		clear();

		// create new image
		image = SDL_CreateSurface(width, height, static_cast<SDL_PixelFormat>(format));

		if (!image) {
			OtLogFatal("Error in SDL_CreateSurface: {}", SDL_GetError());
		}
	}
}


//
//	isPowerOfTwo
//

bool isPowerOfTwo(int n) {
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
	// remove previous image (if required)
	clear();

	// do we have a URL?
	if (OtText::startsWith(address, "http:") || OtText::startsWith(address, "https:")) {
		OtUrl url(address);
		url.download();
		load(url.getDownloadedData(), url.getDownloadedSize());

	} else {
		// load image from file
		image = IMG_Load(address.c_str());

		if (!image) {
			OtLogFatal("Error in IMG_Load: {}", SDL_GetError());
		}
	}

	// validate sides are power of 2 (if required)
	if (powerof2 && !(isPowerOfTwo(image->w))) {
		clear();
		OtLogError("Image width {} is not a power of 2", image->w);
	}

	if (powerof2 && !(isPowerOfTwo(image->h))) {
		clear();
		OtLogError("Image height {} is not a power of 2", image->h);
	}

	// validate squareness (if required)
	if (square && image->w != image->h) {
		clear();
		OtLogError("Image must be square not {} by {}", image->w, image->h);
	}
}


//
//	OtImage::load
//

void OtImage::load(int width, int height, int format, void* pixels) {
	// remove previous image (if required)
	clear();
	image = SDL_CreateSurfaceFrom(width, height, static_cast<SDL_PixelFormat>(format), pixels, SDL_BYTESPERPIXEL(format) * width);

	if (!image) {
		OtLogFatal("Error in IMG_Load_IO: {}", SDL_GetError());
	}
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

	image = IMG_Load_IO(io, true);

	if (!image) {
		OtLogFatal("Error in IMG_Load_IO: {}", SDL_GetError());
	}
}


//
//	OtImage::saveToPNG
//

void OtImage::saveToPNG(const std::string& path) {
	// sanity check
	OtAssert(isValid());

	// write image to file
	if (!IMG_SavePNG(image, path.c_str())){
		OtLogFatal("Error in IMG_SavePNG: {}", SDL_GetError());
	}
}


//
//	OtImage::getPixelRgba
//

glm::vec4 OtImage::getPixelRgba(int x, int y) {
	// sanity check
	OtAssert(isValid());

	x = std::clamp(x, 0, image->w - 1);
	y = std::clamp(y, 0, image->h - 1);

	if (image->format == SDL_PIXELFORMAT_RGBA8888) {
		auto value = &((uint8_t*) image->pixels)[y * image->w + x];
		return glm::vec4(value[0] / 255.0f, value[1] / 255.0f, value[2] / 255.0f, value[3] / 255.0f);

	} else if (image->format == SDL_PIXELFORMAT_RGBA128_FLOAT) {
		auto value = &((float*) image->pixels)[y * image->w + x];
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

	x *= image->w - 1;
	y *= image->h - 1;

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

	x *= image->w - 1;
	y *= image->h - 1;

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
