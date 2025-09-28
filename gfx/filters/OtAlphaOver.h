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
#include "OtAlphaOverComp.h"


//
//	OtAlphaOver
//

class OtAlphaOver : public OtFilter {
public:
	// set properties
	inline void setOverlay(OtTexture overlay) { overlayTexture = overlay; }
	inline void setOverlayBrightness(float value) { brightness = value; }

	// configure the compute pass
	void configurePass(OtComputePass& pass) override {
		// initialize pipeline (if required)
		if (!pipeline.isValid()) {
			pipeline.setShader(OtAlphaOverComp, sizeof(OtAlphaOverComp));
		}

		// add overlay texture
		pass.addInputSampler(overlaySampler, overlayTexture);

		// set uniforms
		struct Uniforms {
			float brightness;
		} uniforms {
			brightness
		};

		pass.addUniforms(&uniforms, sizeof(uniforms));
	}

private:
	// properties
	OtTexture overlayTexture;
	OtSampler overlaySampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	float brightness = 1.0f;
};
