//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <cmath>

#include "OtComputePass.h"
#include "OtComputePipeline.h"
#include "OtSampler.h"
#include "OtTexture.h"


//
//	OtFilter
//

class OtFilter {
public:
	// destructor
	virtual inline ~OtFilter() {}

	// clear GPU resources
	inline void clear() { sampler.clear(); }

	// let filter transform texture to output
	virtual void render(OtTexture& origin, OtTexture& destination) = 0;

protected:
	// run filter
	inline void run(OtComputePipeline& pipeline, OtTexture& input, OtTexture& output, const void* uniforms=nullptr, size_t size=0) {
		OtComputePass pass;
		pass.addInputSampler(input, sampler);
		pass.addOutputTexture(output);

		if (uniforms) {
			pass.addUniforms(uniforms, size);
		}

		pass.execute(
			pipeline,
			static_cast<size_t>(std::ceil(output.getWidth() / 16.0)),
			static_cast<size_t>(std::ceil(output.getHeight() / 16.0)),
			1);
	}

private:
	// work variables
	OtSampler sampler{OtSampler::nearestSampling | OtSampler::clampSampling};
};
