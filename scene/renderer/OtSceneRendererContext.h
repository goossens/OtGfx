//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <unordered_map>
#include <vector>

#include "glm/glm.hpp"

#include "OtCamera.h"
#include "OtCascadedShadowMap.h"
#include "OtGrass.h"
#include "OtImageBasedLighting.h"
#include "OtMaterial.h"
#include "OtRenderPass.h"

#include "OtGeometryRenderData.h"
#include "OtImageBasedLighting.h"
#include "OtModelRenderData.h"
#include "OtScene.h"
#include "OtTerrain.h"


//
//	OtSceneRendererContext
//

class OtSceneRendererContext {
public:
	// initialize context
	void initialize(OtScene* s, OtCamera c);

	// camera information
	OtCamera camera;
	OtCamera reflectionCamera;
	OtCamera refractionCamera;

	size_t cameraID;
	static inline size_t getMainCameraID() { return 0; }
	static inline size_t getReflectionCameraID() { return 1; }
	static inline size_t getRefractionCameraID() { return 2; }
	static inline size_t getShadowCameraID(size_t camera) { return 3 + camera; }

	// scene to render
	OtScene* scene;

	// rendering pass
	OtRenderPass* pass;

	// image base lighting
	OtImageBasedLighting ibl;

	// shadows
	OtCascadedShadowMap csm;

	// rendering flags
	bool hasImageBasedLighting;
	bool hasDirectionalLighting;
	bool hasPointLighting;
	bool hasOpaqueEntities;
	bool hasOpaqueGeometries;
	bool hasOpaqueModels;
	bool hasTerrainEntities;
	bool hasSkyEntities;
	bool hasTransparentGeometries;
	bool hasWaterEntities;
	bool hasGrassEntities;
	bool hasParticlesEntities;
	bool renderingShadow;

	// visible entity lists and render information
	std::vector<OtEntity> geometryEntities;
	std::vector<OtEntity> modelEntities;

	OtEntity iblEntity = OtEntityNull;
	OtEntity waterEntity = OtEntityNull;

	std::vector<OtEntity> opaqueGeometryEntities;
	std::vector<OtEntity> transparentGeometryEntities;
	std::unordered_map<OtEntity, OtGeometryRenderData> geometryRenderData;
	std::unordered_map<OtEntity, OtModelRenderData> modelRenderData;

	// directional light information
	glm::vec3 directionalLightDirection;
	glm::vec3 directionalLightColor;
	float directionalLightAmbient;
	bool renderDirectionalLight;
	bool castShadow;
};
