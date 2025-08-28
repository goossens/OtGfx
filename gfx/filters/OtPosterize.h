//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtComputePipeline.h"
#include "OtFilter.h"
#include "OtPosterizeComp.h"


//
//	OtPosterize
//

class OtPosterize : public OtFilter {
public:
	// set properties
	inline void setLevels(int l) { levels = l; }

	// clear GPU resources
	inline void clear() {
		OtFilter::clear();
		pipeline.clear();
	}

	// let filter transform texture to output
	void render(OtTexture& origin, OtTexture& destination) override {
		struct Uniforms {
			int32_t levels;
		} uniforms {
			static_cast<int32_t>(levels)
		};

		run(pipeline, origin, destination, &uniforms, sizeof(uniforms));
	}

private:
	// properties
	int levels = 10;

	// shader resources
	OtComputePipeline pipeline{OtPosterizeComp, sizeof(OtPosterizeComp)};
};
