//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtMaterial.h"
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
	// utility function for subclasses
	void setCameraUniforms(OtSceneRendererContext& ctx, size_t uniformSlot);
	void setClippingUniforms(OtSceneRendererContext& ctx, size_t uniformSlot);
	void setLightingUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot);
	void setShadowUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot);
	void setMaterialUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot, OtEntity entity);

	void renderEntities(OtSceneRendererContext& ctx);

	// subclasses must overwrite these methods (if required)
	virtual bool isRenderingOpaque() { return false; }
	virtual bool isRenderingTransparent() { return false; }

	virtual void renderOpaqueGeometry([[maybe_unused]] OtSceneRendererContext& ctx, [[maybe_unused]] OtEntity entity, [[maybe_unused]] OtGeometryComponent& component) {}
	virtual void renderOpaqueInstancedGeometry([[maybe_unused]] OtSceneRendererContext& ctx, [[maybe_unused]] OtEntity entity, [[maybe_unused]] OtGeometryComponent& component) {}
	virtual void renderOpaqueModel([[maybe_unused]] OtSceneRendererContext& ctx, [[maybe_unused]] OtEntity entity, [[maybe_unused]] OtModelComponent& component) {}
	virtual void renderOpaqueInstancedModel([[maybe_unused]] OtSceneRendererContext& ctx, [[maybe_unused]] OtEntity entity, [[maybe_unused]] OtModelComponent& component) {}
	virtual void renderTerrain([[maybe_unused]] OtSceneRendererContext& ctx, [[maybe_unused]] OtEntity entity, [[maybe_unused]] OtTerrainComponent& component) {}
	virtual void renderGrass([[maybe_unused]] OtSceneRendererContext& ctx, [[maybe_unused]] OtEntity entity, [[maybe_unused]] OtGrassComponent& component) {}
	virtual void renderTransparentGeometry([[maybe_unused]] OtSceneRendererContext& ctx, [[maybe_unused]] OtEntity entity, [[maybe_unused]] OtGeometryComponent& component) {}

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
