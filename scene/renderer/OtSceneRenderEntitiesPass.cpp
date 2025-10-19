//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <cstdint>
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
			ctx.scene->view<OtGeometryComponent>().each([&](auto entity, auto&) {
				renderEntity(ctx, entity);
			});
		}

		// render models
		if (ctx.hasOpaqueModels) {
			ctx.scene->view<OtModelComponent>().each([&](auto entity, auto&) {
				renderEntity(ctx, entity);
			});
		}

		// render terrain
		if (ctx.hasTerrainEntities) {
			ctx.scene->view<OtTerrainComponent>().each([&](auto entity, auto&) {
				renderEntity(ctx, entity);
			});
		}

		// render grass
		if (ctx.hasGrassEntities) {
			ctx.scene->view<OtGrassComponent>().each([&](auto entity, auto&) {
				renderEntity(ctx, entity);
			});
		}
	}

	// render all transparent geometries
	if (isRenderingTransparent() && ctx.hasTransparentEntities) {
		ctx.scene->view<OtGeometryComponent>().each([&](auto entity, auto&) {
			renderEntity(ctx, entity);
		});
	}
}


//
//	OtSceneRenderEntitiesPass::renderEntity
//

void OtSceneRenderEntitiesPass::renderEntity(OtSceneRendererContext& ctx, OtEntity entity) {
	// render geometry (if required)
	if (ctx.scene->hasComponent<OtGeometryComponent>(entity)) {
		auto& geometry = ctx.scene->getComponent<OtGeometryComponent>(entity);

		if (geometry.asset.isReady() && (!ctx.renderingShadow || geometry.castShadow)) {
			// see if entity is visible
			auto globalTransform = ctx.scene->getGlobalTransform(entity);
			auto aabb = geometry.asset->getGeometry().getAABB().transform(globalTransform);

			// is this a case of instancing?
			if (ctx.scene->hasComponent<OtInstancingComponent>(entity)) {
				auto& instancing = ctx.scene->getComponent<OtInstancingComponent>(entity);
				auto instances = &instancing.asset->getInstances();

				if (!instancing.asset.isNull() && instances->determineVisibility(ctx.camera, aabb)) {
					if (geometry.transparent) {
						renderTransparentInstancedGeometry(ctx, entity, geometry, instances);

					} else {
						renderOpaqueInstancedGeometry(ctx, entity, geometry, instances);
					}
				}

			} else {
				// see if geometry is visible
				if (ctx.camera.isVisibleAABB(aabb)) {
					if (geometry.transparent) {
						renderTransparentGeometry(ctx, entity, geometry);

					} else {
						renderOpaqueGeometry(ctx, entity, geometry);
					}
				}
			}
		}
	}

	// render model (if required)
	if (ctx.scene->hasComponent<OtModelComponent>(entity)) {
		auto& model = ctx.scene->getComponent<OtModelComponent>(entity);

		if (model.asset.isReady() && (!ctx.renderingShadow || model.castShadow)) {
			// see if model is visible
			auto globalTransform = ctx.scene->getGlobalTransform(entity);
			auto aabb = model.asset->getModel().getAABB().transform(globalTransform);

			// see if model is visible
			if (ctx.camera.isVisibleAABB(aabb)) {
				renderOpaqueModel(ctx, entity, model);
			}
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
		ctx.hasImageBasedLighting ? ctx.ibl->maxEnvLevel : 0
	};

	ctx.pass->setFragmentUniforms(uniformSlot, &uniforms, sizeof(uniforms));

	// submit the IBL samplers
	if (ctx.hasImageBasedLighting) {
		ctx.pass->bindFragmentSampler(samplerSlot++, iblBrdfLutSampler, ctx.ibl->iblBrdfLut);
		ctx.pass->bindFragmentSampler(samplerSlot++, iblIrradianceMapSampler, ctx.ibl->iblIrradianceMap);
		ctx.pass->bindFragmentSampler(samplerSlot++, iblEnvironmentMapSampler, ctx.ibl->iblEnvironmentMap);

	} else {
		auto& gpu = OtGpu::instance();
		ctx.pass->bindFragmentSampler(samplerSlot++, iblBrdfLutSampler, gpu.transparentDummyTexture);
		ctx.pass->bindFragmentSampler(samplerSlot++, iblIrradianceMapSampler, gpu.dummyCubeMap);
		ctx.pass->bindFragmentSampler(samplerSlot++, iblEnvironmentMapSampler, gpu.dummyCubeMap);
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
			ctx.csm->getCamera(0).viewProjectionMatrix,
			ctx.csm->getCamera(1).viewProjectionMatrix,
			ctx.csm->getCamera(2).viewProjectionMatrix,
			ctx.csm->getCamera(3).viewProjectionMatrix
		},
		ctx.csm->getDistance(0),
		ctx.csm->getDistance(1),
		ctx.csm->getDistance(2),
		ctx.csm->getDistance(3),
		1.0f / ctx.csm->getSize(),
		static_cast<uint32_t>(ctx.castShadow)
	};

	ctx.pass->setFragmentUniforms(uniformSlot, &uniforms, sizeof(uniforms));

	// set textures
	ctx.pass->bindFragmentSampler(samplerSlot++, shadowMap0Sampler, ctx.csm->getDepthTexture(0));
	ctx.pass->bindFragmentSampler(samplerSlot++, shadowMap1Sampler, ctx.csm->getDepthTexture(1));
	ctx.pass->bindFragmentSampler(samplerSlot++, shadowMap2Sampler, ctx.csm->getDepthTexture(2));
	ctx.pass->bindFragmentSampler(samplerSlot++, shadowMap3Sampler, ctx.csm->getDepthTexture(3));
}


//
//	OtSceneRenderEntitiesPass::setMaterialUniforms
//

void OtSceneRenderEntitiesPass::setMaterialUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot, OtEntity entity) {
	// get current material
	std::shared_ptr<OtMaterial> material;

	if (ctx.scene->hasComponent<OtMaterialComponent>(entity)) {
		material = ctx.scene->getComponent<OtMaterialComponent>(entity).material;

	} else {
		material = std::make_shared<OtMaterial>();
	}

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
	bindFragmentSampler(ctx, samplerSlot++, albedoSampler, material->albedoTexture);
	bindFragmentSampler(ctx, samplerSlot++, metallicRoughnessSampler, material->metallicRoughnessTexture);
	bindFragmentSampler(ctx, samplerSlot++, emissiveSampler, material->emissiveTexture);
	bindFragmentSampler(ctx, samplerSlot++, aoSampler, material->aoTexture);
	bindFragmentSampler(ctx, samplerSlot++, normalSampler, material->normalTexture);
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
