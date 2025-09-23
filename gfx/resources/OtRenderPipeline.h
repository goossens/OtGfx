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
		never,
		less,
		equal,
		lessEqual,
		greater,
		notEqual,
		greaterEqual,
		always
	};

	enum class Culling {
		none,
		cw,
		ccw
	};

	enum class BlendOperation {
		none,
		add,
		subtract,
		reverseSubtract,
		min,
		max
	};

	enum class BlendFactor {
		none,
		zero,
		one,
		srcColor,
		oneMinusSrcColor,
		dstColor,
		oneMinusDstColor,
		srcAlpha,
		oneMinusSrcAlpha,
		dstAlpha,
		oneMinusDstAlpha,
		constantColor,
		oneMinusConstantColor,
		alphaSaturate
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
	inline void setFill(bool mode) { fill = mode; }

	inline void setBlend(BlendOperation operation, BlendFactor src, BlendFactor dst) {
		colorBlendOperation = operation;
		colorSrcFactor = src;
		colorDstFactor = dst;
		alphaBlendOperation = operation;
		alphaSrcFactor = src;
		alphaDstFactor = dst;
	}

	inline void setColorBlend(BlendOperation operation, BlendFactor src, BlendFactor dst) {
		colorBlendOperation = operation;
		colorSrcFactor = src;
		colorDstFactor = dst;
	}

	inline void setAlphaBlend(BlendOperation operation, BlendFactor src, BlendFactor dst) {
		alphaBlendOperation = operation;
		alphaSrcFactor = src;
		alphaDstFactor = dst;
	}

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
		fill = true;

		colorBlendOperation = BlendOperation::none;
		alphaBlendOperation = BlendOperation::none;
		colorSrcFactor = BlendFactor::none;
		colorDstFactor = BlendFactor::none;
		alphaSrcFactor = BlendFactor::none;
		alphaDstFactor = BlendFactor::none;
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

	// pipeline properties
	OtVertexDescription* vertexDescription = nullptr;
	RenderTargetType renderTargetType = RenderTargetType::rgba8d;
	TargetChannel targetChannel = TargetChannel::rgba;
	DepthTest depthTest = DepthTest::less;
	Culling culling = Culling::cw;
	bool fill = true;

	BlendOperation colorBlendOperation = BlendOperation::none;
	BlendOperation alphaBlendOperation = BlendOperation::none;
	BlendFactor colorSrcFactor = BlendFactor::none;
	BlendFactor colorDstFactor = BlendFactor::none;
	BlendFactor alphaSrcFactor = BlendFactor::none;
	BlendFactor alphaDstFactor = BlendFactor::none;

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
				SDL_GPUColorTargetBlendState blendState;
				blendState.enable_blend = false;
				blendState.enable_color_write_mask = false;

				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
					.blend_state = blendState
				});

				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
					.blend_state = blendState
				});

				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
					.blend_state = blendState
				});

				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
					.blend_state = blendState
				});

			} else {
				targetDescriptions.emplace_back(SDL_GPUColorTargetDescription{
					.format = getTextureFormat(),
					.blend_state = SDL_GPUColorTargetBlendState{
						.src_color_blendfactor = getBlendFactor(colorSrcFactor),
						.dst_color_blendfactor = getBlendFactor(colorDstFactor),
						.color_blend_op = getBlendOperation(colorBlendOperation),
						.src_alpha_blendfactor = getBlendFactor(alphaSrcFactor),
						.dst_alpha_blendfactor = getBlendFactor(alphaDstFactor),
						.alpha_blend_op = getBlendOperation(alphaBlendOperation),
						.color_write_mask = getTargetChannel(),
						.enable_blend = colorBlendOperation != BlendOperation::none || alphaBlendOperation != BlendOperation::none,
						.enable_color_write_mask = (targetChannel & TargetChannel::rgba) != TargetChannel::rgba,
						.padding1 = 0,
						.padding2 = 0
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
					.enable_depth_clip = false,
					.padding1 = 0,
					.padding2 = 0
				},
				.multisample_state = SDL_GPUMultisampleState{
					.sample_count = SDL_GPU_SAMPLECOUNT_1,
					.sample_mask = 0,
					.enable_mask = false,
					.padding1 = 0,
					.padding2 = 0,
					.padding3 = 0
				},
				.depth_stencil_state = SDL_GPUDepthStencilState{
					.compare_op = getDepthTest(),
					.back_stencil_state = SDL_GPUStencilOpState{},
					.front_stencil_state = SDL_GPUStencilOpState{},
					.compare_mask = 0,
					.write_mask = 0,
					.enable_depth_test = (depthTest != DepthTest::none),
					.enable_depth_write = (targetChannel & TargetChannel::z) != 0,
					.enable_stencil_test = false,
					.padding1 = 0,
					.padding2 = 0,
					.padding3 = 0
				},
				.target_info = SDL_GPUGraphicsPipelineTargetInfo{
					.color_target_descriptions = targetDescriptions.data(),
					.num_color_targets = static_cast<Uint32>(targetDescriptions.size()),
					.depth_stencil_format = getDepthStencilFormat(),
					.has_depth_stencil_target = hasDepthStencilFormat(),
					.padding1 = 0,
					.padding2 = 0,
					.padding3 = 0
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

		return SDL_GPU_TEXTUREFORMAT_INVALID;
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

		return false;
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

		return SDL_GPU_TEXTUREFORMAT_INVALID;
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
			case DepthTest::none: return SDL_GPU_COMPAREOP_INVALID;
			case DepthTest::never: return SDL_GPU_COMPAREOP_NEVER;
			case DepthTest::less: return SDL_GPU_COMPAREOP_LESS;
			case DepthTest::equal: return SDL_GPU_COMPAREOP_EQUAL;
			case DepthTest::lessEqual: return SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
			case DepthTest::greater: return SDL_GPU_COMPAREOP_GREATER;
			case DepthTest::notEqual: return SDL_GPU_COMPAREOP_NOT_EQUAL;
			case DepthTest::greaterEqual: return SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;
			case DepthTest::always: return SDL_GPU_COMPAREOP_ALWAYS;
		}

		return SDL_GPU_COMPAREOP_INVALID;
	}

	SDL_GPUCullMode getCullingMode() {
		switch (culling) {
			case Culling::none: return SDL_GPU_CULLMODE_NONE;
			case Culling::cw: return SDL_GPU_CULLMODE_BACK;
			case Culling::ccw: return SDL_GPU_CULLMODE_FRONT;
		}

		return SDL_GPU_CULLMODE_NONE;
	}

	SDL_GPUBlendOp getBlendOperation(BlendOperation operation) {
		switch (operation) {
			case BlendOperation::none: return SDL_GPU_BLENDOP_INVALID;
			case BlendOperation::add: return SDL_GPU_BLENDOP_ADD;
			case BlendOperation::subtract: return SDL_GPU_BLENDOP_SUBTRACT;
			case BlendOperation::reverseSubtract: return SDL_GPU_BLENDOP_REVERSE_SUBTRACT;
			case BlendOperation::min: return SDL_GPU_BLENDOP_MIN;
			case BlendOperation::max: return SDL_GPU_BLENDOP_MAX;
		}

		return SDL_GPU_BLENDOP_INVALID;
	}

	SDL_GPUBlendFactor getBlendFactor(BlendFactor factor) {
		switch (factor) {
			case BlendFactor::none: return SDL_GPU_BLENDFACTOR_INVALID;
			case BlendFactor::zero: return SDL_GPU_BLENDFACTOR_ZERO;
			case BlendFactor::one: return SDL_GPU_BLENDFACTOR_ONE;
			case BlendFactor::srcColor: return SDL_GPU_BLENDFACTOR_SRC_COLOR;
			case BlendFactor::oneMinusSrcColor: return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
			case BlendFactor::dstColor: return SDL_GPU_BLENDFACTOR_DST_COLOR;
			case BlendFactor::oneMinusDstColor: return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR;
			case BlendFactor::srcAlpha: return SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			case BlendFactor::oneMinusSrcAlpha: return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			case BlendFactor::dstAlpha: return SDL_GPU_BLENDFACTOR_DST_ALPHA;
			case BlendFactor::oneMinusDstAlpha: return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
			case BlendFactor::constantColor: return SDL_GPU_BLENDFACTOR_CONSTANT_COLOR;
			case BlendFactor::oneMinusConstantColor: return SDL_GPU_BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR;
			case BlendFactor::alphaSaturate: return SDL_GPU_BLENDFACTOR_SRC_ALPHA_SATURATE;
		}

		return SDL_GPU_BLENDFACTOR_INVALID;
	}
};
