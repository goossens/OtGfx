//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtGeometryComponent.h"
#include "OtGrassComponent.h"
#include "OtHighlightPass.h"
#include "OtModelComponent.h"
#include "OtTerrainComponent.h"

#include "OtRenderPass.h"

#include "OtSimpleVert.h"
#include "OtSimpleAnimatedVert.h"
#include "OtSimpleInstancingVert.h"

#include "OtSelectOpaqueFrag.h"
#include "OtSelectTransparentFrag.h"

#include "OtOutlineVert.h"
#include "OtOutlineFrag.h"


//
//	OtHighlightPass::render
//

void OtHighlightPass::render(OtSceneRendererContext& ctx, OtTexture* texture, OtEntity entity) {
	// see if we have a "highlightable" entity
	if (isHighlightable(ctx.scene, entity)) {
		// run both passes
		renderSelectedPass(ctx, entity);
		renderHighlightPass(ctx, texture);
	}
}


//
//	OtHighlightPass::renderSelectedPass
//

void OtHighlightPass::renderSelectedPass(OtSceneRendererContext& ctx, OtEntity entity) {
	// update framebuffer size
	selectedBuffer.update(ctx.camera.width, ctx.camera.height);

	// setup pass to render selected entities as opaque blobs
	OtRenderPass pass;
	ctx.pass = &pass;
	pass.setClearColor(true);
	pass.setClearDepth(true);
	pass.start(selectedBuffer);
	renderHighlight(ctx, entity);
	pass.end();
}


//
//	OtHighlightPass::renderHighlightPass
//

void OtHighlightPass::renderHighlightPass([[maybe_unused]] OtSceneRendererContext& ctx, [[maybe_unused]] OtTexture* texture) {
		// configure pass
		OtRenderPass pass;
		pass.start(*texture);
		pass.bindPipeline(outlinePipeline);
		pass.bindFragmentSampler(0, sampler, selectedBuffer.getColorTexture());

		// set uniforms
		struct Uniforms {
			glm::vec2 texelSize;
		} uniforms {
			glm::vec2(1.0f / static_cast<float>(ctx.camera.width), 1.0f / static_cast<float>(ctx.camera.height))
		};

		pass.setFragmentUniforms(0, &uniforms, sizeof(uniforms));
		pass.render(3);
		pass.end();
}


//
//	OtHighlightPass::renderHighlight
//

void OtHighlightPass::renderHighlight(OtSceneRendererContext& ctx, OtEntity entity) {
	// render entity and its children
	renderEntity(ctx, entity);
	OtEntity child = ctx.scene->getFirstChild(entity);

	while (ctx.scene->isValidEntity(child)) {
		renderHighlight(ctx, child);
		child = ctx.scene->getNextSibling(child);
	}
}


//
//	OtHighlightPass::isHighlightable
//

bool OtHighlightPass::isHighlightable(OtScene* scene, OtEntity entity) {
	// see if specified entity or one of the children is "highlightable"
	bool highlightable = false;

	if (scene->isValidEntity(entity)) {
		highlightable |= scene->hasComponent<OtGeometryComponent>(entity);
		highlightable |= scene->hasComponent<OtModelComponent>(entity);
		highlightable |= scene->hasComponent<OtTerrainComponent>(entity);
		highlightable |= scene->hasComponent<OtGrassComponent>(entity);

		OtEntity child = scene->getFirstChild(entity);

		while (scene->isValidEntity(child)) {
			highlightable |= isHighlightable(scene, child);
			child = scene->getNextSibling(child);
		}
	}

	return highlightable;
}


//
//	OtHighlightPass::renderOpaqueGeometry
//

