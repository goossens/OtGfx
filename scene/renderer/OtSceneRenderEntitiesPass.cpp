//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

#include "OtGpu.h"
#include "OtMaterial.h"

#include "OtMaterialComponent.h"
#include "OtSceneRenderEntitiesPass.h"


//
//	OtSceneRenderEntitiesPass::renderEntities
//

void OtSceneRenderEntitiesPass::renderEntities(OtSceneRendererContext& ctx) {
	// render all opaque entities
	if (isRenderingOpaque() && ctx.hasOpaqueEntities) {
		// render geometries
		if (ctx.hasOpaqueGeometries) {
			for (auto entity : ctx.opaqueGeometryEntities) {
				auto& grd = ctx.geometryRenderData[entity];
				auto& camera = grd.cameras[ctx.cameraID];

				if (camera.visible) {
					renderOpaqueGeometry(ctx, grd);
				}
			}
		}

		// render models
		if (ctx.hasOpaqueModels) {
			for (auto entity : ctx.modelEntities) {
				auto& mrd = ctx.modelRenderData[entity];

				if (mrd.visible[ctx.cameraID]) {
					renderOpaqueModel(ctx, mrd);
				}
			}
		}

		// render terrain
		if (ctx.hasTerrainEntities) {
			ctx.scene->view<OtTerrainComponent>().each([&](auto entity, auto& terrain) {
				if (!ctx.renderingShadow || terrain.terrain->isCastingShadow()) {
					renderTerrain(ctx, entity, terrain);
				}
			});
		}

		// render grass
		if (ctx.hasGrassEntities) {
			ctx.scene->view<OtGrassComponent>().each([&](auto entity, auto& grass) {
				if (!ctx.renderingShadow || grass.grass->castShadow) {
					renderGrass(ctx, entity, grass);
				}
			});
		}
	}

	// render all transparent geometries
	if (isRenderingTransparent() && ctx.hasTransparentGeometries) {
		for (auto entity : ctx.transparentGeometryEntities) {
			auto& grd = ctx.geometryRenderData[entity];
			auto& camera = grd.cameras[ctx.cameraID];

			if (camera.visible) {
				renderTransparentGeometry(ctx, grd);
			}
		}
	}
}


//
//	OtSceneRenderEntitiesPass::renderEntity
//

void OtSceneRenderEntitiesPass::renderEntity(OtSceneRendererContext& ctx, OtEntity entity) {
	// render geometry (if required)
	if (ctx.scene->hasComponent<OtGeometryComponent>(entity)) {
		auto& grd = ctx.geometryRenderData[entity];
		auto& camera = grd.cameras[ctx.cameraID];

		if (camera.visible) {
			auto& geometry = ctx.scene->getComponent<OtGeometryComponent>(entity);

			if (geometry.transparent) {
				renderTransparentGeometry(ctx, grd);

			} else {
				renderOpaqueGeometry(ctx, grd);
			}
		}
	}

	// render model (if required)
	if (ctx.scene->hasComponent<OtModelComponent>(entity)) {
		auto& mrd = ctx.modelRenderData[entity];

		if (mrd.visible[ctx.cameraID]) {
			renderOpaqueModel(ctx, mrd);
		}
	}

	// render terrain (if required)
	if (ctx.scene->hasComponent<OtTerrainComponent>(entity)) {
		auto& terrain = ctx.scene->getComponent<OtTerrainComponent>(entity);

		if (!ctx.renderingShadow || terrain.terrain->isCastingShadow()) {
			renderTerrain(ctx, entity, terrain);
		}
	}

	// render grass (if required)
	if (ctx.scene->hasComponent<OtGrassComponent>(entity)) {
		auto& grass = ctx.scene->getComponent<OtGrassComponent>(entity);

		if (!ctx.renderingShadow || grass.grass->castShadow) {
			renderGrass(ctx, entity, grass);
		}
	}
}


//
//	OtSceneRenderEntitiesPass::renderGeometryHelper
//

void OtSceneRenderEntitiesPass::renderGeometryHelper(
	OtSceneRendererContext& ctx,
	OtGeometryRenderData& grd,
	OtRenderPipeline& cullingPipeline,
	OtRenderPipeline& noCullingPipeline,
	OtRenderPipeline& linesPipeline,
	OtRenderPipeline& instancedCullingPipeline,
	OtRenderPipeline& instancedNoCullingPipeline,
	OtRenderPipeline& instancedLinesPipeline) {

	// do we have instances?
	if (grd.instances) {
		// bind pipeline
		if (grd.component->wireframe) {
			ctx.pass->bindPipeline(instancedLinesPipeline);

		} else if (grd.component->cullBack) {
			ctx.pass->bindPipeline(instancedCullingPipeline);

		} else {
			ctx.pass->bindPipeline(instancedNoCullingPipeline);
		}

		// setup instance data
		ctx.pass->setInstanceData(grd.cameras[ctx.cameraID].idb);

	} else {
		// bind pipeline
		if (grd.component->wireframe) {
			ctx.pass->bindPipeline(linesPipeline);

		} else if (grd.component->cullBack) {
			ctx.pass->bindPipeline(cullingPipeline);

		} else {
			ctx.pass->bindPipeline(noCullingPipeline);
		}

		// set vertex uniforms
		struct Uniforms {
			glm::mat4 modelMatrix;
		} uniforms {
			ctx.scene->getGlobalTransform(grd.entity)
		};

		ctx.pass->setVertexUniforms(0, &uniforms, sizeof(Uniforms));
	}

	// render geometry
	setMaterialUniforms(ctx, 0, 0, grd.entity);
	ctx.pass->render(grd.component->asset->getGeometry());
}


