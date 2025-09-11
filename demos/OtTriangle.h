//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include "OtFrameBuffer.h"
#include "OtIndexBuffer.h"
#include "OtRenderPass.h"
#include "OtRenderPipeline.h"
#include "OtVertexBuffer.h"

#include "OtDemo.h"
#include "OtTriangleVert.h"
#include "OtTriangleFrag.h"


//
//	OtTriangle
//

class OtTriangle : public OtDemo {
public:
	// constructor
	OtTriangle() {
		// initialize vertex buffer
		static OtVertexPosColor verts[] = {
	    	{glm::vec3(0.0f, 0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
    		{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},
    		{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)}
		};

		vertices.set(verts, sizeof(verts) / sizeof(verts[0]), OtVertexPosColor::getDescription());

		// configure rendering pipeline
		pipeline.setShaders(OtTriangleVert, sizeof(OtTriangleVert), OtTriangleFrag, sizeof(OtTriangleFrag));
		pipeline.setVertexDescription(OtVertexPosColor::getDescription());
		pipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba8);
		pipeline.setDepthTest(OtRenderPipeline::DepthTest::none);
		pipeline.setCulling(OtRenderPipeline::Culling::none);
	}

	// run demo
	inline void run() override {
		// render frame
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
		ImGui::Begin("Triangle", nullptr, ImGuiWindowFlags_NoDecoration);
		auto size = ImGui::GetContentRegionAvail();

		// update buffer (if required)
		framebuffer.update(size.x, size.y);

		// render triangle to framebuffer
		OtRenderPass pass;
		pass.start(framebuffer);
		float time = static_cast<float>(ImGui::GetTime());
		pass.setFragmentUniforms(0, &time, sizeof(time));
		pass.bindPipeline(pipeline);
		pass.render(vertices);
		pass.end();

		// put it on the screen
		ImGui::Image(framebuffer.getColorTextureID(), size);
		ImGui::End();
	}

private:
	// properties
	OtVertexBuffer vertices;
	OtRenderPipeline pipeline;
	OtFrameBuffer framebuffer{OtTexture::Format::rgba8};
};
