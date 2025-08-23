//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "SDL3_shadercross/SDL_shadercross.h"

#include "OtLog.h"

#include "OtGraphicsShader.h"


//
//	OtGraphicsShader::OtGraphicsShader
//

OtGraphicsShader::OtGraphicsShader(const uint32_t* code, size_t size, Stage stage) {
	// figure out shader metadata
	SDL_ShaderCross_GraphicsShaderMetadata* metadata = SDL_ShaderCross_ReflectGraphicsSPIRV((Uint8*) code, size, 0);

	if (metadata == nullptr) {
		OtLogFatal("Error in SDL_ShaderCross_ReflectGraphicsSPIRV: {}", SDL_GetError());
	}

	// cross compile to the appropriate shader format and create a shader object
	SDL_ShaderCross_SPIRV_Info info{};
	info.bytecode = (Uint8*) code;
	info.bytecode_size = size;
	info.entrypoint = "main";
	info.shader_stage = static_cast<SDL_ShaderCross_ShaderStage>(stage);
	assign(SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(OtGpu::instance().device, &info, metadata, 0));

	if (shader == nullptr) {
		OtLogFatal("Error in SDL_ShaderCross_CompileGraphicsShaderFromSPIRV: {}", SDL_GetError());
	}

	// cleanup
	SDL_free(metadata);
}
