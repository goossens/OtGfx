//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtLog.h"

#include "OtRenderPass.h"
#include "OtRenderPipeline.h"
#include "OtSampler.h"
#include "OtTexture.h"

#include "OtCompositingFrag.h"
#include "OtCompositingVert.h"


//
//	OtCompositing
//

class OtCompositing {
public:
	// destructor
	virtual inline ~OtCompositing() {}

	// clear GPU resources
	virtual inline void clear() {
		pipeline.clear();
		sampler.clear();
	}

	// set properties
	inline void setBrightness(float value) { brightness = value; }

	// composite input on top of output
	void render(OtTexture& source, OtTexture& destination) {
		if (!source.canBeSampled()) {
			OtLogFatal("Input texture to compositing does not have [sampler] usage");
		}

		if (!destination.isColorTarget()) {
			OtLogFatal("Output texture to compositing is not a render target");
		}

		// create pipeline (if required)
		if (!pipeline.isValid()) {
			pipeline.setShaders(OtCompositingVert, sizeof(OtCompositingVert), OtCompositingFrag, sizeof(OtCompositingFrag));
			pipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba8);
			pipeline.setTargetChannels(OtRenderPipeline::TargetChannels::rgba);
			pipeline.setDepthTest(OtRenderPipeline::DepthTest::none);
			pipeline.setCulling(OtRenderPipeline::Culling::none);
			configurePipeline(pipeline);
		}

		// configure pass
		OtRenderPass pass;
		pass.start(destination);
		pass.bindPipeline(pipeline);
		pass.bindFragmentSampler(0, sampler, source);

		// set uniforms
		struct Uniforms {
			float brightness;
		} uniforms {
			brightness
		};

		pass.setFragmentUniforms(0, &uniforms, sizeof(uniforms));
		pass.render(3);
		pass.end();
	}

protected:
	// methods to be overridden by derived classes (if required)
	virtual void configurePipeline([[maybe_unused]] OtRenderPipeline& pipeline) {}
	virtual void configurePass([[maybe_unused]] OtRenderPass& pass) {}

	// the specific rendering pipeline (to be set by derived class)
	OtRenderPipeline pipeline;

	// work variables
	OtSampler sampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	float brightness = 1.0f;
};
