//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <cmath>
#include <cstring>

#include "nlohmann/json.hpp"
#include "SDL3_image/SDL_image.h"

#include "OtLog.h"
#include "OtPath.h"
#include "OtText.h"

#include "OtAsset.h"
#include "OtComputePass.h"
#include "OtCubeMap.h"
#include "OtSampler.h"
#include "OtTexture.h"
#include "OtVertex.h"

#include "OtHdrReprojectComp.h"


//
//	OtCubeMap::create
//

void OtCubeMap::create(int s, bool m) {
	size = s;
	mip = m;

	// create new cubemap
	SDL_GPUTextureCreateInfo info{
		.type = SDL_GPU_TEXTURETYPE_CUBE,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = static_cast<Uint32>(size),
		.height = static_cast<Uint32>(size),
		.layer_count_or_depth = 6,
		.num_levels = static_cast<Uint32>(mip ? getMipLevels() : 1),
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
		.props = 0
	};

	auto sdlCubemap = SDL_CreateGPUTexture(OtGpu::instance().device, &info);

	if (!sdlCubemap) {
		OtLogFatal("Error in SDL_CreateGPUTexture: {}", SDL_GetError());
	}

	assign(sdlCubemap);
	incrementVersion();
}


//
//	OtCubeMap::load
//

void OtCubeMap::load(const std::string& path, bool async) {
	auto ext = OtPath::getExtension(path);

	if (ext == ".json") {
		loadJSON(path, async);

	} else {
	 	loadHdrImage(path, async);
	}
}


//
//	OtCubeMap::loadJSON
//

void OtCubeMap::loadJSON(const std::string& path, bool async) {
	// parse json
	std::string text;
	OtText::load(path, text);

	auto basedir = OtPath::getParent(path);
	auto data = nlohmann::json::parse(text);
	size = data.value("size", 0);

	if (!size) {
		OtLogError("Invalid size in CubeMap specification [{}]", path);
	}

	bytesPerImage = static_cast<size_t>(size * size * 4);
	imageData = std::make_unique<std::byte[]>(bytesPerImage * 6);

	std::array<std::string, 6> paths = {
		OtAssetDeserialize(&data, "posx", &basedir),
		OtAssetDeserialize(&data, "negx", &basedir),
		OtAssetDeserialize(&data, "posy", &basedir),
		OtAssetDeserialize(&data, "negy", &basedir),
		OtAssetDeserialize(&data, "posz", &basedir),
		OtAssetDeserialize(&data, "negz", &basedir)
	};

	// load images
	for (size_t i = 0; i < 6; i++) {
		// sanity check
		if (paths[i].empty()) {
			OtLogError("Incomplete CubeMap specification [{}]", path);
		}

		// load image
		SDL_Surface *surface = IMG_Load(paths[i].c_str());

		if (!surface) {
			OtLogFatal("Error while loading [{}]: {}", paths[i], SDL_GetError());

		} else if (surface->w != size) {
			OtLogFatal("Image [{}] has incorrect width [{}]. Should be [{}]", paths[i], surface->w, size);

		} else if (surface->h != size) {
			OtLogFatal("Image [{}] has incorrect height [{}]. Should be [{}]", paths[i], surface->h, size);
		}

		if (surface->format != SDL_PIXELFORMAT_RGBA32) {
			SDL_Surface *next = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
			SDL_DestroySurface(surface);
			surface = next;
		}

		// copy pixels
		std::memcpy(imageData.get() + bytesPerImage * i, surface->pixels, bytesPerImage);
		SDL_DestroySurface(surface);
	}

	if (async) {
		// schedule a task to upload images to texture
		// we can't do that here as loading is done in a separate thread
		// the callback below will be called in the main thread
		asyncHandle = new uv_async_t;
		asyncHandle->data = this;

		auto status = uv_async_init(uv_default_loop(), asyncHandle, [](uv_async_t* handle){
			auto cubemap = (OtCubeMap*) handle->data;
			cubemap->createCubemapFromSides();

			// cleanup
			uv_close((uv_handle_t*) cubemap->asyncHandle, [](uv_handle_t* handle) {
				auto cubemap = (OtCubeMap*) handle->data;
				delete (uv_fs_event_t*) handle;
				cubemap->asyncHandle = nullptr;
			});
		});

		UV_CHECK_ERROR("uv_async_init", status);

		status = uv_async_send(asyncHandle);
		UV_CHECK_ERROR("uv_async_send", status);

	} else {
		createCubemapFromSides();
	}
}


//
//	OtCubeMap::loadHdrImage
//

