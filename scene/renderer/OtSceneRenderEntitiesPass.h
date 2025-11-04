//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <memory>

#include "OtInstances.h"
#include "OtMaterial.h"
#include "OtSampler.h"
#include "OtTextureAsset.h"

#include "OtGeometryRenderData.h"
#include "OtGrassComponent.h"
#include "OtModelRenderData.h"
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

	virtual void renderOpaqueGeometry(OtSceneRendererContext&, OtGeometryRenderData&) {}
	virtual void renderOpaqueModel(OtSceneRendererContext&, OtModelRenderData&) {}
	virtual void renderTerrain(OtSceneRendererContext&, OtEntity, OtTerrainComponent&) {}
	virtual void renderGrass(OtSceneRendererContext&, OtEntity, OtGrassComponent&) {}
	virtual void renderTransparentGeometry(OtSceneRendererContext&, OtGeometryRenderData&) {}

	// subclasses must overwrite these methods if required (these are called by renderEntities)
	virtual bool isRenderingOpaque() { return false; }
	virtual bool isRenderingTransparent() { return false; }

	// utility function for subclasses
	void renderGeometryHelper(
		OtSceneRendererContext& ctx,
		OtGeometryRenderData& grd,
		OtRenderPipeline& cullingPipeline,
		OtRenderPipeline& noCullingPipeline,
		OtRenderPipeline& linesPipeline,
		OtRenderPipeline& instancedCullingPipeline,
		OtRenderPipeline& instancedNoCullingPipeline,
		OtRenderPipeline& instancedLinesPipeline);

	void renderModelHelper(
		OtSceneRendererContext& ctx,
		OtModelRenderData& mrd,
		OtRenderPipeline& staticPipeline,
		OtRenderPipeline& animatedPipeline);

	void setCameraUniforms(OtSceneRendererContext& ctx, size_t uniformSlot);
	void setLightingUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot);
	void setShadowUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot);
	void setMaterialUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot, std::shared_ptr<OtMaterial> material);
	void setMaterialUniforms(OtSceneRendererContext& ctx, size_t uniformSlot, size_t samplerSlot, OtEntity entity);

private:
	// local support functions
	void bindFragmentSampler(OtSceneRendererContext& ctx, size_t slot, OtSampler& sampler, OtAsset<OtTextureAsset>& texture);
};
