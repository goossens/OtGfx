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
#include "OtInstances.h"
#include "OtVertex.h"

#include "OtDeferredVert.h"
#include "OtDeferredInstancingVert.h"
#include "OtDeferredPbrFrag.h"
#include "OtDeferredLightingVert.h"
#include "OtDeferredLightingFrag.h"


//
//	OtDeferredPass::render
//

void OtDeferredPass::render(OtSceneRendererContext & ctx) {
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
	pass.start(gbuffer);
	ctx.pass = &pass;

	// submit common uniforms
	setCameraUniforms(ctx, 1);

	// render all entities
	renderEntities(ctx);

	// we're done
	pass.end();
}


//
//	OtDeferredPass::renderOpaqueGeometry
//

void OtDeferredPass::renderOpaqueGeometry(OtSceneRendererContext& ctx, OtEntity entity, OtGeometryComponent& geometry) {
	// bind pipeline
	if (geometry.wireframe) {
		ctx.pass->bindPipeline(linesPipeline);

	} else if (geometry.cullBack) {
		ctx.pass->bindPipeline(cullingPipeline);

	} else {
		ctx.pass->bindPipeline(noCullingPipeline);
	}

	// set vertex uniforms
	struct Uniforms {
		glm::mat4 modelMatrix;
	} uniforms {
		ctx.scene->getGlobalTransform(entity)
	};

	ctx.pass->setVertexUniforms(0, &uniforms, sizeof(Uniforms));

	//set fragment uniforms
	setMaterialUniforms(ctx, 0, 0, entity);

	// render geometry
	ctx.pass->render(geometry.asset->getGeometry());
}


//
//	OtDeferredPass::renderOpaqueInstancedGeometry
//

void OtDeferredPass::renderOpaqueInstancedGeometry(OtSceneRendererContext& ctx, OtEntity entity, OtGeometryComponent& geometry, OtInstances* instances) {
	// bind pipeline
	if (geometry.wireframe) {
		ctx.pass->bindPipeline(instancedLinesPipeline);

	} else if (geometry.cullBack) {
		ctx.pass->bindPipeline(instancedCullingPipeline);

	} else {
		ctx.pass->bindPipeline(instancedNoCullingPipeline);
	}

	setMaterialUniforms(ctx, 0, 0, entity);

	// render geometry
	ctx.pass->setInstanceData(*instances);
	ctx.pass->render(geometry.asset->getGeometry());
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
	setLightingUniforms(ctx, 1, 5);
	setShadowUniforms(ctx, 2, 8);

	// bind samplers
	pass.bindFragmentSampler(0, lightingAlbedoSampler, gbuffer.getAlbedoTexture());
	pass.bindFragmentSampler(1, lightingNormalSampler, gbuffer.getNormalTexture());
	pass.bindFragmentSampler(2, lightingPbrSampler, gbuffer.getPbrTexture());
	pass.bindFragmentSampler(3, lightingEmissiveSampler, gbuffer.getEmissiveTexture());
	pass.bindFragmentSampler(4, lightingDepthSampler, gbuffer.getDepthTexture());

	pass.render(3, 1);
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
//	OtDeferredPass::initializePipelines
//

void OtDeferredPass::initializePipelines() {
	// configure rendering pipelines
	cullingPipeline.setShaders(OtDeferredVert, sizeof(OtDeferredVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	cullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	cullingPipeline.setVertexDescription(OtVertex::getDescription());

	noCullingPipeline.setShaders(OtDeferredVert, sizeof(OtDeferredVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	noCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	noCullingPipeline.setVertexDescription(OtVertex::getDescription());
	noCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	linesPipeline.setShaders(OtDeferredVert, sizeof(OtDeferredVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	linesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	linesPipeline.setVertexDescription(OtVertex::getDescription());
	linesPipeline.setCulling(OtRenderPipeline::Culling::none);
	linesPipeline.setFill(false);

	instancedCullingPipeline.setShaders(OtDeferredInstancingVert, sizeof(OtDeferredInstancingVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	instancedCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	instancedCullingPipeline.setVertexDescription(OtVertex::getDescription());
	instancedCullingPipeline.setInstanceDescription(OtInstances::getDescription());

	instancedNoCullingPipeline.setShaders(OtDeferredInstancingVert, sizeof(OtDeferredInstancingVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	instancedNoCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	instancedNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	instancedCullingPipeline.setInstanceDescription(OtInstances::getDescription());
	instancedNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	instancedLinesPipeline.setShaders(OtDeferredInstancingVert, sizeof(OtDeferredInstancingVert), OtDeferredPbrFrag, sizeof(OtDeferredPbrFrag));
	instancedLinesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::gBuffer);
	instancedLinesPipeline.setVertexDescription(OtVertex::getDescription());
	instancedCullingPipeline.setInstanceDescription(OtInstances::getDescription());
	instancedLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	instancedLinesPipeline.setFill(false);

	directionalLightPipeline.setShaders(OtDeferredLightingVert, sizeof(OtDeferredLightingVert), OtDeferredLightingFrag, sizeof(OtDeferredLightingFrag));
	directionalLightPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16d32);
	directionalLightPipeline.setCulling(OtRenderPipeline::Culling::none);
}
