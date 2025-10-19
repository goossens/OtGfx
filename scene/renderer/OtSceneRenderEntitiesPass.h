//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtInstances.h"
#include "OtSampler.h"
#include "OtTextureAsset.h"

#include "OtGeometryComponent.h"
#include "OtGrassComponent.h"
#include "OtModelComponent.h"
#include "OtSceneRendererContext.h"
#include "OtTerrainComponent.h"


//
//	OtSceneRenderEntitiesPass
//

class OtSceneRenderEntitiesPass {
public:
	// destructor
	virtual ~OtSceneRenderEntitiesPass() {}

protected:
	// render all entities in scene and call appropriate render functions (see below)
	void renderEntities(OtSceneRendererContext& ctx);
	void renderEntity(OtSceneRendererContext& ctx, OtEntity entity);

	// subclasses must overwrite these methods if required (these are called by renderEntities)
	virtual bool isRenderingOpaque() { return false; }
	virtual bool isRenderingTransparent() { return false; }

	virtual void renderOpaqueGeometry(OtSceneRendererContext&, OtEntity, OtGeometryComponent&) {}
	virtual void renderOpaqueInstancedGeometry(OtSceneRendererContext&, OtEntity, OtGeometryComponent&, OtInstances*) {}
	virtual void renderOpaqueModel(OtSceneRendererContext&, OtEntity, OtModelComponent&) {}
	virtual void renderOpaqueInstancedModel(OtSceneRendererContext&, OtEntity, OtModelComponent&, OtInstances*) {}
	virtual void renderTerrain(OtSceneRendererContext&, OtEntity, OtTerrainComponent&) {}
	virtual void renderGrass(OtSceneRendererContext&, OtEntity, OtGrassComponent&) {}
	virtual void renderTransparentGeometry(OtSceneRendererContext&, OtEntity, OtGeometryComponent&) {}
	virtual void renderTransparentInstancedGeometry(OtSceneRendererContext&, OtEntity, OtGeometryComponent&, OtInstances*) {}

	// utility function for subclasses
	void setCameraUniforms(OtSceneRendererContext& ctx, size_t uniformSlot);
	void setLightingUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot);
	void setShadowUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot);
	void setMaterialUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot, OtEntity entity);

private:
	// properties
	OtSampler iblBrdfLutSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler iblIrradianceMapSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler iblEnvironmentMapSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};

	OtSampler shadowMap0Sampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler shadowMap1Sampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler shadowMap2Sampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler shadowMap3Sampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};

	OtSampler albedoSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler normalSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler metallicRoughnessSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler emissiveSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};
	OtSampler aoSampler{OtSampler::Filter::nearest, OtSampler::Addressing::clamp};

	// local support functions
	void bindFragmentSampler(OtSceneRendererContext& ctx, size_t slot, OtSampler& sampler, OtAsset<OtTextureAsset>& texture);
};
