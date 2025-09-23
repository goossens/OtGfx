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
	// configure the compute pass
	void configurePass([[maybe_unused]] OtComputePass& pass) override {
		// initialize pipeline (if required)
		if (!pipeline.isValid()) {
			pipeline.setShader(OtAlphaOverComp, sizeof(OtAlphaOverComp));
		}
	}
};
