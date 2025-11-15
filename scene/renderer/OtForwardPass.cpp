//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "glm/glm.hpp"

#include "OtRenderPass.h"

#include "OtForwardPass.h"
#include "OtForwardVert.h"
#include "OtForwardPbrFrag.h"


//
//	OtForwardPass::render
//

void OtForwardPass::render(OtSceneRendererContext& ctx) {
	// initialize resources (if required)
	if (!resourcesInitialized) {
		initializeResources();
		resourcesInitialized = true;
	}

	// setup pass
	OtRenderPass pass;
	pass.start(framebuffer);
	ctx.pass = &pass;

	// submit common uniforms
	ctx.setCameraUniforms(1);
	ctx.setLightingUniforms(1, 5);
	ctx.setShadowUniforms(2, 8);

	// render all entities
	renderEntities(ctx);

	// we're done
	pass.end();
}


//
//	OtForwardPass::renderTransparentGeometry
//

void OtForwardPass::renderTransparentGeometry(OtSceneRendererContext& ctx, OtGeometryRenderData& grd) {
	renderGeometryHelper(
		ctx,
		grd,
		cullingPipeline,
		noCullingPipeline,
		linesPipeline,
		instancedCullingPipeline,
		instancedNoCullingPipeline,
		instancedLinesPipeline);
}


//
//	OtForwardPass::initializeResources
//

void OtForwardPass::initializeResources() {
	cullingPipeline.setShaders(OtForwardVert, sizeof(OtForwardVert), OtForwardPbrFrag, sizeof(OtForwardPbrFrag));
	cullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16d32);
	cullingPipeline.setVertexDescription(OtVertex::getDescription());
	cullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);
	cullingPipeline.setCulling(OtRenderPipeline::Culling::cw);

	cullingPipeline.setBlend(
		OtRenderPipeline::BlendOperation::add,
		OtRenderPipeline::BlendFactor::srcAlpha,
		OtRenderPipeline::BlendFactor::oneMinusSrcAlpha
	);

	noCullingPipeline.setShaders(OtForwardVert, sizeof(OtForwardVert), OtForwardPbrFrag, sizeof(OtForwardPbrFrag));
	noCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16d32);
	noCullingPipeline.setVertexDescription(OtVertex::getDescription());
	noCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);

	noCullingPipeline.setBlend(
		OtRenderPipeline::BlendOperation::add,
		OtRenderPipeline::BlendFactor::srcAlpha,
		OtRenderPipeline::BlendFactor::oneMinusSrcAlpha
	);

	linesPipeline.setShaders(OtForwardVert, sizeof(OtForwardVert), OtForwardPbrFrag, sizeof(OtForwardPbrFrag));
	linesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16d32);
	linesPipeline.setVertexDescription(OtVertex::getDescription());
	linesPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);
	linesPipeline.setFill(false);

	linesPipeline.setBlend(
		OtRenderPipeline::BlendOperation::add,
		OtRenderPipeline::BlendFactor::srcAlpha,
		OtRenderPipeline::BlendFactor::oneMinusSrcAlpha
	);
}