void OtHighlightPass::renderOpaqueGeometry(OtSceneRendererContext& ctx, OtGeometryRenderData& grd) {
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
//	OtHighlightPass::renderOpaqueModel
//

void OtHighlightPass::renderOpaqueModel(OtSceneRendererContext& ctx, OtModelRenderData& mrd) {
	renderModelHelper(
		ctx,
		mrd,
		opaqueCullingPipeline,
		animatedPipeline);
}


//
//	OtHighlightPass::renderTransparentGeometry
//

void OtHighlightPass::renderTransparentGeometry(OtSceneRendererContext& ctx, OtGeometryRenderData& grd) {
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
//	OtHighlightPass::initializePipelines
//

void OtHighlightPass::initializePipelines() {
	opaqueCullingPipeline.setShaders(OtSimpleVert, sizeof(OtSimpleVert), OtSelectOpaqueFrag, sizeof(OtSelectOpaqueFrag));
	opaqueCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	opaqueCullingPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);

	opaqueNoCullingPipeline.setShaders(OtSimpleVert, sizeof(OtSimpleVert), OtSelectOpaqueFrag, sizeof(OtSelectOpaqueFrag));
	opaqueNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	opaqueNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueNoCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	opaqueNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	opaqueLinesPipeline.setShaders(OtSimpleVert, sizeof(OtSimpleVert), OtSelectOpaqueFrag, sizeof(OtSelectOpaqueFrag));
	opaqueLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	opaqueLinesPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueLinesPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	opaqueLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	opaqueLinesPipeline.setFill(false);

	opaqueInstancedCullingPipeline.setShaders(OtSimpleInstancingVert, sizeof(OtSimpleInstancingVert), OtSelectOpaqueFrag, sizeof(OtSelectOpaqueFrag));
	opaqueInstancedCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	opaqueInstancedCullingPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueInstancedCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);

	opaqueInstancedNoCullingPipeline.setShaders(OtSimpleInstancingVert, sizeof(OtSimpleInstancingVert), OtSelectOpaqueFrag, sizeof(OtSelectOpaqueFrag));
	opaqueInstancedNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	opaqueInstancedNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueInstancedNoCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	opaqueInstancedNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	opaqueInstancedLinesPipeline.setShaders(OtSimpleInstancingVert, sizeof(OtSimpleInstancingVert), OtSelectOpaqueFrag, sizeof(OtSelectOpaqueFrag));
	opaqueInstancedLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	opaqueInstancedLinesPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueInstancedLinesPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	opaqueInstancedLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	opaqueInstancedLinesPipeline.setFill(false);

	animatedPipeline.setShaders(OtSimpleAnimatedVert, sizeof(OtSimpleAnimatedVert), OtSelectOpaqueFrag, sizeof(OtSelectOpaqueFrag));
	animatedPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	animatedPipeline.setVertexDescription(OtVertex::getDescription());
	animatedPipeline.setAnimatedDescription(OtVertexBones::getDescription());
	animatedPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);

	transparentCullingPipeline.setShaders(OtSimpleVert, sizeof(OtSimpleVert), OtSelectTransparentFrag, sizeof(OtSelectTransparentFrag));
	transparentCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	transparentCullingPipeline.setVertexDescription(OtVertex::getDescription());
	transparentCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);

	transparentNoCullingPipeline.setShaders(OtSimpleVert, sizeof(OtSimpleVert), OtSelectTransparentFrag, sizeof(OtSelectTransparentFrag));
	transparentNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	transparentNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	transparentNoCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	transparentNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	transparentLinesPipeline.setShaders(OtSimpleVert, sizeof(OtSimpleVert), OtSelectTransparentFrag, sizeof(OtSelectTransparentFrag));
	transparentLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	transparentLinesPipeline.setVertexDescription(OtVertex::getDescription());
	transparentLinesPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	transparentLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	transparentLinesPipeline.setFill(false);

	transparentInstancedCullingPipeline.setShaders(OtSimpleInstancingVert, sizeof(OtSimpleInstancingVert), OtSelectTransparentFrag, sizeof(OtSelectTransparentFrag));
	transparentInstancedCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	transparentInstancedCullingPipeline.setVertexDescription(OtVertex::getDescription());
	transparentInstancedCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);

	transparentInstancedNoCullingPipeline.setShaders(OtSimpleInstancingVert, sizeof(OtSimpleInstancingVert), OtSelectTransparentFrag, sizeof(OtSelectTransparentFrag));
	transparentInstancedNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	transparentInstancedNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	transparentInstancedNoCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	transparentInstancedNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	transparentInstancedLinesPipeline.setShaders(OtSimpleInstancingVert, sizeof(OtSimpleInstancingVert), OtSelectTransparentFrag, sizeof(OtSelectTransparentFrag));
	transparentInstancedLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::r8);
	transparentInstancedLinesPipeline.setVertexDescription(OtVertex::getDescription());
	transparentInstancedLinesPipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	transparentInstancedLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	transparentInstancedLinesPipeline.setFill(false);

	outlinePipeline.setShaders(OtOutlineVert, sizeof(OtOutlineVert), OtOutlineFrag, sizeof(OtOutlineFrag));
	outlinePipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16);
	outlinePipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	outlinePipeline.setCulling(OtRenderPipeline::Culling::none);

	outlinePipeline.setBlend(
		OtRenderPipeline::BlendOperation::add,
		OtRenderPipeline::BlendFactor::srcAlpha,
		OtRenderPipeline::BlendFactor::oneMinusSrcAlpha
	);
}
