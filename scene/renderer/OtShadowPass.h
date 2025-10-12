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
//	OtShadowPass
//

class OtShadowPass : public OtSceneRenderEntitiesPass {
public:
	// constructor
	OtShadowPass();

	// render the pass
	void render(OtSceneRendererContext& ctx);

private:
	// properties
	OtRenderPipeline opaqueCullingPipeline;
	OtRenderPipeline opaqueNoCullingPipeline;
	OtRenderPipeline opaqueLinesPipeline;

	OtRenderPipeline instancedOpaqueCullingPipeline;
	OtRenderPipeline instancedOpaqueNoCullingPipeline;
	OtRenderPipeline instancedOpaqueLinesPipeline;

	OtRenderPipeline animatedOpaquePipeline;

	OtRenderPipeline terrainPipeline;
	OtRenderPipeline grassPipeline;

	// support functions
	bool isRenderingOpaque() override { return true; }
	bool isRenderingTransparent() override { return true; }

	void renderOpaqueGeometry(OtSceneRendererContext& ctx, OtEntity entity, OtGeometryComponent& component) override;
};
