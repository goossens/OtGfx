//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <memory>

#include "imgui.h"

#include "OtFramework.h"

#include "OtFilterDemo.h"
#include "OtGeneratorDemo.h"
#include "OtSplashScreen.h"

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
		// setup first demo
		demo = demos[currentDemo]();
	}

	void onRender() override {
		// see if we need to change demo based on user inputs
		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
			if (currentDemo == 0) {
				currentDemo = demos.size() - 1;

			} else {
				currentDemo--;
			}

			demo = demos[currentDemo]();

		} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
			currentDemo++;

			if (currentDemo == demos.size()) {
				currentDemo = 0;
			}

			demo = demos[currentDemo]();
		}

		// run the demo
		demo->run();
	}

	void onTerminate() override {
		demo = nullptr;
	}

private:

	// properties
	std::unique_ptr<OtDemo> demo;
	size_t currentDemo = 0;

	std::vector<std::function<std::unique_ptr<OtDemo>()>> demos {
		[]() { return std::make_unique<OtSplashScreen>(); },
		[]() { return std::make_unique<OtGeneratorDemo>(); },
		[]() { return std::make_unique<OtFilterDemo>(); }
	};
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
