//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtFramework.h"

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

	void onTerminate() override {
		generator.clear();
		logo.clear();
	}

private:
	OtComputePipeline generator;
	OtLogo logo;
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
