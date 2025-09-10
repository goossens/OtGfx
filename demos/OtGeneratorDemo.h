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

#include "OtTexture.h"

#include "OtCheckerBoard.h"
#include "OtColorWheel.h"
#include "OtFbm.h"
#include "OtTileableFbm.h"

#include "OtDemo.h"


//
//	OtGeneratorDemo
//

class OtGeneratorDemo : public OtDemo {
public:
	// run demo
	inline void run() override {
		// track changes
		bool changed = false;

		// initialize (if required)
		if (!generator) {
			generator = generators[currentGenerator]();
			changed = true;

		// handle user interactions
		} else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
			if (currentGenerator == 0) {
				currentGenerator = generators.size() - 1;

			} else {
				currentGenerator--;
			}

			generator = generators[currentGenerator]();
			changed = true;

		} else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
			currentGenerator++;

			if (currentGenerator == generators.size()) {
				currentGenerator = 0;
			}

			generator = generators[currentGenerator]();
			changed = true;
		}

		// render frame
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
		ImGui::Begin("Generate", nullptr, ImGuiWindowFlags_NoDecoration);
		auto size = ImGui::GetContentRegionAvail();
		auto usage = OtTexture::Usage(OtTexture::Usage::sampler | OtTexture::Usage::computeStorageWrite);

		// regenerate texture if required
		if (texture.update(size.x, size.y, OtTexture::Format::rgba8, usage) || changed) {
			generator->render(texture);
		}

		ImGui::Image(texture.getTextureID(), size);
		ImGui::End();
	}

private:
	// properties
	OtTexture texture;

	std::unique_ptr<OtGenerator> generator;
	size_t currentGenerator = 0;

	std::vector<std::function<std::unique_ptr<OtGenerator>()>> generators {
		[]() { return std::make_unique<OtCheckerBoard>(); },
		[]() { return std::make_unique<OtColorWheel>(); },
		[]() { return std::make_unique<OtFbm>(); },
		[]() { return std::make_unique<OtTileableFbm>(); }
	};
};
