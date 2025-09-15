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
#include <vector>

#include "SDL3/SDL.h"

#include "OtLog.h"

#include "OtRenderShader.h"
#include "OtVertex.h"


//
//	OtRenderPipeline
//

class OtRenderPipeline {
public:
	// property types
	enum class RenderTargetType {
		r8,
		rgba8,
		rgba32,
		rgba8d,
		rgba32d,
		gBuffer
	};

	enum TargetChannel {
		none = 0,
		r = 1 << 0,
		g = 1 << 1,
		b = 1 << 2,
		a = 1 << 3,
		z = 1 << 4,
		rgb = (r | g | b),
		rgba = (r | g | b | a),
		rgbaz = (r | g | b | a | z)
	};

	enum class DepthTest {
		none,
		less,
		lessEqual,
		greater,
		greaterEqual
	};

	enum class Culling {
		none,
		cw,
		ccw
	};

	enum class BlendOperation {
		none,
		alpha,
		add
	};

	// constructor
	OtRenderPipeline() = default;

	OtRenderPipeline(const uint32_t* vertexCode, size_t vertexSize, const uint32_t* fragmentCode, size_t fragmentSize) :
		vertexShaderCode(vertexCode),
		vertexShaderSize(vertexSize),
		fragmentShaderCode(fragmentCode),
		fragmentShaderSize(fragmentSize) {}

	// set pipeline properties
	inline void setShaders(const uint32_t* vertexCode, size_t vertexSize, const uint32_t* fragmentCode, size_t fragmentSize) {
		vertexShaderCode = vertexCode;
		vertexShaderSize = vertexSize;
		fragmentShaderCode = fragmentCode;
		fragmentShaderSize = fragmentSize;
		pipeline = nullptr;
	}

	inline void setVertexDescription(OtVertexDescription* description) { vertexDescription = description; }
	inline void setRenderTargetType(RenderTargetType value) { renderTargetType = value; }
	inline void setTargetChannel(TargetChannel value) { targetChannel = value; }
	inline void setDepthTest(DepthTest value) { depthTest = value; }
	inline void setCulling(Culling value) { culling = value; }
	inline void setBlendOperation(BlendOperation value) { blendOperation = value; }
	inline void setFill(bool mode) { fill = mode; }

	// clear the object
	inline void clear() {
		vertexShaderCode = nullptr;
		vertexShaderSize = 0;
		fragmentShaderCode = nullptr;
		fragmentShaderSize = 0;

		pipeline = nullptr;

		vertexDescription = nullptr;
		renderTargetType = RenderTargetType::rgba8d;
		targetChannel = TargetChannel::rgba;
		depthTest = DepthTest::less;
		culling = Culling::cw;
		blendOperation = BlendOperation::none;
		fill = true;
	}

	// see if pipeline is valid
	inline bool isValid() { return pipeline != nullptr; }

private:
	// shader definitions
	const uint32_t* vertexShaderCode = nullptr;
	size_t vertexShaderSize = 0;
	const uint32_t* fragmentShaderCode = nullptr;
	size_t fragmentShaderSize = 0;

	// the GPU resource
	std::shared_ptr<SDL_GPUGraphicsPipeline> pipeline;

	// memory manage SDL resource
	inline void assign(SDL_GPUGraphicsPipeline* newPipeline) {
		pipeline = std::shared_ptr<SDL_GPUGraphicsPipeline>(
			newPipeline,
			[](SDL_GPUGraphicsPipeline* oldPipeline) {
				SDL_ReleaseGPUGraphicsPipeline(OtGpu::instance().device, oldPipeline);
			});
	}

	// pipline properties
	OtVertexDescription* vertexDescription = nullptr;
	RenderTargetType renderTargetType = RenderTargetType::rgba8d;
	TargetChannel targetChannel = TargetChannel::rgba;
	DepthTest depthTest = DepthTest::less;
	Culling culling = Culling::cw;
	BlendOperation blendOperation = BlendOperation::none;
	bool fill = true;

