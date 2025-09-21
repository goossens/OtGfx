//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <cstring>

#include "OtLog.h"

#include "OtImage.h"
#include "OtTexture.h"
#include "OtGpu.h"


//
//	OtTexture::clear
//

void OtTexture::clear() {
	texture = nullptr;
	width = 1;
	height = 1;
	format = Format::none;
	usage = Usage::none;
	incrementVersion();
}


//
//	OtTexture::create
//

bool OtTexture::update(int w, int h, Format f, Usage u) {
	if (!texture || width != w || h != height || f != format || u != usage) {
		// remember settings
		width = w;
		height = h;
		format = f;
		usage = u;

		// create new texture
		SDL_GPUTextureCreateInfo info{
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPUTextureFormat(format),
			.usage = SDL_GPUTextureUsageFlags(usage),
			.width = static_cast<Uint32>(width),
			.height = static_cast<Uint32>(height),
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
			.props = 0
		};

		auto sdlTexture = SDL_CreateGPUTexture(OtGpu::instance().device, &info);

		if (!sdlTexture) {
			OtLogFatal("Error in SDL_CreateGPUTexture: {}", SDL_GetError());
		}

		assign(sdlTexture);
		return true;

	} else {
		return false;
	}
}


//
//	OtTexture::load
//

void OtTexture::load(OtImage& image) {
	// update the texture
	update(image.getWidth(), image.getHeight(), convertFromImageFormat(image.getFormat()), Usage::sampler);

	// create a transfer buffer
	SDL_GPUTransferBufferCreateInfo bufferInfo{
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = static_cast<Uint32>(width * height * getBpp())
	};

	auto& gpu = OtGpu::instance();
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu.device, &bufferInfo);

	if (!transferBuffer) {
		OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
	}

	// put image in transfer buffer
	void* data = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer, false);
	std::memcpy(data, image.getPixels(), bufferInfo.size);
	SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer);

	// upload image to GPU
	SDL_GPUTextureTransferInfo transferInfo{
		.transfer_buffer = transferBuffer,
		.offset = 0,
	};

	SDL_GPUTextureRegion region{
		.texture = texture.get(),
		.x = 0,
		.y = 0,
		.z = 0,
		.w = static_cast<Uint32>(width),
		.h = static_cast<Uint32>(height),
		.d = 1
	};

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);
	SDL_UploadToGPUTexture(copyPass, &transferInfo, &region, false);
	SDL_EndGPUCopyPass(copyPass);
	SDL_ReleaseGPUTransferBuffer(gpu.device, transferBuffer);
}


//
//	OtTexture::load
//

void OtTexture::load(const std::string& path, bool async) {
	if (async) {
		// get image and schedule an asynchronous load to the GPU
		asyncImage = std::make_shared<OtImage>(path);
		loadAsync();

	} else {
		// get the image
		OtImage image(path);
		load(image);
	}
}


//
//	OtTexture::load
//

void OtTexture::load(void* data, size_t size, bool async) {
	if (async) {
		// get image and schedule an asynchronous load to the GPU
		asyncImage = std::make_shared<OtImage>();
		asyncImage->load(data, size);
		loadAsync();

	} else {
		// get the image
		OtImage image;
		image.load(data, size);
		load(image);
	}
}


//
//	OtTexture::load
//

void OtTexture::load(int w, int h, Format f, void* pixels, bool async) {
	OtImage::Format fmt;

	if (f == Format::r8) {
		fmt = OtImage::Format::r8;

	} else if (f == Format::rgba8) {
		fmt = OtImage::Format::rgba8;

	} else if (f == Format::rgbaFloat32) {
		fmt = OtImage::Format::rgbaFloat32;

	} else {
		OtLogFatal("Unsupported pixel format");
	}

	if (async) {
		// get image and schedule an asynchronous load to the GPU
		asyncImage = std::make_shared<OtImage>();
		asyncImage->load(w, h, fmt, pixels);
		loadAsync();

	} else {
		// get the image
		OtImage image;
		image.load(w, h, fmt, pixels);
		load(image);
	}
}


//
//	OtTexture::update
//

void OtTexture::update(int x, int y, int w, int h, void* pixels) {
	// create a transfer buffer
	SDL_GPUTransferBufferCreateInfo bufferInfo{
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = static_cast<Uint32>(w * h * getBpp())
	};

	auto& gpu = OtGpu::instance();
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu.device, &bufferInfo);

	if (!transferBuffer) {
		OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
	}

	// put image in transfer buffer
	void* data = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer, false);
	std::memcpy(data, pixels, bufferInfo.size);
	SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer);

	// upload image to GPU
	SDL_GPUTextureTransferInfo transferInfo{
		.transfer_buffer = transferBuffer,
		.offset = 0
	};

	SDL_GPUTextureRegion region{
		.texture = texture.get(),
		.x = static_cast<Uint32>(x),
		.y = static_cast<Uint32>(y),
		.z = 0,
		.w = static_cast<Uint32>(w),
		.h = static_cast<Uint32>(h),
		.d = 1
	};

	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);
	SDL_UploadToGPUTexture(copyPass, &transferInfo, &region, false);
	SDL_EndGPUCopyPass(copyPass);
	SDL_ReleaseGPUTransferBuffer(gpu.device, transferBuffer);
}


//
//	OtTexture::loadAsync
//

void OtTexture::loadAsync() {
	// schedule a task to upload image to texture
	// we can't do that here as loading is done in a separate thread (where there is no GPU context)
	// the callback below will be called in the main thread (where we have a GPU context)
	asyncHandle = new uv_async_t;
	asyncHandle->data = this;

	auto status = uv_async_init(uv_default_loop(), asyncHandle, [](uv_async_t* handle){
		// load image to texture
		auto texture = (OtTexture*) handle->data;
		texture->load(*texture->asyncImage);
		texture->asyncImage = nullptr;

		// cleanup
		uv_close((uv_handle_t*) texture->asyncHandle, [](uv_handle_t* handle) {
			auto texture = (OtTexture*) handle->data;
			delete (uv_fs_event_t*) handle;
			texture->asyncHandle = nullptr;
		});
	});

	UV_CHECK_ERROR("uv_async_init", status);

	status = uv_async_send(asyncHandle);
	UV_CHECK_ERROR("uv_async_send", status);
}