//
//	OtSceneRenderEntitiesPass::renderModelHelper
//

void OtSceneRenderEntitiesPass::renderModelHelper(
	OtSceneRendererContext& ctx,
	OtModelRenderData& mrd,
	OtRenderPipeline& staticPipeline,
	OtRenderPipeline& animatedPipeline) {

	// process all render commands
	auto globalTransform = ctx.scene->getGlobalTransform(mrd.entity);
	auto renderList = mrd.model->getRenderList(globalTransform);

	for (auto& cmd : renderList) {
		// handle animations
		if (cmd.animation) {
			// bind pipeline
			ctx.pass->bindPipeline(animatedPipeline);

			// set vertex uniforms
			struct Uniforms {
				glm::mat4 models[64];
			} uniforms;

			std::memcpy(&uniforms, cmd.transforms.data(), std::min(cmd.transforms.size(), 64ul) * sizeof(glm::mat4));
			ctx.pass->setVertexUniforms(0, &uniforms, sizeof(Uniforms));

			// bind animation data
			ctx.pass->setAnimationData(cmd.mesh->getBonesBuffer());

		} else {
			// bind pipeline
			ctx.pass->bindPipeline(staticPipeline);

			// set vertex uniforms
			struct Uniforms {
				glm::mat4 modelMatrix;
			} uniforms {
				cmd.transforms[0]
			};

			ctx.pass->setVertexUniforms(0, &uniforms, sizeof(Uniforms));
		}

		// render geometry
		setMaterialUniforms(ctx, 0, 0, cmd.material);
		ctx.pass->render(cmd.mesh->getVertexBuffer(), cmd.mesh->getIndexBuffer());
	}
}
//
//	OtSceneRenderEntitiesPass::setCameraUniforms
//

void OtSceneRenderEntitiesPass::setCameraUniforms(OtSceneRendererContext& ctx, size_t uniformSlot) {
	// set uniforms
	struct Uniforms {
		glm::mat4 viewProjectionMatrix;
		glm::mat4 inverseViewProjectionMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 inverseProjectionMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 inverseViewMatrix;
	} uniforms {
		ctx.camera.viewProjectionMatrix,
		glm::inverse(ctx.camera.viewProjectionMatrix),
		ctx.camera.projectionMatrix,
		glm::inverse(ctx.camera.projectionMatrix),
		ctx.camera.viewMatrix,
		glm::inverse(ctx.camera.viewMatrix)
	};

	ctx.pass->setVertexUniforms(uniformSlot, &uniforms, sizeof(uniforms));
}


//
//	OtSceneRenderEntitiesPass::setLightingUniforms
//

void OtSceneRenderEntitiesPass::setLightingUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot) {
	// set uniforms
	struct Uniforms {
		glm::vec3 cameraPosition;
		uint32_t hasDirectionalLighting;
		glm::vec3 directionalLightDirection;
		float directionalLightAmbient;
		glm::vec3 directionalLightColor;
		uint32_t hasImageBasedLighting;
		int iblEnvLevel;
	} uniforms {
		ctx.camera.position,
		static_cast<uint32_t>(ctx.hasDirectionalLighting),
		ctx.directionalLightDirection,
		ctx.directionalLightAmbient,
		ctx.directionalLightColor,
		static_cast<uint32_t>(ctx.hasImageBasedLighting),
		ctx.hasImageBasedLighting ? ctx.ibl.maxEnvLevel : 0
	};

	ctx.pass->setFragmentUniforms(uniformSlot, &uniforms, sizeof(uniforms));

	// submit the IBL samplers
	if (ctx.hasImageBasedLighting) {
		ctx.pass->bindFragmentSampler(samplerSlot++, ctx.iblBrdfLutSampler, ctx.ibl.iblBrdfLut);
		ctx.pass->bindFragmentSampler(samplerSlot++, ctx.iblIrradianceMapSampler, ctx.ibl.iblIrradianceMap);
		ctx.pass->bindFragmentSampler(samplerSlot++, ctx.iblEnvironmentMapSampler, ctx.ibl.iblEnvironmentMap);

	} else {
		auto& gpu = OtGpu::instance();
		ctx.pass->bindFragmentSampler(samplerSlot++, ctx.iblBrdfLutSampler, gpu.transparentDummyTexture);
		ctx.pass->bindFragmentSampler(samplerSlot++, ctx.iblIrradianceMapSampler, gpu.dummyCubeMap);
		ctx.pass->bindFragmentSampler(samplerSlot++, ctx.iblEnvironmentMapSampler, gpu.dummyCubeMap);
	}
}


