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
	virtual inline void clear() {
		pipeline.clear();
		sampler.clear();
	}

	// methods to be overridden by derived classes (if required)
	virtual void beforePass() {}
	virtual void configurePass([[maybe_unused]] OtComputePass& pass) {}

	// let filter transform texture to output
	void render(OtTexture& origin, OtTexture& destination) {
		// give subclasses the option to do things before we start the compute pass
		// e.g. prepare buffers
		beforePass();

		// start a compute pass and setup the input and output textures
		OtComputePass pass;
		pass.addInputSampler(sampler, origin);
		pass.addOutputTexture(destination);

		// ask derived class to configure the compute pass
		// e.g. create compute pipeline, add input samplers and/or set uniforms
		configurePass(pass);

		// execute the compute pass
		pass.execute(
			pipeline,
			static_cast<size_t>(std::ceil(destination.getWidth() / 16.0)),
			static_cast<size_t>(std::ceil(destination.getHeight() / 16.0)),
			1);
	}

protected:
	// the filter specific rendering pipeline (to be set by derived class)
	OtComputePipeline pipeline;

	// work variables
	OtSampler sampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
};
