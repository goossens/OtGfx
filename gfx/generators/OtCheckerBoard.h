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

#include "OtCheckerBoardComp.h"
#include "OtColor.h"
#include "OtComputePipeline.h"
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

	// clear GPU resources
	inline void clear() { pipeline.clear(); }

	// let generator render to texture
	inline void render(OtTexture& texture) override {
		struct Uniforms {
			glm::vec4 blackColor;
			glm::vec4 whiteColor;
			int repeat;
		} uniforms{blackColor, whiteColor, repeat};

		run(pipeline, texture, &uniforms, sizeof(uniforms));
	}

private:
	// properties
	OtColor blackColor{0.0f, 0.0f, 0.0f};
	OtColor whiteColor{1.0f, 1.0f, 1.0f};
	int repeat = 10;

	// shader resources
	OtComputePipeline pipeline{OtCheckerBoardComp, sizeof(OtCheckerBoardComp)};
};
