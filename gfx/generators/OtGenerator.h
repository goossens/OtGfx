//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <cmath>

#include "OtComputePass.h"
#include "OtComputePipeline.h"
#include "OtTexture.h"


//
//	OtGenerator
//

class OtGenerator {
public:
	// destructor
	virtual inline ~OtGenerator() {}

	// let generator render to texture
	virtual void render(OtTexture& texture) = 0;

protected:
	// run generator
	inline void run(OtComputePipeline& pipeline, OtTexture& texture, const void* uniforms=nullptr, size_t size=0) {
		OtComputePass pass;
		pass.addOutputTexture(texture);

		if (uniforms) {
			pass.addUniforms(uniforms, size);
		}

		pass.execute(
			pipeline,
			static_cast<size_t>(std::ceil(texture.getWidth() / 16.0)),
			static_cast<size_t>(std::ceil(texture.getHeight() / 16.0)),
			1);
	}
};
