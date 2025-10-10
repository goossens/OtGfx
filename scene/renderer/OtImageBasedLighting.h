//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtComputePipeline.h"
#include "OtCubeMap.h"
#include "OtSampler.h"
#include "OtTexture.h"

#include "OtIblComponent.h"

#include "OtBrdfLutComp.h"
#include "OtIblEnvironmentMapComp.h"
#include "OtIblIrradianceMapComp.h"


//
//	OtImageBasedLighting
//

class OtImageBasedLighting {
public:
	// update (if required)
	void update(OtIblComponent& component);

	// textures
	OtTexture iblBrdfLut;

	// cubemaps
	OtCubeMap* iblSkyMap;
	OtCubeMap iblIrradianceMap;
	OtCubeMap iblEnvironmentMap;
	int maxEnvLevel;

private:
	// samplers
	OtSampler cubemapSampler;

	// compute programs
	OtComputePipeline brdfLutPipeline{OtBrdfLutComp, sizeof(OtBrdfLutComp)};
	OtComputePipeline iblEnvironmentMapPipeline{OtIblEnvironmentMapComp, sizeof(OtIblEnvironmentMapComp)};
	OtComputePipeline iblIrradianceMapPipeline{OtIblIrradianceMapComp, sizeof(OtIblIrradianceMapComp)};

	// image based lighting data
	int iblSkyMapVersion = 0;
};
