//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <set>

#include "glm/gtc/type_ptr.hpp"

#include "OtSceneRendererContext.h"


//
//	OtSceneRendererContext::initialize
//

void OtSceneRendererContext::initialize(OtScene* s, OtCamera c) {
	// store provided properties
	scene = s;
	camera = c;
	cameraID = getMainCameraID();

	// reset state
	hasImageBasedLighting = false;
	hasDirectionalLighting = false;
	hasPointLighting = false;
	hasOpaqueEntities = false;
	hasOpaqueGeometries = false;
	hasOpaqueModels = false;
	hasTerrainEntities = false;
	hasSkyEntities = false;
	hasTransparentGeometries = false;
	hasWaterEntities = false;
	hasGrassEntities = false;
	hasParticlesEntities = false;

	renderDirectionalLight = false;
	castShadow = false;

	iblEntity = OtEntityNull;
	waterEntity = OtEntityNull;

	directionalLightDirection = glm::vec3(0.0f);
	directionalLightColor = glm::vec3(0.0f);
	directionalLightAmbient = 0.0f;
	renderDirectionalLight = false;
	renderingShadow = false;

	geometryEntities.clear();
	opaqueGeometryEntities.clear();
	transparentGeometryEntities.clear();
	modelEntities.clear();

	// check entities, collect data and set flags
	scene->eachEntityBreadthFirst([&](OtEntity entity) {
		// process image based lighting
		if (scene->hasComponent<OtIblComponent>(entity)) {
			hasImageBasedLighting |= scene->getComponent<OtIblComponent>(entity).cubemap.isReady();
			iblEntity = entity;
		}

		// process directional light
		if (scene->hasComponent<OtDirectionalLightComponent>(entity)) {
			auto& light = scene->getComponent<OtDirectionalLightComponent>(entity);

			hasDirectionalLighting = true;
			directionalLightDirection = light.getDirectionToLight();
			directionalLightColor = light.color;
			directionalLightAmbient = light.ambient;
			renderDirectionalLight = true;
			castShadow = light.castShadow;
		}

		// process procedural sky
		if (scene->hasComponent<OtSkyComponent>(entity)) {
			auto& sky = scene->getComponent<OtSkyComponent>(entity);

			hasDirectionalLighting = true;
			directionalLightDirection = sky.getDirectionToSun();
			directionalLightColor = glm::vec3(0.2f + std::clamp(sky.elevation / 10.0f, 0.0f, 0.8f));
			directionalLightAmbient = std::clamp((sky.elevation + 6.0f) / 200.0f, 0.0f, 0.2f);
			renderDirectionalLight = false;
			castShadow = sky.castShadow;
		}

		// process point lighting
		if (scene->hasComponent<OtPointLightComponent>(entity)) {
			hasPointLighting = true;
		}

		// process all geometry entities
		if (scene->hasComponent<OtGeometryComponent>(entity)) {
			auto& geometry = scene->getComponent<OtGeometryComponent>(entity);

			// ignore geometry if it is not ready yet
			if (geometry.asset.isReady()) {
				bool transparent = geometry.transparent;
				hasOpaqueEntities |= !transparent;
				hasOpaqueGeometries |= !transparent;
				hasTransparentGeometries |= transparent;
				geometryEntities.emplace_back(entity);

				if (transparent) {
					transparentGeometryEntities.emplace_back(entity);

				} else {
					opaqueGeometryEntities.emplace_back(entity);
				}
			}
		}

		// process all model entities
		if (scene->hasComponent<OtModelComponent>(entity)) {
			// ignore model if it is not ready yet
			auto& model = scene->getComponent<OtModelComponent>(entity);

			if (model.asset.isReady()) {
				hasOpaqueEntities = true;
				hasOpaqueModels = true;
				modelEntities.emplace_back(entity);
			}
		}

		// process all terrain entities
		if (scene->hasComponent<OtTerrainComponent>(entity)) {
			hasOpaqueEntities = true;
			hasTerrainEntities = true;
		}

		// process all sky entities
		if (scene->hasComponent<OtSkyComponent>(entity) || scene->hasComponent<OtSkyBoxComponent>(entity)) {
			hasSkyEntities = true;
		}

		// process all water entities
		if (scene->hasComponent<OtWaterComponent>(entity)) {
			hasTransparentGeometries = true;
			hasWaterEntities = true;
			waterEntity = entity;
		}

		// process all grass entities
		if (scene->hasComponent<OtGrassComponent>(entity)) {
			hasOpaqueEntities = true;
			hasGrassEntities = true;
		}

		// process all particle entities
		if (scene->hasComponent<OtParticlesComponent>(entity)) {
			hasParticlesEntities = true;
		}
	});

	// update image based lighting (if required)
	if (hasImageBasedLighting) {
		ibl.update(scene->getComponent<OtIblComponent>(iblEntity));
	}

	// update shadowmaps (if required)
	if (castShadow) {
		csm.update(camera, directionalLightDirection);
	}

	// create water cameras (if required)
	if (waterEntity != OtEntityNull) {
		// get water component
		auto& water = scene->getComponent<OtWaterComponent>(waterEntity);

		// determine new view matrix
		// see http://khayyam.kaplinski.com/2011/09/reflective-water-with-glsl-part-i.html
		// and http://bcnine.com/articles/water/water.md.html
		static const float flip[16] = { // these must be in column-wise order for glm library
			1.0f,  0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f,  0.0f, 1.0f, 0.0f,
			0.0f,  0.0f, 0.0f, 1.0f
		};

		static float reflection[16] = { // these must be in column-wise order for glm library
			1.0f,  0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f,  0.0f, 1.0f, 0.0f,
			0.0f,  0.0f, 0.0f, 1.0f
		};

		reflection[13] = 2.0f * water.level;

		glm::mat4 sceneCameraMatrix = glm::inverse(camera.viewMatrix);
		glm::mat4 reflectionCameraMatrix = glm::make_mat4(reflection) * sceneCameraMatrix * glm::make_mat4(flip);
		glm::mat4 reflectionViewMatrix = glm::inverse(reflectionCameraMatrix);

		// setup the reflection camera
		int width = camera.width / 2;
		int height = camera.height / 2;
		reflectionCamera = OtCamera{width, height, camera.projectionMatrix, reflectionViewMatrix};

		// setup the refraction camera (if required)
		if (water.useRefractance) {
			refractionCamera = OtCamera{width, height, camera.projectionMatrix, camera.viewMatrix};
		}
	}

	// update geometry render data
	std::set<OtEntity> currentEntities;

	for (auto entity : geometryEntities) {
		auto& grd = geometryRenderData[entity];
		grd.analyzeEntity(scene, entity);
		grd.analyzeCamera(getMainCameraID(), camera);

		if (castShadow) {
			grd.analyzeCamera(getShadowCameraID(0), csm.getCamera(0));
			grd.analyzeCamera(getShadowCameraID(1), csm.getCamera(1));
			grd.analyzeCamera(getShadowCameraID(2), csm.getCamera(2));
			grd.analyzeCamera(getShadowCameraID(3), csm.getCamera(3));
		}

		if (waterEntity != OtEntityNull) {
			grd.analyzeCamera(getReflectionCameraID(), reflectionCamera);

			if (scene->getComponent<OtWaterComponent>(waterEntity).useRefractance) {
				grd.analyzeCamera(getRefractionCameraID(), refractionCamera);
			}
		}

		currentEntities.emplace(entity);
	}

	// remove geometries that might have disappeared
	for (auto i = geometryRenderData.begin(); i != geometryRenderData.end();) {
		if (currentEntities.find(i->first) == currentEntities.end()) {
			i = geometryRenderData.erase(i);

		} else {
			i++;
		}
	}

	// update model render data
	currentEntities.clear();

	for (auto entity : modelEntities) {
		auto& mrd = modelRenderData[entity];
		mrd.analyzeEntity(scene, entity);
		mrd.analyzeCamera(getMainCameraID(), camera);

		if (castShadow) {
			mrd.analyzeCamera(getShadowCameraID(0), csm.getCamera(0));
			mrd.analyzeCamera(getShadowCameraID(1), csm.getCamera(1));
			mrd.analyzeCamera(getShadowCameraID(2), csm.getCamera(2));
			mrd.analyzeCamera(getShadowCameraID(3), csm.getCamera(3));
		}

		currentEntities.emplace(entity);
	}

	// remove models that might have disappeared
	for (auto i = modelRenderData.begin(); i != modelRenderData.end();) {
		if (currentEntities.find(i->first) == currentEntities.end()) {
			i = modelRenderData.erase(i);

		} else {
			i++;
		}
	}
}
