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

#include "SDL3/SDL.h"
#include "SDL3_shadercross/SDL_shadercross.h"

#include "OtGpu.h"
#include "OtLog.h"


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
			.shader_stage = static_cast<SDL_ShaderCross_ShaderStage>(stage)
		};

		auto graphicsShader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(OtGpu::instance().device, &info, metadata, 0);

		if (graphicsShader == nullptr) {
			OtLogFatal("Error in SDL_ShaderCross_CompileGraphicsShaderFromSPIRV: {}", SDL_GetError());
		}

		assign(graphicsShader);

		// cleanup
		SDL_free(metadata);
	}

	// clear the object
	inline void clear() { shader = nullptr; }

	// see if pipeline is valid
	inline bool isValid() { return shader != nullptr; }

private:
	// the GPU resource
	std::shared_ptr<SDL_GPUShader> shader;

	// memory manage SDL resource
	inline void assign(SDL_GPUShader* newShader) {
		shader = std::shared_ptr<SDL_GPUShader>(
			newShader,
			[](SDL_GPUShader* oldShader) {
				SDL_ReleaseGPUShader(OtGpu::instance().device, oldShader);
			});
	}

	// get the raw shader object
	friend class OtRenderPipeline;
	inline SDL_GPUShader* getShader() { return shader.get(); }
};