	// get the raw pipeline object
	friend class OtRenderPass;

	inline SDL_GPUGraphicsPipeline* getPipeline() {
		// create pipeline (if required)
		if (!pipeline) {
			// ensure shaders are provided
			if (!vertexShaderCode || !vertexShaderSize || !vertexShaderCode || !vertexShaderSize) {
				OtLogFatal("Graphics pipeline is missing shaders");
			}

			// ensure vertex description is provided
			if (!vertexDescription) {
				OtLogFatal("Graphics pipeline is missing vertex description");
			}

			// create shaders
			OtRenderShader vertexShader{vertexShaderCode, vertexShaderSize, OtRenderShader::Stage::vertex};
			OtRenderShader fragmentShader{fragmentShaderCode, fragmentShaderSize, OtRenderShader::Stage::fragment};

			// setup information
			SDL_GPUVertexBufferDescription vertexBufferDescription{
				.slot = 0,
				.pitch = static_cast<Uint32>(vertexDescription->size),
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0
			};

			std::vector<SDL_GPUColorTargetDescription> targetDescriptions;

			if (renderTargetType == RenderTargetType::gBuffer) {
				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
					.blend_state = SDL_GPUColorTargetBlendState{
						.enable_blend = false,
						.enable_color_write_mask = false
					}
				});

				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
					.blend_state = SDL_GPUColorTargetBlendState{
						.enable_blend = false,
						.enable_color_write_mask = false
					}
				});

				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
					.blend_state = SDL_GPUColorTargetBlendState{
						.enable_blend = false,
						.enable_color_write_mask = false
					}
				});

				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
					.blend_state = SDL_GPUColorTargetBlendState{
						.enable_blend = false,
						.enable_color_write_mask = false
					}
				});

			} else {
				auto src = blendOperation == BlendOperation::alpha ? SDL_GPU_BLENDFACTOR_SRC_ALPHA : SDL_GPU_BLENDFACTOR_ONE;
				auto dst = blendOperation == BlendOperation::alpha ? SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA : SDL_GPU_BLENDFACTOR_ONE;

				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = getTextureFormat(),
					.blend_state = SDL_GPUColorTargetBlendState{
						.src_color_blendfactor = src,
						.dst_color_blendfactor = dst,
						.color_blend_op = SDL_GPU_BLENDOP_ADD,
						.src_alpha_blendfactor = src,
						.dst_alpha_blendfactor = dst,
						.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
						.color_write_mask = getTargetChannel(),
						.enable_blend = blendOperation != BlendOperation::none,
						.enable_color_write_mask = (targetChannel & TargetChannel::rgba) != TargetChannel::rgba
					}
				});
			}

			SDL_GPUGraphicsPipelineCreateInfo createInfo{
				.vertex_shader = vertexShader.getShader(),
				.fragment_shader = fragmentShader.getShader(),
				.vertex_input_state = SDL_GPUVertexInputState{
					.vertex_buffer_descriptions = &vertexBufferDescription,
					.num_vertex_buffers = 1,
					.vertex_attributes = vertexDescription->attributes,
					.num_vertex_attributes = static_cast<Uint32>(vertexDescription->members)
				},
				.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
				.rasterizer_state = SDL_GPURasterizerState{
					.fill_mode = fill ? SDL_GPU_FILLMODE_FILL : SDL_GPU_FILLMODE_LINE,
					.cull_mode = getCullingMode(),
					.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
					.depth_bias_constant_factor = 0.0f,
					.depth_bias_clamp = 0.0f,
					.depth_bias_slope_factor = 0.0f,
					.enable_depth_bias = false,
					.enable_depth_clip = false
				},
				.multisample_state = SDL_GPUMultisampleState{
					.sample_count = SDL_GPU_SAMPLECOUNT_1,
					.sample_mask = 0,
					.enable_mask = false
				},
				.depth_stencil_state = SDL_GPUDepthStencilState{
					.compare_op = getDepthTest(),
					.enable_depth_test = (depthTest != DepthTest::none),
					.enable_depth_write = (targetChannel & TargetChannel::z) != 0,
					.enable_stencil_test = false
				},
				.target_info = SDL_GPUGraphicsPipelineTargetInfo{
					.color_target_descriptions = targetDescriptions.data(),
					.num_color_targets = static_cast<Uint32>(targetDescriptions.size()),
					.depth_stencil_format = getDepthStencilFormat(),
					.has_depth_stencil_target = hasDepthStencilFormat()
				},
				.props = 0
			};

			// create graphics pipeline
			auto graphicsPipeline = SDL_CreateGPUGraphicsPipeline(OtGpu::instance().device, &createInfo);

			if (graphicsPipeline == nullptr) {
				OtLogFatal("Error in SDL_CreateGPUGraphicsPipeline: {}", SDL_GetError());
			}

			assign(graphicsPipeline);
		}

		// return the raw pipeline object
		return pipeline.get();
	}

	// translate properties
	SDL_GPUTextureFormat getTextureFormat() {
		switch (renderTargetType) {
			case RenderTargetType::r8: return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
			case RenderTargetType::rgba8: return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
			case RenderTargetType::rgba32: return SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
			case RenderTargetType::rgba8d: return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
			case RenderTargetType::rgba32d: return SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
			case RenderTargetType::gBuffer: return SDL_GPU_TEXTUREFORMAT_INVALID;
		}
	}

	bool hasDepthStencilFormat() {
		switch (renderTargetType) {
			case RenderTargetType::r8: return false;
			case RenderTargetType::rgba8: return false;
			case RenderTargetType::rgba32: return false;
			case RenderTargetType::rgba8d: return true;
			case RenderTargetType::rgba32d: return true;
			case RenderTargetType::gBuffer: return true;
		}
	}

	SDL_GPUTextureFormat getDepthStencilFormat() {
		switch (renderTargetType) {
			case RenderTargetType::r8: return SDL_GPU_TEXTUREFORMAT_INVALID;
			case RenderTargetType::rgba8: return SDL_GPU_TEXTUREFORMAT_INVALID;
			case RenderTargetType::rgba32: return SDL_GPU_TEXTUREFORMAT_INVALID;
			case RenderTargetType::rgba8d: return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
			case RenderTargetType::rgba32d: return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
			case RenderTargetType::gBuffer: return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
		}
	}

	SDL_GPUColorComponentFlags getTargetChannel() {
		SDL_GPUColorComponentFlags flags = 0;

		if (targetChannel & TargetChannel::r) {
			flags |= SDL_GPU_COLORCOMPONENT_R;
		}

		if (targetChannel & TargetChannel::g) {
			flags |= SDL_GPU_COLORCOMPONENT_G;
		}

		if (targetChannel & TargetChannel::b) {
			flags |= SDL_GPU_COLORCOMPONENT_B;
		}

		if (targetChannel & TargetChannel::a) {
			flags |= SDL_GPU_COLORCOMPONENT_A;
		}

		return flags;
	}

	SDL_GPUCompareOp getDepthTest() {
		switch (depthTest) {
			case DepthTest::none: return SDL_GPU_COMPAREOP_ALWAYS;
			case DepthTest::less: return SDL_GPU_COMPAREOP_LESS;
			case DepthTest::lessEqual: return SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
			case DepthTest::greater: return SDL_GPU_COMPAREOP_GREATER;
			case DepthTest::greaterEqual: return SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;
		}
	}

	SDL_GPUCullMode getCullingMode() {
		switch (culling) {
			case Culling::none: return SDL_GPU_CULLMODE_NONE;
			case Culling::cw: return SDL_GPU_CULLMODE_BACK;
			case Culling::ccw: return SDL_GPU_CULLMODE_FRONT;
		}
	}
};
