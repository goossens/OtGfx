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
	OtGraphicsShader(const uint32_t* code, size_t size, Stage stage);

private:
	std::shared_ptr<SDL_GPUShader> shader;
};
