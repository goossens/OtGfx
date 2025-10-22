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
	// setup pass
	OtRenderPass pass;
	pass.start(framebuffer);
	ctx.pass = &pass;

	// submit common uniforms
	setCameraUniforms(ctx, 1);
	setLightingUniforms(ctx, 1, 5);
	setShadowUniforms(ctx, 2, 8);

	// render all entities
	renderEntities(ctx);

	// we're done
	pass.end();
}


//
//	OtForwardPass::renderTransparentGeometry
//

void OtForwardPass::renderTransparentGeometry(OtSceneRendererContext& ctx, OtEntity entity, OtGeometryComponent& component) {
	// bind pipeline
	if (component.wireframe) {
		ctx.pass->bindPipeline(linesPipeline);

	} else if (component.cullBack) {
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
	ctx.pass->render(component.asset->getGeometry());
}


//
//	OtForwardPass::initializePipelines
//

void OtForwardPass::initializePipelines() {
	cullingPipeline.setShaders(OtForwardVert, sizeof(OtForwardVert), OtForwardPbrFrag, sizeof(OtForwardPbrFrag));
	cullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16d32);
	cullingPipeline.setVertexDescription(OtVertex::getDescription());

	cullingPipeline.setBlend(
		OtRenderPipeline::BlendOperation::add,
		OtRenderPipeline::BlendFactor::one,
		OtRenderPipeline::BlendFactor::oneMinusSrcAlpha
	);

	noCullingPipeline.setShaders(OtForwardVert, sizeof(OtForwardVert), OtForwardPbrFrag, sizeof(OtForwardPbrFrag));
	noCullingPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16d32);
	noCullingPipeline.setVertexDescription(OtVertex::getDescription());
	noCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	noCullingPipeline.setBlend(
		OtRenderPipeline::BlendOperation::add,
		OtRenderPipeline::BlendFactor::one,
		OtRenderPipeline::BlendFactor::oneMinusSrcAlpha
	);

	linesPipeline.setShaders(OtForwardVert, sizeof(OtForwardVert), OtForwardPbrFrag, sizeof(OtForwardPbrFrag));
	linesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba16d32);
	linesPipeline.setVertexDescription(OtVertex::getDescription());
	linesPipeline.setCulling(OtRenderPipeline::Culling::none);
	linesPipeline.setFill(false);

	linesPipeline.setBlend(
		OtRenderPipeline::BlendOperation::add,
		OtRenderPipeline::BlendFactor::one,
		OtRenderPipeline::BlendFactor::oneMinusSrcAlpha
	);
}
