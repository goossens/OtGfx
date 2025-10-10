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
#include <unordered_map>

#include "SDL3/SDL.h"
#include "SDL3_shadercross/SDL_shadercross.h"

#include "OtHash.h"
#include "OtLog.h"

#include "OtFrameworkAtExit.h"
#include "OtGpu.h"


//
//	OtRenderShader
//

class OtRenderShader {
public:
	// shader stages
	enum class Stage {
		vertex = SDL_SHADERCROSS_SHADERSTAGE_VERTEX,
		fragment = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT
	};

	// constructor
	inline OtRenderShader(const uint32_t* code, size_t size, Stage stage) {
		// initialize cache (if required)
		if (!cacheInitialized) {
			// we need to clear the cache when the framework exits
			// waiting the cache is destructed doesn't work
			// since the GPU context is no longer available
			OtFrameworkAtExit::add([]() {
				for (auto& [hash, shader]: shaders) {
					SDL_ReleaseGPUShader(OtGpu::instance().device, shader);
				}

				shaders.clear();
			});

			cacheInitialized = true;
		}

		// see if shader has already been compiled
		auto hash = OtHash::generate(code, size, stage);

		if (shaders.find(hash) == shaders.end()) {
			// shader is not it cache so we need to cross compile it

			// figure out shader metadata
			SDL_ShaderCross_GraphicsShaderMetadata* metadata = SDL_ShaderCross_ReflectGraphicsSPIRV((Uint8*) code, size, 0);

			if (metadata == nullptr) {
				OtLogFatal("Error in SDL_ShaderCross_ReflectGraphicsSPIRV: {}", SDL_GetError());
			}

			// cross compile to the appropriate shader format and create a shader object
			SDL_ShaderCross_SPIRV_Info info{
				.bytecode = (Uint8*) code,
				.bytecode_size = size,
				.entrypoint = "main",
				.shader_stage = static_cast<SDL_ShaderCross_ShaderStage>(stage),
				.props = 0
			};

			shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
				OtGpu::instance().device,
				&info,
				&metadata->resource_info,
				0);

			if (shader == nullptr) {
				OtLogFatal("Error in SDL_ShaderCross_CompileGraphicsShaderFromSPIRV: {}", SDL_GetError());
			}

			// store shader in cache
			shaders[hash] = shader;

			// cleanup
			SDL_free(metadata);

		} else {
			// shader was already compiled, get it from the cache
			shader = shaders[hash];
		}
	}

private:
	// the GPU resource
	SDL_GPUShader* shader = nullptr;

	// get the raw shader object
	friend class OtRenderPipeline;
	inline SDL_GPUShader* getShader() { return shader; }

	// shader cache
	static inline bool cacheInitialized = false;
	static inline std::unordered_map<size_t, SDL_GPUShader*> shaders;
};
