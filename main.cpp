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

#include "OtFbm.h"
#include "OtLogo.h"


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

		if (texture.update(
			size.x,
			size.y,
			OtTexture::rgba8Texture,
			OtTexture::sampler | OtTexture::computeStorageWrite)) {

			fbm.render(texture);
		}

		ImGui::Image(texture.getTextureID(), size);
		ImGui::End();
	}

	void onTerminate() override {
		logo.clear();
		texture.clear();
		fbm.clear();
	}

private:
	OtLogo logo;
	OtTexture texture;
	OtFbm fbm;
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
