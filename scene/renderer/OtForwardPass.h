//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtRenderPipeline.h"

#include "OtSceneRendererContext.h"
#include "OtSceneRenderEntitiesPass.h"


//
//	OtForwardPass
//

class OtForwardPass : public OtSceneRenderEntitiesPass {
public:
	// constructor
	OtForwardPass(OtFrameBuffer& fb) : framebuffer(fb) {
		initializePipelines();
	}

	// render the pass
	void render(OtSceneRendererContext& ctx);

protected:
	bool isRenderingOpaque() override { return false; }
	bool isRenderingTransparent() override { return true; }

	void renderTransparentGeometry(OtSceneRendererContext& ctx, OtEntity entity, OtGeometryComponent& component) override;

private:
	// properties
	OtFrameBuffer& framebuffer;

	OtRenderPipeline cullingPipeline;
	OtRenderPipeline noCullingPipeline;
	OtRenderPipeline linesPipeline;

	OtRenderPipeline instancedCullingPipeline;
	OtRenderPipeline instancedNoCullingPipeline;
	OtRenderPipeline instancedLinesPipeline;

	OtRenderPipeline animatedPipeline;

	// support functions
	void initializePipelines();
};
