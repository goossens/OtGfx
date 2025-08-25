//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtColorWheelComp.h"
#include "OtComputePipeline.h"
#include "OtGenerator.h"


//
//	OtColorWheel
//

class OtColorWheel : public OtGenerator {
public:
	// clear GPU resources
	inline void clear() { pipeline.clear(); }

	// execute generator
	inline void render(OtTexture& texture) override {
		run(pipeline, texture);
	}

private:
	// shader resources
	OtComputePipeline pipeline{OtColorWheelComp, sizeof(OtColorWheelComp)};
};
