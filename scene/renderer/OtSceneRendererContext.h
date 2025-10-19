//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "glm/glm.hpp"

#include "OtCamera.h"
#include "OtCascadedShadowMap.h"
#include "OtRenderPass.h"

#include "OtScene.h"

#include "OtImageBasedLighting.h"


//
//	OtSceneRendererContext
//

class OtSceneRendererContext {
public:
	// initialize context
	void initialize(OtCamera c, OtScene* s, OtImageBasedLighting* i, OtCascadedShadowMap* sm);

	// camera information
	OtCamera camera;

	// scene to render
	OtScene* scene;

	// rendering pass
	OtRenderPass* pass;

	// image base lighting
	OtImageBasedLighting* ibl;

	// shadows
	OtCascadedShadowMap* csm;

	// rendering flags
	bool hasImageBasedLighting;
	bool hasDirectionalLighting;
	bool hasPointLighting;
	bool hasOpaqueEntities;
	bool hasOpaqueGeometries;
	bool hasOpaqueModels;
	bool hasTerrainEntities;
	bool hasSkyEntities;
	bool hasTransparentEntities;
	bool hasWaterEntities;
	bool hasGrassEntities;
	bool hasParticlesEntities;
	bool renderingShadow;

	// directional light information
	glm::vec3 directionalLightDirection;
	glm::vec3 directionalLightColor;
	float directionalLightAmbient;
	bool renderDirectionalLight;
	bool castShadow;

	// key entities
	OtEntity iblEntity = OtEntityNull;
	OtEntity waterEntity = OtEntityNull;
};
