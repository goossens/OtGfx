//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <cstdint>
#include <vector>

#include "glm/glm.hpp"
#include "SDL3/SDL.h"

#include "OtCubeMap.h"
#include "OtFrameBuffer.h"
#include "OtGbuffer.h"
#include "OtGeometry.h"
#include "OtIndexBuffer.h"
#include "OtMesh.h"
#include "OtRenderPipeline.h"
#include "OtSampler.h"
#include "OtTexture.h"
#include "OtVertexBuffer.h"


//
//	OtRenderPass
//

class OtRenderPass {
public:
	// destructor
	~OtRenderPass();

	// start a render pass
	void start(OtTexture& texture, bool clear);
	void start(OtFrameBuffer& framebuffer);
	void start(OtGbuffer& gbuffer);

	// bind a render pipeline
	void bindPipeline(OtRenderPipeline& pipeline);

	// bind sampler
	void bindVertexSampler(size_t slot, OtSampler& sampler, SDL_GPUTexture* texture);
	void bindVertexSampler(size_t slot, OtSampler& sampler, OtTexture& texture);
	void bindVertexSampler(size_t slot, OtSampler& sampler, OtCubeMap& cubemap);

	void bindFragmentSampler(size_t slot, OtSampler& sampler, SDL_GPUTexture* texture);
	void bindFragmentSampler(size_t slot, OtSampler& sampler, OtTexture& texture);
	void bindFragmentSampler(size_t slot, OtSampler& sampler, OtCubeMap& cubemap);

	// set uniforms
	void setVertexUniforms(size_t slot, const void* data, size_t size);
	void setFragmentUniforms(size_t slot, const void* data, size_t size);

	// set the stencil reference
	void setStencilReference(uint8_t reference);

	// execute a rendering command
	void render(size_t vertices, size_t instances=1);
	void render(OtVertexBuffer& buffer);
	void render(OtVertexBuffer& vertexBuffer, OtIndexBuffer& indexBuffer, size_t offset=0, size_t count=0);
	inline void render(OtMesh& mesh) { render(mesh.getVertexBuffer(), mesh.getIndexBuffer()); }
	inline void render(OtGeometry& geometry) { render(geometry.getMesh()); }

	// end a render pass
	void end();

private:
	// the GPU resource
	SDL_GPURenderPass* pass;

	// state
	bool open = false;
};
