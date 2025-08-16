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

#include "SDL3_shadercross/SDL_shadercross.h"


//
//	OtGraphicsShader
//

class OtGraphicsShader {
public:
	// shader stages
	enum class Stage {
		vertex = SDL_SHADERCROSS_SHADERSTAGE_VERTEX,
		fragment = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT
	};

	// constructor/destructor
	OtGraphicsShader(SDL_GPUDevice* d, const uint32_t* code, size_t size, Stage stage);
	~OtGraphicsShader();

	// access information
	SDL_ShaderCross_GraphicsShaderMetadata* getMetadata() { return metadata;}
	SDL_GPUShader* getShader() { return shader;}

private:
	SDL_GPUDevice* device = nullptr;
	SDL_ShaderCross_GraphicsShaderMetadata* metadata = nullptr;
	SDL_GPUShader* shader = nullptr;
};