//
//	OtSceneRenderEntitiesPass::setShadowUniforms
//

void OtSceneRenderEntitiesPass::setShadowUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot) {
	// set uniforms
	struct Uniforms {
		glm::mat4 viewTransform;
		glm::mat4 shadowViewProjTransform[4];
		float cascade0Distance;
		float cascade1Distance;
		float cascade2Distance;
		float cascade3Distance;
		float shadowTexelSize;
		uint32_t shadowEnabled;
	} uniforms {
		ctx.camera.viewMatrix,
		{
			ctx.csm.getCamera(0).viewProjectionMatrix,
			ctx.csm.getCamera(1).viewProjectionMatrix,
			ctx.csm.getCamera(2).viewProjectionMatrix,
			ctx.csm.getCamera(3).viewProjectionMatrix
		},
		ctx.csm.getDistance(0),
		ctx.csm.getDistance(1),
		ctx.csm.getDistance(2),
		ctx.csm.getDistance(3),
		1.0f / ctx.csm.getSize(),
		static_cast<uint32_t>(ctx.castShadow)
	};

	ctx.pass->setFragmentUniforms(uniformSlot, &uniforms, sizeof(uniforms));

	// set textures
	ctx.pass->bindFragmentSampler(samplerSlot++, ctx.shadowMap0Sampler, ctx.csm.getDepthTexture(0));
	ctx.pass->bindFragmentSampler(samplerSlot++, ctx.shadowMap1Sampler, ctx.csm.getDepthTexture(1));
	ctx.pass->bindFragmentSampler(samplerSlot++, ctx.shadowMap2Sampler, ctx.csm.getDepthTexture(2));
	ctx.pass->bindFragmentSampler(samplerSlot++, ctx.shadowMap3Sampler, ctx.csm.getDepthTexture(3));
}


//
//	OtSceneRenderEntitiesPass::setMaterialUniforms
//

void OtSceneRenderEntitiesPass::setMaterialUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot, std::shared_ptr<OtMaterial> material) {
	// set uniforms
	struct Uniforms {
		glm::vec4 albedoColor;
		glm::vec3 emissiveColor;
		float textureScale;
		glm::vec2 textureOffset;
		float metallicFactor;
		float roughnessFactor;
		float aoFactor;
		uint32_t hasAlbedoTexture;
		uint32_t hasMetallicRoughnessTexture;
		uint32_t hasEmissiveTexture;
		uint32_t hasAoTexture;
		uint32_t hasNormalTexture;
	} uniforms {
		material->albedo,
		material->emissive,
		material->scale,
		material->offset,
		material->metallic,
		material->roughness,
		material->ao,
		static_cast<uint32_t>(material->albedoTexture.isReady()),
		static_cast<uint32_t>(material->metallicRoughnessTexture.isReady()),
		static_cast<uint32_t>(material->emissiveTexture.isReady()),
		static_cast<uint32_t>(material->aoTexture.isReady()),
		static_cast<uint32_t>(material->normalTexture.isReady())
	};

	ctx.pass->setFragmentUniforms(uniformSlot, &uniforms, sizeof(uniforms));

	// set textures
	bindFragmentSampler(ctx, samplerSlot++, ctx.albedoSampler, material->albedoTexture);
	bindFragmentSampler(ctx, samplerSlot++, ctx.metallicRoughnessSampler, material->metallicRoughnessTexture);
	bindFragmentSampler(ctx, samplerSlot++, ctx.emissiveSampler, material->emissiveTexture);
	bindFragmentSampler(ctx, samplerSlot++, ctx.aoSampler, material->aoTexture);
	bindFragmentSampler(ctx, samplerSlot++, ctx.normalSampler, material->normalTexture);
}


//
//	OtSceneRenderEntitiesPass::setMaterialUniforms
//

void OtSceneRenderEntitiesPass::setMaterialUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot, OtEntity entity) {
	setMaterialUniforms(
		ctx,
		uniformSlot,
		samplerSlot,
		ctx.scene->hasComponent<OtMaterialComponent>(entity)
			? ctx.scene->getComponent<OtMaterialComponent>(entity).material
			: std::make_shared<OtMaterial>());
}


//
//	OtSceneRenderEntitiesPass::bindFragmentSampler
//

void OtSceneRenderEntitiesPass::bindFragmentSampler(OtSceneRendererContext& ctx, size_t slot, OtSampler& sampler, OtAsset<OtTextureAsset>& texture) {
	if (texture.isReady()) {
		ctx.pass->bindFragmentSampler(slot, sampler, texture->getTexture());

	} else {
		ctx.pass->bindFragmentSampler(slot, sampler, OtGpu::instance().transparentDummyTexture);
	}
}
