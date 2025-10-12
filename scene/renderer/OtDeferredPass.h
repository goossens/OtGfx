//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtFrameBuffer.h"
#include "OtGbuffer.h"
#include "OtSampler.h"
#include "OtVertex.h"
#include "OtVertexBuffer.h"

#include "OtSceneRenderEntitiesPass.h"


//
//	OtDeferredPass
//

class OtDeferredPass : public OtSceneRenderEntitiesPass {
public:
	// constructor
	OtDeferredPass(OtGbuffer& gb, OtFrameBuffer& fb) : gbuffer(gb), framebuffer(fb) {
		initializePipelines();
	}

	// render the pass
	void render(OtSceneRendererContext& ctx);

protected:
	bool isRenderingOpaque() override { return true; }
	bool isRenderingTransparent() override { return false; }

	void renderOpaqueGeometry(OtSceneRendererContext& ctx, OtEntity entity, OtGeometryComponent& component) override;

private:
	// properties
	OtGbuffer& gbuffer;
	OtFrameBuffer& framebuffer;

	OtRenderPipeline cullingPipeline;
	OtRenderPipeline noCullingPipeline;
	OtRenderPipeline linesPipeline;

	OtRenderPipeline instancedCullingPipeline;
	OtRenderPipeline instancedNoCullingPipeline;
	OtRenderPipeline instancedLinesPipeline;

	OtRenderPipeline animatedPipeline;
	OtRenderPipeline terrainPipeline;
	OtRenderPipeline grassPipeline;

	OtRenderPipeline directionalLightPipeline;
	OtRenderPipeline pointLightPipeline;

	OtSampler lightingAlbedoSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler lightingNormalSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler lightingPbrSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler lightingEmissiveSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler lightingDepthSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};

	// support functions
	void renderGeometry(OtSceneRendererContext& ctx);
	void renderDirectionalLight(OtSceneRendererContext& ctx);
	void renderPointLights(OtSceneRendererContext& ctx);
	void initializePipelines();
};
