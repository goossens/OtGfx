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
#include "OtBlurComp.h"


//
//	OtBlur
//

class OtBlur : public OtFilter {
public:
	// set properties
	inline void setDirection(const glm::vec2& value) { direction = value; }
	inline void setBrightness(float value) { brightness = value; }
	inline void setTransparency(float value) { transparency = value; }

	// configure the compute pass
	void configurePass(OtComputePass& pass) override {
		// initialize pipeline (if required)
		if (!pipeline.isValid()) {
			pipeline.setShader(OtBlurComp, sizeof(OtBlurComp));
		}

		// set uniforms
		struct Uniforms {
			glm::vec4 direction;
			float brightness;
			float transparency;
		} uniforms {
			glm::vec4{direction, 0.0f, 0.0f},
			brightness,
			transparency
		};

		pass.addUniforms(&uniforms, sizeof(uniforms));
	}

private:
	// properties
	glm::vec2 direction{1.0f};
	float brightness = 2.0f;
	float transparency = 1.0f;
};
