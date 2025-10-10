//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtComputePass.h"

#include "OtImageBasedLighting.h"


//
//	OtImageBasedLighting::update
//

void OtImageBasedLighting::update(OtIblComponent& component) {
	// see if the cubemap has changed
	OtCubeMap& cubemap = component.cubemap->getCubeMap();

	if (cubemap.getVersion() != iblSkyMapVersion) {
		static constexpr int threadCount = 16;
		static constexpr int brdfLutSize = 128;
		static constexpr int irradianceSize = 64;
		static constexpr int environmentSize = 256;
		static constexpr int environmentMipLevels = 8;

		// generate the Smith BRDF LUT
		auto usage = OtTexture::Usage(OtTexture::Usage::sampler | OtTexture::Usage::computeStorageWrite);
		iblBrdfLut.update(brdfLutSize, brdfLutSize, OtTexture::Format::rg16, usage);

		OtComputePass pass;
		pass.addOutputTexture(iblBrdfLut);
		pass.execute(brdfLutPipeline, brdfLutSize / threadCount, brdfLutSize / threadCount, 1);

		// render irradiance map
		iblIrradianceMap.create(irradianceSize, false);
		pass.addInputSampler(cubemapSampler, cubemap);
		pass.addOutputCubeMap(iblIrradianceMap);
		pass.execute(iblIrradianceMapPipeline, irradianceSize / threadCount, irradianceSize / threadCount, 1);

		// render environment map
		iblEnvironmentMap.create(environmentSize, true);
		maxEnvLevel = environmentMipLevels;

		for (auto mipLevel = 0; mipLevel <= environmentMipLevels; mipLevel++) {
			// set uniforms
			struct Uniforms {
				float roughness;
				float mipLevel;
				float environmentSize;
			} uniforms{
				float(mipLevel) / float(environmentMipLevels),
				float(mipLevel),
				float(environmentSize)};

			pass.addUniforms(&uniforms, sizeof(uniforms));
			pass.addInputSampler(cubemapSampler, cubemap);
			pass.addOutputCubeMap(iblEnvironmentMap);

			int mipSize = environmentSize / (1 << mipLevel);
			pass.execute(iblEnvironmentMapPipeline, mipSize / threadCount, mipSize / threadCount, 1);
		}

		iblSkyMapVersion = cubemap.getVersion();
		iblSkyMap = &cubemap;
	}
}