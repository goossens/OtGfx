//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtFlood.h"
#include "OtFrameBuffer.h"

#include "OtSceneRendererContext.h"


//
//	OtBackgroundPass
//

class OtBackgroundPass {
public:
	// constructor
	OtBackgroundPass(OtFrameBuffer& fb) : framebuffer(fb) {}

	// render the pass
	inline void render(OtSceneRendererContext& ctx) {
		// determine background color
		glm::vec3 backgroundColor{0.0f};

		for (auto&& [entity, component] : ctx.scene->view<OtBackgroundComponent>().each()) {
			backgroundColor = component.color;
		}

		// fill framebuffer with background color;
		flood.setColor(backgroundColor);
		flood.render(framebuffer.getColorTexture());
	}

private:
	// properties
	OtFrameBuffer& framebuffer;
	OtFlood flood;
};
