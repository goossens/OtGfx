//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"

#include "OtFunction.h"
#include "OtLog.h"

#include "OtBlitPass.h"
#include "OtUi.h"
#include "OtVertex.h"

#include "OtDial.h"


//
//	OtDialClass::init
//

void OtDialClass::init(size_t count, OtObject* parameters) {
	switch (count) {
		case 4:
			setVerticalAlignment(OtUi::Alignment(parameters[3]->operator int()));
			[[fallthrough]];

		case 3:
			setHorizontalAlignment(OtUi::Alignment(parameters[2]->operator int()));
			[[fallthrough]];

		case 2:
			setNeedle(parameters[1]->operator std::string());
			[[fallthrough]];

		case 1:
			setBackground(parameters[0]->operator std::string());
			[[fallthrough]];

		case 0:
			break;

		default:
		OtLogError("[Dial] constructor expects up to 4 arguments (not {})", count);
	}
}


//
//	OtDialClass::render
//

void OtDialClass::render() {
	// sanity check
	if (background.isNull()) {
		OtLogError("[Dial] does not have a background");
	}

	// wait until background is ready
	if (background.isReady()) {
		// do we need to redraw widget
		if (redraw) {
			// determine framebuffer dimensions
			auto& backgroundTexture = background->getTexture();
			auto w = backgroundTexture.getWidth();
			auto h = backgroundTexture.getHeight();
			output.update(w, h, OtTexture::Format::rgba8, OtTexture::Usage::rwDefault);

			// render needle (if required)
			if (needle.isReady()) {
				// determine needle rotation
				auto ratio = (value - minValue) / (maxValue - minValue);
				auto rotation = minRotation + ratio * (maxRotation - minRotation);

				// determine transformation
				glm::mat4 transform{1.0f};
				transform = glm::translate(transform, glm::vec3(bx, by, 0.0f));
				transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	 			transform = glm::translate(transform, glm::vec3(-nx, -ny, 0.0f));

				// render the needle
				needleFilter.setTransform(transform);
				needleFilter.setNeedle(needle->getTexture());
				needleFilter.render(background->getTexture(), output);

				// reset flag
				redraw = false;

			} else {
				OtBlitPass::blit(background->getTexture(), output);
			}
		}

		auto size = ImVec2(output.getWidth() * scale, output.getHeight() * scale);
		OtUi::align(size, horizontalAlign, verticalAlign);
		ImGui::Image(output.getTextureID(), size);
	}
}


//
//	OtDialClass::getMeta
//

OtType OtDialClass::getMeta() {
	static OtType type;

	if (!type) {
		type = OtType::create<OtDialClass>("Dial", OtWidgetClass::getMeta());
		type->set("__init__", OtFunction::create(&OtDialClass::init));
		type->set("setBackground", OtFunction::create(&OtDialClass::setBackground));
		type->set("setNeedle", OtFunction::create(&OtDialClass::setNeedle));
		type->set("setValue", OtFunction::create(&OtDialClass::setValue));
		type->set("getValue", OtFunction::create(&OtDialClass::getValue));
		type->set("setValueLimits", OtFunction::create(&OtDialClass::setValueLimits));
		type->set("setRotationLimits", OtFunction::create(&OtDialClass::setRotationLimits));
		type->set("setScale", OtFunction::create(&OtDialClass::setScale));
		type->set("setHorizontalAlignment", OtFunction::create(&OtDialClass::setHorizontalAlignment));
		type->set("setVerticalAlignment", OtFunction::create(&OtDialClass::setVerticalAlignment));
	}

	return type;
}
