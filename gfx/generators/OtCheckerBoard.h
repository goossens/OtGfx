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

#include "glm/glm.hpp"

#include "OtCheckerBoardComp.h"
#include "OtColor.h"
#include "OtGenerator.h"


//
//	OtCheckerBoard
//

class OtCheckerBoard : public OtGenerator {
public:
	// set the properties
	inline void setRepeat(float r) { repeat = r; }
	inline void setBlackColor(OtColor color) { blackColor = color; }
	inline void setWhiteColor(OtColor color) { whiteColor = color; }

	// configure the compute pass
	void configurePass(OtComputePass& pass) override {
		// initialize pipeline (if required)
		if (!pipeline.isValid()) {
			pipeline.initialize(OtCheckerBoardComp, sizeof(OtCheckerBoardComp));
		}

		// set uniforms
		struct Uniforms {
			glm::vec4 blackColor;
			glm::vec4 whiteColor;
			int32_t repeat;
		} uniforms{
			blackColor,
			whiteColor,
			static_cast<int32_t>(repeat)};

		pass.addUniforms(&uniforms, sizeof(uniforms));
	}

private:
	// properties
	OtColor blackColor{0.0f, 0.0f, 0.0f};
	OtColor whiteColor{1.0f, 1.0f, 1.0f};
	int repeat = 10;
};
