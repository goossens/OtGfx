#include "glm/glm.hpp"

#include "OtBackgroundPass.h"


//
//	OtBackgroundPass::render
//

void OtBackgroundPass::render(OtSceneRendererContext& ctx) {
	// determine background color
	glm::vec3 backgroundColor{0.0f};

	for (auto&& [entity, component] : ctx.scene->view<OtBackgroundComponent>().each()) {
		backgroundColor = component.color;
	}

	// fill framebuffer with background color;
	flood.setColor(backgroundColor);
	flood.render(framebuffer.getColorTexture());
}