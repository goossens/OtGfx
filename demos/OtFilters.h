//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtTexture.h"

#include "OtCheckerBoard.h"
#include "OtColorWheel.h"
#include "OtFbm.h"
#include "OtTileableFbm.h"

#include "OtAlphaOver.h"
#include "OtBlit.h"
#include "OtBlur.h"
#include "OtContrastSaturationBrightness.h"
#include "OtGaussian.h"
#include "OtIslandizer.h"
#include "OtNormalMapper.h"
#include "OtPixelate.h"
#include "OtPosterize.h"
#include "OtRgbaAdjust.h"
#include "OtRgbaCurve.h"
#include "OtSeamlessTile.h"
#include "OtSharpen.h"

#include "OtDemo.h"


//
//	OtFilters
//

class OtFilters : public OtDemo {
public:
	// run demo
	inline void run() override {
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
		ImGui::Begin("Filter", nullptr, ImGuiWindowFlags_NoDecoration);
		auto size = ImGui::GetContentRegionAvail();
		auto usage = OtTexture::Usage(OtTexture::Usage::sampler | OtTexture::Usage::computeStorageWrite);

		if (rawTexture.update(size.x, size.y, OtTexture::Format::rgba8, usage)) {
			generator.render(rawTexture);
			filteredTexture.update(size.x, size.y, OtTexture::Format::rgba8, usage);
			filter.render(rawTexture, filteredTexture);
		}

		ImGui::Image(filteredTexture.getTextureID(), size);
		ImGui::End();
	}

private:
	// properties
	OtTexture rawTexture;
	OtTexture filteredTexture;
	OtCheckerBoard generator;
	OtBlur filter;
};
