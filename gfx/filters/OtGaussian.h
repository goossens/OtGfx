//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "glm/glm.hpp"

#include "OtFilter.h"
#include "OtGaussianComp.h"


//
//	OtGaussian
//

class OtGaussian : public OtFilter {
public:
	// constructor
	OtGaussian() {
		// switch to linear sampling for input texture
		sampler.setFilter(OtSampler::Filter::linear);
	}

	// set properties
	inline void setRadius(float value) { radius = value; }
	inline void setDirection(const glm::vec2& value) { direction = value; }

	// configure the compute pass
	void configurePass(OtComputePass& pass) override {
		// initialize pipeline (if required)
		if (!pipeline.isValid()) {
			pipeline.setShader(OtGaussianComp, sizeof(OtGaussianComp));
		}

		// set uniforms
		struct Uniforms {
			glm::vec2 pixelSize;
			glm::vec2 direction;
		} uniforms {
			sourcePixelSize,
			direction * radius
		};

		pass.addUniforms(&uniforms, sizeof(uniforms));
	}

private:
	// properties
	float radius = 1.0f;
	glm::vec2 direction{1.0f};
};
