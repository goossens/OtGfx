//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtRenderPass.h"

#include "OtShadowPass.h"
#include "OtShadowAnimatedVert.h"
#include "OtShadowInstancingVert.h"
#include "OtShadowOpaqueFrag.h"
#include "OtShadowTransparentFrag.h"
#include "OtShadowVert.h"


//
//	OtShadowPass::render
//

void OtShadowPass::render(OtSceneRendererContext& ctx) {
	// save context part the we will temporarily overwrite
	auto camera = ctx.camera;
	auto cameraID = ctx.cameraID;
	auto renderingShadow = ctx.renderingShadow;

	// put context in shadow rendering mode
	ctx.renderingShadow = true;

	// render each cascade
	for (size_t i = 0; i < OtCascadedShadowMap::maxCascades; i++) {
		// setup pass to render entities as opaque blobs
		OtRenderPass pass;
		pass.setClearDepth(true);
		pass.start(ctx.csm.getFrameBuffer(i));
		ctx.pass = &pass;

		ctx.camera = ctx.csm.getCamera(i);
		ctx.cameraID = OtSceneRendererContext::getShadowCameraID(i);

		// render all entities
		ctx.setCameraUniforms(1);
		renderEntities(ctx);

		// we're done
		pass.end();
	}

	// restore old rendering context
	ctx.camera = camera;
	ctx.cameraID = cameraID;
	ctx.renderingShadow = renderingShadow;
}


//
//	OtShadowPass::renderOpaqueGeometry
//

void OtShadowPass::renderOpaqueGeometry(OtSceneRendererContext& ctx, OtGeometryRenderData& grd) {
	renderGeometryHelper(
		ctx,
		grd,
		opaqueCullingPipeline,
		opaqueNoCullingPipeline,
		opaqueLinesPipeline,
		opaqueInstancedCullingPipeline,
		opaqueInstancedNoCullingPipeline,
		opaqueInstancedLinesPipeline);
}


//
//	OtShadowPass::renderOpaqueModel
//

void OtShadowPass::renderOpaqueModel(OtSceneRendererContext& ctx, OtModelRenderData& mrd) {
	renderModelHelper(
		ctx,
		mrd,
		opaqueCullingPipeline,
		animatedPipeline);
}


//
//	OtShadowPass::renderTransparentGeometry
//

void OtShadowPass::renderTransparentGeometry(OtSceneRendererContext& ctx, OtGeometryRenderData& grd) {
	renderGeometryHelper(
		ctx,
		grd,
		transparentCullingPipeline,
		transparentNoCullingPipeline,
		transparentLinesPipeline,
		transparentInstancedCullingPipeline,
		transparentInstancedNoCullingPipeline,
		transparentInstancedLinesPipeline);
}


//
//	OtShadowPass::initializePipelines
//

void OtShadowPass::initializePipelines() {
	opaqueCullingPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	opaqueCullingPipeline.setVertexDescription(OtVertex::getDescription());

	opaqueNoCullingPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	opaqueNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	opaqueLinesPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	opaqueLinesPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	opaqueLinesPipeline.setFill(false);

	opaqueInstancedCullingPipeline.setShaders(OtShadowInstancingVert, sizeof(OtShadowInstancingVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueInstancedCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	opaqueInstancedCullingPipeline.setVertexDescription(OtVertex::getDescription());

	opaqueInstancedNoCullingPipeline.setShaders(OtShadowInstancingVert, sizeof(OtShadowInstancingVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueInstancedNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	opaqueInstancedNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueInstancedNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	opaqueInstancedLinesPipeline.setShaders(OtShadowInstancingVert, sizeof(OtShadowInstancingVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueInstancedLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	opaqueInstancedLinesPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueInstancedLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	opaqueInstancedLinesPipeline.setFill(false);

	animatedPipeline.setShaders(OtShadowAnimatedVert, sizeof(OtShadowAnimatedVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	animatedPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	animatedPipeline.setVertexDescription(OtVertex::getDescription());
	animatedPipeline.setAnimatedDescription(OtVertexBones::getDescription());

	transparentCullingPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowTransparentFrag, sizeof(OtShadowTransparentFrag));
	transparentCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	transparentCullingPipeline.setVertexDescription(OtVertex::getDescription());

	transparentNoCullingPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowTransparentFrag, sizeof(OtShadowTransparentFrag));
	transparentNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	transparentNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	transparentNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	transparentLinesPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowTransparentFrag, sizeof(OtShadowTransparentFrag));
	transparentLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	transparentLinesPipeline.setVertexDescription(OtVertex::getDescription());
	transparentLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	transparentLinesPipeline.setFill(false);

	transparentInstancedCullingPipeline.setShaders(OtShadowInstancingVert, sizeof(OtShadowInstancingVert), OtShadowTransparentFrag, sizeof(OtShadowTransparentFrag));
	transparentInstancedCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	transparentInstancedCullingPipeline.setVertexDescription(OtVertex::getDescription());

	transparentInstancedNoCullingPipeline.setShaders(OtShadowInstancingVert, sizeof(OtShadowInstancingVert), OtShadowTransparentFrag, sizeof(OtShadowTransparentFrag));
	transparentInstancedNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	transparentInstancedNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	transparentInstancedNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	transparentInstancedLinesPipeline.setShaders(OtShadowInstancingVert, sizeof(OtShadowInstancingVert), OtShadowTransparentFrag, sizeof(OtShadowTransparentFrag));
	transparentInstancedLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::d32);
	transparentInstancedLinesPipeline.setVertexDescription(OtVertex::getDescription());
	transparentInstancedLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	transparentInstancedLinesPipeline.setFill(false);
}
