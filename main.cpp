//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <cmath>

#include "OtFramework.h"

#include "OtComputePass.h"
#include "OtComputePipeline.h"
#include "OtGradientComp.h"
#include "OtLogo.h"


//
//	Simple app
//

class SimpleApp : public OtFrameworkApp {
public:
	void onSetup() override {
		generator.load(OtGradientComp,sizeof(OtGradientComp));
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

		texture.update(size.x, size.y, OtTexture::rgba8Texture, OtTexture::sampler | OtTexture::computeStorageWrite);

		OtComputePass pass;
		pass.addOutputTexture(texture);
		pass.begin(generator);
		pass.dispatch(static_cast<size_t>(std::ceil(size.x / 16.0)), static_cast<size_t>(std::ceil(size.y / 16.0)), 1);
		pass.end();

		ImGui::Image(texture.getTextureID(), size);
		ImGui::End();
	}

	void onTerminate() override {
		logo.clear();
		texture.clear();
		generator.clear();
	}

private:
	OtLogo logo;
	OtTexture texture;
	OtComputePipeline generator;
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
