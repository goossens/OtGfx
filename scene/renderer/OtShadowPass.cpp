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
#include "OtShadowVert.h"
#include "OtShadowOpaqueFrag.h"


//
//	OtShadowPass::OtShadowPass
//

OtShadowPass::OtShadowPass() {
	// configure rendering pipelines
	opaqueCullingPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueCullingPipeline.setVertexDescription(OtVertex::getDescription());

	opaqueNoCullingPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueNoCullingPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueNoCullingPipeline.setCulling(OtRenderPipeline::Culling::none);

	opaqueLinesPipeline.setShaders(OtShadowVert, sizeof(OtShadowVert), OtShadowOpaqueFrag, sizeof(OtShadowOpaqueFrag));
	opaqueLinesPipeline.setVertexDescription(OtVertex::getDescription());
	opaqueLinesPipeline.setCulling(OtRenderPipeline::Culling::none);
	opaqueLinesPipeline.setFill(false);
}


//
//	OtShadowPass::render
//

void OtShadowPass::render(OtSceneRendererContext& ctx) {
	// update shadowmaps
	ctx.csm->update(ctx.camera, ctx.directionalLightDirection);

	// save context part the we will temporarily overwrite
	auto camera = ctx.camera;
	auto renderingShadow = ctx.renderingShadow;

	// render each cascade
	for (size_t i = 0; i < OtCascadedShadowMap::maxCascades; i++) {
		// setup pass to render entities as opaque blobs
		OtRenderPass pass;
		pass.start(ctx.csm->getFrameBuffer(i));

		ctx.camera = ctx.csm->getCamera(i);
		ctx.renderingShadow = true;
		ctx.pass = &pass;

		setCameraUniforms(ctx, 1);
		renderEntities(ctx);

		pass.end();
	}

	ctx.camera = camera;
	ctx.renderingShadow = renderingShadow;
}


//
//	OtShadowPass::renderOpaqueGeometry
//

void OtShadowPass::renderOpaqueGeometry(OtSceneRendererContext& ctx, OtEntity entity, OtGeometryComponent& component) {
	// bind pipeline
	if (component.wireframe) {
		ctx.pass->bindPipeline(opaqueLinesPipeline);

	} else if (component.cullBack) {
		ctx.pass->bindPipeline(opaqueCullingPipeline);

	} else {
		ctx.pass->bindPipeline(opaqueNoCullingPipeline);
	}

	// set uniforms
	struct Uniforms {
		glm::mat4 modelMatrix;
	} uniforms {
		ctx.scene->getGlobalTransform(entity)
	};

	ctx.pass->setVertexUniforms(0, &uniforms, sizeof(Uniforms));

	// render geometry
	ctx.pass->render(component.asset->getGeometry());
}
