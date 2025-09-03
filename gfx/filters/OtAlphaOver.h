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
#include "OtSampler.h"
#include "OtTexture.h"


//
//	OtAlphaOver
//

class OtAlphaOver : public OtFilter {
public:
	// set properties
	inline void setOverlay(OtTexture& overlay) { overlayTexture = overlay; }

	// clear GPU resources
	inline void clear() override {
		overlayTexture.clear();
		overlaySampler.clear();
		OtFilter::clear();
	}

	// configure the compute pass
	void configurePass(OtComputePass& pass) override {
		// initialize pipeline (if required)
		if (!pipeline.isValid()) {
			pipeline.setShader(OtAlphaOverComp, sizeof(OtAlphaOverComp));
		}

		pass.addInputSampler(overlaySampler, overlayTexture);
	}

private:
	// properties
	OtTexture overlayTexture;
	OtSampler overlaySampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
};
