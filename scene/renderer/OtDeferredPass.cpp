//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"

#include "OtRenderPass.h"
#include "OtRenderPipeline.h"

#include "OtDeferredPass.h"
#include "OtVertex.h"

#include "OtDeferredVert.h"
#include "OtDeferredAnimatedVert.h"
#include "OtDeferredInstancingVert.h"
#include "OtDeferredPbrFrag.h"
#include "OtDeferredLightingVert.h"
#include "OtDeferredLightingFrag.h"


//
//	OtDeferredPass::render
//

void OtDeferredPass::render(OtSceneRendererContext& ctx) {
	// initialize resources (if required)
	if (!resourcesInitialized) {
		initializeResources();
		resourcesInitialized = true;
	}

	// run the geometry and lighting passes
	renderGeometry(ctx);
	renderDirectionalLight(ctx);

	if (ctx.hasPointLighting) {
		renderPointLights(ctx);
	}
}


//
//	OtDeferredPass::renderGeometry
//

void OtDeferredPass::renderGeometry(OtSceneRendererContext& ctx) {
	// setup pass
	OtRenderPass pass;
	pass.setClearColor(true, glm::vec4(0.0f));
	pass.setClearDepth(true);
	pass.start(gbuffer);
	ctx.pass = &pass;

	// submit common uniforms
	ctx.setCameraUniforms(1);

	// render all entities
	renderEntities(ctx);

	// we're done
	pass.end();
}


//
//	OtDeferredPass::renderOpaqueGeometry
//

void OtDeferredPass::renderOpaqueGeometry(OtSceneRendererContext& ctx, OtGeometryRenderData& grd) {
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
//	OtDeferredPass::renderOpaqueModel
//

void OtDeferredPass::renderOpaqueModel(OtSceneRendererContext& ctx, OtModelRenderData& mrd) {
	renderModelHelper(
		ctx,
		mrd,
		cullingPipeline,
		animatedPipeline);
}


//
//	OtDeferredPass::renderDirectionalLight
//

void OtDeferredPass::renderDirectionalLight(OtSceneRendererContext& ctx) {
	// setup pass
	OtRenderPass pass;
	pass.start(framebuffer);
	pass.bindPipeline(directionalLightPipeline);
	ctx.pass = &pass;

	// set uniforms
	struct Uniforms {
		glm::mat4 viewUniform;
		glm::mat4 invViewProjUniform;
	} uniforms {
		ctx.camera.viewMatrix,
		glm::inverse(ctx.camera.viewProjectionMatrix)
	};

	pass.setFragmentUniforms(0, &uniforms, sizeof(Uniforms));
	ctx.setLightingUniforms(1, 5);
	ctx.setShadowUniforms(2, 8);

	// bind samplers
	pass.bindFragmentSampler(0, ctx.lightingAlbedoSampler, gbuffer.getAlbedoTexture());
	pass.bindFragmentSampler(1, ctx.lightingNormalSampler, gbuffer.getNormalTexture());
	pass.bindFragmentSampler(2, ctx.lightingPbrSampler, gbuffer.getPbrTexture());
	pass.bindFragmentSampler(3, ctx.lightingEmissiveSampler, gbuffer.getEmissiveTexture());
	pass.bindFragmentSampler(4, ctx.lightingDepthSampler, gbuffer.getDepthTexture());

	pass.render(3);
	pass.end();
}


//
//	OtDeferredPass::renderPointLights
//

void OtDeferredPass::renderPointLights(OtSceneRendererContext& ctx) {
	// setup pass
	OtRenderPass pass;
	pass.start(framebuffer);
	ctx.pass = &pass;
	pass.end();
}


//
//	OtDeferredPass::initializeResources
//

void OtDeferredPass::initializeResources() {
	// configure rendering pipelines
	cullingPipeline.setShaders(OtDeferredVert, sizeof(OtDeferredVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	cullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	cullingPipeline.setVertexDescription(OtVertex::getDescription());
	cullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);
	cullingPipeline.setCulling(OtRenderPipeline::Culling::cw);

	noCullingPipeline.setShaders(OtDeferredVert, sizeof(OtDeferredVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	noCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	noCullingPipeline.setVertexDescription(OtVertex::getDescription());
	noCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);

	linesPipeline.setShaders(OtDeferredVert, sizeof(OtDeferredVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	linesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	linesPipeline.setVertexDescription(OtVertex::getDescription());
	linesPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);
	linesPipeline.setFill(false);

	instancedCullingPipeline.setShaders(OtDeferredInstancingVert, sizeof(OtDeferredInstancingVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	instancedCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	instancedCullingPipeline.setVertexDescription(OtVertex::getDescription());
	instancedCullingPipeline.setInstanceDescription(OtVertexMatrix::getDescription());
	instancedCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);
	instancedCullingPipeline.setCulling(OtRenderPipeline::Culling::cw);

	instancedNoCullingPipeline.setShaders(OtDeferredInstancingVert, sizeof(OtDeferredInstancingVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	instancedNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	instancedNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	instancedNoCullingPipeline.setInstanceDescription(OtVertexMatrix::getDescription());
	instancedNoCullingPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);

	instancedLinesPipeline.setShaders(OtDeferredInstancingVert, sizeof(OtDeferredInstancingVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	instancedLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	instancedLinesPipeline.setVertexDescription(OtVertex::getDescription());
	instancedLinesPipeline.setInstanceDescription(OtVertexMatrix::getDescription());
	instancedLinesPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);
	instancedLinesPipeline.setFill(false);

	animatedPipeline.setShaders(OtDeferredAnimatedVert, sizeof(OtDeferredAnimatedVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	animatedPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	animatedPipeline.setVertexDescription(OtVertex::getDescription());
	animatedPipeline.setAnimatedDescription(OtVertexBones::getDescription());
	animatedPipeline.setDepthTest(OtRenderPipeline::CompareOperation::less);
	animatedPipeline.setCulling(OtRenderPipeline::Culling::cw);

	directionalLightPipeline.setShaders(OtDeferredLightingVert, sizeof(OtDeferredLightingVert), OtDeferredLightingFrag, sizeof(OtDeferredLightingFrag));
	directionalLightPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16d32);
	directionalLightPipeline.setDepthTest(OtRenderPipeline::CompareOperation::always);
}
