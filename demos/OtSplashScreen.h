//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtLogo.h"
#include "OtUi.h"

#include "OtDemo.h"


//
//	OtSplashScreen
//

class OtSplashScreen : public OtDemo {
public:
	// run demo
	inline void run() override {
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
		OtUi::centeredText("Welcome to the ObjectTalk");
		OtUi::centeredText("Graphics Backend Rewrite Testbed");

		OtUi::centeredText("");
		OtUi::centeredText("Use the up and down arrows to cycle");
		OtUi::centeredText("between the demos and use the left and right arrows");
		OtUi::centeredText("to cycle between demo variations (if available)");

		ImGui::End();
	}

private:
	// properties
	OtLogo logo;
};
