//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtFilter.h"
#include "OtBlitComp.h"


//
//	OtBlit
//

class OtBlit : public OtFilter {
public:
	// set properties
	inline void setBrightness(float value) { brightness = value; }
	inline void setTransparency(float value) { transparency = value; }

	// configure the compute pass
	void configurePass(OtComputePass& pass) override {
		// initialize pipeline (if required)
		if (!pipeline.isValid()) {
			pipeline.initialize(OtBlitComp, sizeof(OtBlitComp));
		}

		// set uniforms
		struct Uniforms {
			float brightness;
			float transparency;
		} uniforms {
			brightness,
			transparency
		};

		pass.addUniforms(&uniforms, sizeof(uniforms));
	}

private:
	// properties
	float brightness = 2.0f;
	float transparency = 1.0f;
};