void OtCubeMap::loadHdrImage(const std::string& path, bool async) {
	// load image
	asyncImage = std::make_unique<OtImage>(path);
	size = 1024;

	if (async) {
		// schedule a task to upload and convert image to texture
		// we can't do that here as loading is done in a separate thread
		// the callback below will be called in the main thread
		asyncHandle = new uv_async_t;
		asyncHandle->data = this;

		auto status = uv_async_init(uv_default_loop(), asyncHandle, [](uv_async_t* handle){
			auto cubemap = (OtCubeMap*) handle->data;
			cubemap->createCubemapFromHDR();

			// cleanup
			uv_close((uv_handle_t*) cubemap->asyncHandle, [](uv_handle_t* handle) {
				auto cubemap = (OtCubeMap*) handle->data;
				delete (uv_fs_event_t*) handle;
				cubemap->asyncHandle = nullptr;
				cubemap->asyncImage = nullptr;
			});
		});

		UV_CHECK_ERROR("uv_async_init", status);

		status = uv_async_send(asyncHandle);
		UV_CHECK_ERROR("uv_async_send", status);

	} else {
		createCubemapFromHDR();
	}
}


//
//	OtCubeMap::createCubemapFromSides
//

void OtCubeMap::createCubemapFromSides() {
	// create new cubemap
	SDL_GPUTextureCreateInfo info{
		.type = SDL_GPU_TEXTURETYPE_CUBE,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = static_cast<Uint32>(size),
		.height = static_cast<Uint32>(size),
		.layer_count_or_depth = 6,
		.num_levels = static_cast<Uint32>(mip ? getMipLevels() : 1),
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
		.props = 0
	};

	auto sdlCubemap = SDL_CreateGPUTexture(OtGpu::instance().device, &info);

	if (!sdlCubemap) {
		OtLogFatal("Error in SDL_CreateGPUTexture: {}", SDL_GetError());
	}

	assign(sdlCubemap);

	// create a transfer buffer
	SDL_GPUTransferBufferCreateInfo bufferInfo{
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = static_cast<Uint32>(bytesPerImage * 6)
	};

	auto& gpu = OtGpu::instance();
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu.device, &bufferInfo);

	if (!transferBuffer) {
		OtLogFatal("Error in SDL_CreateGPUTransferBuffer: {}", SDL_GetError());
	}

	// put images in transfer buffer
	void* data = SDL_MapGPUTransferBuffer(gpu.device, transferBuffer, false);
	std::memcpy(data, imageData.get(), bytesPerImage * 6);
	SDL_UnmapGPUTransferBuffer(gpu.device, transferBuffer);

	// upload images to GPU
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(gpu.copyCommandBuffer);

	for (size_t i = 0; i < 6; i++) {
		SDL_GPUTextureTransferInfo transferInfo{
			.transfer_buffer = transferBuffer,
			.offset = static_cast<Uint32>(bytesPerImage * i)
		};

		SDL_GPUTextureRegion region{
			.texture = cubemap.get(),
			.layer = static_cast<Uint32>(i),
			.x = 0,
			.y = 0,
			.z = 0,
			.w = static_cast<Uint32>(size),
			.h = static_cast<Uint32>(size),
			.d = 1
		};

		SDL_UploadToGPUTexture(copyPass, &transferInfo, &region, false);
	}

	SDL_EndGPUCopyPass(copyPass);
	SDL_ReleaseGPUTransferBuffer(gpu.device, transferBuffer);

	// housekeeping
	incrementVersion();
	imageData = nullptr;
}


//
//	OtCubeMap::createCubemapFromHDR
//

void OtCubeMap::createCubemapFromHDR() {
	// upload image to GPU texture
	OtTexture inputTexture;
	inputTexture.load(*asyncImage);

	// create a cubemap texture
	SDL_GPUTextureCreateInfo info {
		.type = SDL_GPU_TEXTURETYPE_CUBE,
		.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
		.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = static_cast<Uint32>(size),
		.height = static_cast<Uint32>(size),
		.layer_count_or_depth = 6,
		.num_levels = static_cast<Uint32>(mip ? getMipLevels() : 1),
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
		.props = 0
	};

	auto sdlCubemap = SDL_CreateGPUTexture(OtGpu::instance().device, &info);

	if (!sdlCubemap) {
		OtLogFatal("Error in SDL_CreateGPUTexture: {}", SDL_GetError());
	}

	assign(sdlCubemap);

	// start a compute pass and setup the input and output textures
	OtSampler sampler{OtSampler::Filter::linear, OtSampler::Addressing::clamp};

	OtComputePipeline pipeline;
	pipeline.setShader(OtHdrReprojectComp, sizeof(OtHdrReprojectComp));

	OtComputePass pass;
	pass.addInputSampler(sampler, inputTexture);
	pass.addOutputCubeMap(*this);

	pass.execute(
		pipeline,
		static_cast<size_t>(std::ceil(size / 16.0)),
		static_cast<size_t>(std::ceil(size / 16.0)),
		6);
}
