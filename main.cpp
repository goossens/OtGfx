//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtFramework.h"

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

#include "OtLogo.h"
#include "OtPixelate.h"

#include "OtFrameBuffer.h"
#include "OtIndexBuffer.h"
#include "OtVertexBuffer.h"
#include "OtRenderPass.h"
#include "OtRenderPipeline.h"


//
//	Simple app
//

class SimpleApp : public OtFrameworkApp {
public:
	void onSetup() override {
	}

	void onRender() override {
		compute();
	}

	void splash() {
		// render splash screen
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		ImGui::Begin(
			"SplashScreen",
			nullptr,
			ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoInputs);

		ImGui::Image(logo.getTextureID(), logo.getSize());
		ImGui::End();
	}

	void compute(){
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
		ImGui::Begin("Compute", nullptr, ImGuiWindowFlags_NoDecoration);
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

	void onTerminate() override {
		logo.clear();
		rawTexture.clear();
		filteredTexture.clear();
		generator.clear();
		filter.clear();
	}

private:
	OtLogo logo;
	OtTexture rawTexture;
	OtTexture filteredTexture;
	OtCheckerBoard generator;
	OtBlur filter;
};


//
//	main
//

int main(int, char**) {
	OtFramework framework;
	SimpleApp app;
	framework.run(&app);
	return 0;
}
