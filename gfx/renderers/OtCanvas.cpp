//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include "OtException.h"
#include "OtFunction.h"
#include "OtLog.h"
#include "OtPath.h"

#include "OtCanvas.h"
#include "OtImage.h"

#include "OtCanvasFrag.h"
#include "OtCanvasVert.h"


//
//	OtCanvas::OtCanvas
//

OtCanvas::OtCanvas() {
	// initialize canvas renderer
	NVGparams params;

	params.renderCreate = [](void* ptr) { return ((OtCanvas*) ptr)->renderCreate(); };
	params.renderCreateTexture = [](void* ptr, int type, int w, int h, int imageFlags, const unsigned char* data) { return ((OtCanvas*) ptr)->renderCreateTexture(type, w, h, imageFlags, data); };
	params.renderDeleteTexture = [](void* ptr, int texture) { return ((OtCanvas*) ptr)->renderDeleteTexture(texture); };
	params.renderUpdateTexture = [](void* ptr, int texture, int x, int y, int w, int h, const unsigned char* data) { return ((OtCanvas*) ptr)->renderUpdateTexture(texture, x, y, w, h, data); };
	params.renderGetTextureSize = [](void* ptr, int texture, int* w, int* h) { return ((OtCanvas*) ptr)->renderGetTextureSize(texture, w, h); };
	params.renderViewport = [](void*, float, float, float) {};
	params.renderCancel = [](void*) {};
	params.renderFlush = [](void*) {};
	params.renderFill = [](void* ptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths) { ((OtCanvas*) ptr)->renderFill(paint, compositeOperation, scissor, fringe, bounds, paths, npaths); };
	params.renderStroke = [](void* ptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths) { ((OtCanvas*) ptr)->renderStroke(paint, compositeOperation, scissor, fringe, strokeWidth, paths, npaths); };
	params.renderTriangles = [](void* ptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, const NVGvertex* verts, int nverts, float fringe) { ((OtCanvas*) ptr)->renderTriangles(paint, compositeOperation, scissor, verts, nverts, fringe); };
	params.renderDelete = [](void*) {};
	params.userPtr = this;
	params.edgeAntiAlias = 1;
	context = nvgCreateInternal(&params);

	if (!context) {
		OtLogFatal("Internal error: can't create a new canvas context");
	}

	// configure rendering pipelines
	pipeline.setShaders(OtCanvasVert, sizeof(OtCanvasVert), OtCanvasFrag, sizeof(OtCanvasFrag));
	pipeline.setVertexDescription(OtVertexPosUv2D::getDescription());
	pipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba8d24s8);
	pipeline.setTargetChannels(OtRenderPipeline::TargetChannels::rgba);
	pipeline.setDepthTest(OtRenderPipeline::CompareOperation::none);
	pipeline.setCulling(OtRenderPipeline::Culling::none);

	pipeline.setBlend(
		OtRenderPipeline::BlendOperation::add,
		OtRenderPipeline::BlendFactor::srcAlpha,
		OtRenderPipeline::BlendFactor::oneMinusSrcAlpha
	);

	shapesPipeline.setShaders(OtCanvasVert, sizeof(OtCanvasVert), OtCanvasFrag, sizeof(OtCanvasFrag));
	shapesPipeline.setVertexDescription(OtVertexPosUv2D::getDescription());
	shapesPipeline.setRenderTargetType(OtRenderPipeline::RenderTargetType::rgba8d24s8);
	shapesPipeline.setTargetChannels(OtRenderPipeline::TargetChannels::z);
	shapesPipeline.setDepthTest(OtRenderPipeline::CompareOperation::always);
	shapesPipeline.setCulling(OtRenderPipeline::Culling::none);

	shapesPipeline.setStencil(
		0, 0xff,
		OtRenderPipeline::CompareOperation::always,
		OtRenderPipeline::StencilOperation::incrementAndWrap,
		OtRenderPipeline::StencilOperation::none,
		OtRenderPipeline::StencilOperation::none,
		OtRenderPipeline::CompareOperation::always,
		OtRenderPipeline::StencilOperation::decrementAndWrap,
		OtRenderPipeline::StencilOperation::none,
		OtRenderPipeline::StencilOperation::none);
}


//
//	OtCanvas::~OtCanvas
//

OtCanvas::~OtCanvas() {
	nvgDeleteInternal(context);
}


//
//	OtCanvas::loadImage
//

int OtCanvas::loadImage(const std::string &path, int flags) {
	OtImage image{path};
	return nvgCreateImageRGBA(context, image.getWidth(), image.getHeight(), flags, (const unsigned char *) image.getPixels());
}


//
//	OtCanvas::createLinearGradient
//

int OtCanvas::createLinearGradient(float sx, float sy, float ex, float ey, const std::string& startColor, const std::string& endColor) {
	auto sc = OtColorParser::toVec4(startColor);
	auto ec = OtColorParser::toVec4(endColor);
	auto paint = nvgLinearGradient(context, sx, sy, ex, ey, nvgRGBAf(sc.r, sc.g, sc.b, sc.a), nvgRGBAf(ec.r, ec.g, ec.b, ec.a));
	return addPaint(paint);
}


//
//	OtCanvas::createBoxGradient
//

int OtCanvas::createBoxGradient(float x, float y, float w, float h, float r, float f, const std::string& startColor, const std::string& endColor) {
	auto sc = OtColorParser::toVec4(startColor);
	auto ec = OtColorParser::toVec4(endColor);
	auto paint = nvgBoxGradient(context, x, y, w, h, r, f, nvgRGBAf(sc.r, sc.g, sc.b, sc.a), nvgRGBAf(ec.r, ec.g, ec.b, ec.a));
	return addPaint(paint);
}


//
//	OtCanvas::createRadialGradient
//

int OtCanvas::createRadialGradient(float cx, float cy, float inner, float outer, const std::string& startColor, const std::string& endColor) {
	auto sc = OtColorParser::toVec4(startColor);
	auto ec = OtColorParser::toVec4(endColor);
	auto paint = nvgRadialGradient(context, cx, cy, inner, outer, nvgRGBAf(sc.r, sc.g, sc.b, sc.a), nvgRGBAf(ec.r, ec.g, ec.b, ec.a));
	return addPaint(paint);
}


//
//	OtCanvas::deletePaint
//

void OtCanvas::deletePaint(int id) {
	auto entry = paints.find(id);

	if (entry != paints.end()) {
		paints.erase(entry);
	}
}


//
//	OtCanvas::loadFont
//

int OtCanvas::loadFont(const std::string& path) {
	auto id =  nvgCreateFont(context, OtPath::getStem(path).c_str(), path.c_str());

	if (id < 0) {
		OtLogError("Can't load font at [{}]", path);
	}

	return id;
}


//
//	createTexturePattern
//

int OtCanvas::createTexturePattern(float sx, float sy, float ex, float ey, float angle, int texture, float alpha) {
	auto paint = nvgImagePattern(context, sx, sy, ex, ey, angle, texture, alpha);
	return addPaint(paint);
}


//
//	OtCanvas::drawImage
//

void OtCanvas::drawImage(int image, float x, float y) {
	int w, h;
	nvgImageSize(context, image, &w, &h);
	drawImage(image, 0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h), x, y, static_cast<float>(w), static_cast<float>(h));
}

void OtCanvas::drawImage(int image, float x, float y, float w, float h) {
	int sw, sh;
	nvgImageSize(context, image, &sw, &sh);
	drawImage(image, 0.0f, 0.0f, static_cast<float>(sw), static_cast<float>(sh), x, y, w, h);
}

void OtCanvas::drawImage(int image, float sx, float sy, float sw, float sh, float x, float y, float w, float h) {
	int iw, ih;
	nvgImageSize(context, image, &iw, &ih);

	float ax = w / sw;
	float ay = h / sh;
	NVGpaint imgPaint = nvgImagePattern(context, x - sx * ax, y - sy * ay, static_cast<float>(iw) * ax, static_cast<float>(ih) * ay, 0.0f, image, 1.0f);

	nvgBeginPath(context);
	nvgRect(context, x, y, w, h);
	nvgFillPaint(context, imgPaint);
	nvgFill(context);
}


//
//	OtCanvas::textSize
//

glm::vec2 OtCanvas::textSize(const std::string& string) {
	float bounds[4];
	nvgTextBounds(context, 0.0f, 0.0f, string.c_str(), nullptr, bounds);
	return glm::vec2(bounds[2] - bounds[0], bounds[3] - bounds[1]);
}


//
//	OtCanvas::textBoxSize
//

glm::vec2 OtCanvas::textBoxSize(const std::string& string, float w) {
	float bounds[4];
	nvgTextBoxBounds(context, 0.0f, 0.0f, w, string.c_str(), nullptr, bounds);
	return glm::vec2(bounds[2] - bounds[0], bounds[3] - bounds[1]);
}


//
//	OtCanvas::render
//

void OtCanvas::render(OtFrameBuffer& framebuffer, float scale, std::function<void()> renderer) {
	// get dimensions
	width = static_cast<float>(framebuffer.getWidth());
	height = static_cast<float>(framebuffer.getHeight());

	// render the canvas
	nvgBeginFrame(context, width, height, 1.0f);
	nvgScale(context, scale, scale);

	try {
		renderer();

	} catch (OtException& e) {
		nvgEndFrame(context);
		throw(e);
	}

	// start rendering pass
	OtRenderPass pass;
	pass.start(framebuffer);
	pass.bindPipeline(pipeline);

	// ignore empty canvases
	if (calls.size()) {
		// update vertex and index buffers
		vertexBuffer.set(vertices.data(), vertices.size(), OtVertexPosUv2D::getDescription(), true);
		indexBuffer.set(indices.data(), indices.size(), true);

		// render all calls
		for (auto& call : calls) {
			// set call specific uniforms
			setUniforms(pass, call);

			switch (call.type) {
				case CallType::none:
					break;

				case CallType::fill:
					if (call.shapeCount) {
						pass.render(vertexBuffer, indexBuffer, call.shapeOffset, call.shapeCount);
					}

					break;

				case CallType::convexFill:
					if (call.shapeCount) {
						pass.render(vertexBuffer, indexBuffer, call.shapeOffset, call.shapeCount);
					}

					if (call.strokeCount) {
						pass.render(vertexBuffer, indexBuffer, call.strokeOffset, call.strokeCount);
					}

					break;

				case CallType::stroke:
					if (call.strokeCount) {
						pass.render(vertexBuffer, indexBuffer, call.strokeOffset, call.strokeCount);
					}

					break;

				case CallType::triangles:
					if (call.fillCount) {
						pass.render(vertexBuffer, indexBuffer, call.fillOffset, call.fillCount);
					}

					break;
			}
		}
	}

	pass.end();

	// cleanup
	nvgEndFrame(context);
	calls.clear();
	vertices.clear();
	indices.clear();
	fragmentUniforms.clear();
}


//
//	OtCanvas::addPaint
//

int OtCanvas::addPaint(const NVGpaint& paint) {
	auto id = paintID++;
	paints[id] = paint;
	return id;
}


//
//	OtCanvas::renderCreate
//

int OtCanvas::renderCreate() {
	return 1;
}


//
//	OtCanvas::renderCreateTexture
//

int OtCanvas::renderCreateTexture(int type, int w, int h, int imageFlags, const unsigned char* data) {
	// create the texture record
	auto [it, valid] = textures.emplace(nextTextureID++, Texture{});
	auto id = it->first;
	auto& texture = it->second.texture;
	auto& sampler = it->second.sampler;

	// update the texture
	it->second.flip = imageFlags & NVG_IMAGE_FLIPY;
	auto format = (type == NVG_TEXTURE_RGBA) ? OtTexture::Format::rgba8 : OtTexture::Format::r8;

	if (format == OtTexture::Format::r8) {
		id = id++;
		id = id--;
	}

	if (data) {
		texture.load(w, h, format, (void*) data);

	} else {
		texture.update(w, h, format, OtTexture::Usage::readAll);
	}

	// update the sampler
	sampler.setFilter(imageFlags & NVG_IMAGE_NEAREST ? OtSampler::Filter::nearest : OtSampler::Filter::linear);
	sampler.setAddressingX(imageFlags & NVG_IMAGE_REPEATX ? OtSampler::Addressing::repeat : OtSampler::Addressing::clamp);
	sampler.setAddressingY(imageFlags & NVG_IMAGE_REPEATY ? OtSampler::Addressing::repeat : OtSampler::Addressing::clamp);

	// return texture id
	return id;
}


//
//	OtCanvas::renderDeleteTexture
//

int OtCanvas::renderDeleteTexture(int textureID) {
	// find and delete texture
	auto entry = textures.find(textureID);

	if (entry != textures.end()) {
		textures.erase(textureID);
	}

	return 1;
}


//
//	OtCanvas::renderUpdateTexture
//

int OtCanvas::renderUpdateTexture(int textureID, int x, int y, int w, int h, const unsigned char* data) {
	// find texture
	auto entry = textures.find(textureID);

	if (entry == textures.end()) {
		OtLogFatal("Can't find canvas texture with id {}", textureID);
	}

	// extract update region (since data points to complete texture)
	auto& texture = entry->second.texture;
	auto bpp = texture.getBpp();

	auto srcPitch = texture.getWidth() * bpp;
	auto src = data + y * srcPitch + x * bpp;

	auto dstPitch = w * bpp;
	unsigned char* buffer = new unsigned char[h * dstPitch];
	auto dst = buffer;

	for (auto i = 0; i < h; i++) {
		std::memcpy(dst, src, dstPitch);
		dst += dstPitch;
		src += srcPitch;
	}

	// update the texture
	texture.update(x, y, w, h, (void*) buffer);

	// cleanup
	delete [] buffer;
	return 1;
}


//
//	OtCanvas::renderGetTextureSize
//

int OtCanvas::renderGetTextureSize(int textureID, int* w, int* h) {
	// find texture
	auto entry = textures.find(textureID);

	if (entry == textures.end()) {
		OtLogFatal("Can't find canvas texture with id {}", textureID);
	}

	// get the size
	auto& texture = entry->second.texture;
	*w = texture.getWidth();
	*h = texture.getHeight();
	return 1;
}


//
//	OtCanvas::renderFill
//

void OtCanvas::renderFill(NVGpaint* paint, NVGcompositeOperationState, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths) {
	// setup new rendering call
	auto& call = calls.emplace_back(Call{});
	call.type = (npaths == 1 && paths[0].convex) ? CallType::convexFill : CallType::fill;
	call.image = paint->image;
	call.shapeOffset = indices.size();

	// copy vertex and index data for shapes
	for (int j = 0; j < npaths; j++) {
		auto& path =  paths[j];

		if (path.nfill > 2) {
			auto p = &path.fill[0];
			size_t vertOffset = vertices.size();
			size_t hubVertOffset = vertOffset++;

			for (int i = 0; i < path.nfill; i++, p++) {
				vertices.emplace_back(glm::vec2(p->x, p->y), glm::vec2(p->u, p->v));
			}

			for (int i = 2; i < path.nfill; i++) {
				indices.emplace_back(hubVertOffset);
				indices.emplace_back(vertOffset++);
				indices.emplace_back(vertOffset);
			}
		}
	}

	call.shapeCount = indices.size() - call.shapeOffset;
	call.strokeOffset = indices.size();

	// process strokes
	for (int p = 0; p < npaths; p++) {
		auto& path =  paths[p];

		if (path.nstroke > 0) {
			auto p = &path.stroke[0];
			size_t vertOffset = vertices.size();

			for (int i = 0; i < path.nstroke; i++, p++) {
				vertices.emplace_back(glm::vec2(p->x, p->y), glm::vec2(p->u, p->v));
			}

			for (int i = 2; i < path.nstroke; i++, p++) {
				if (i & 1) {
					indices.emplace_back(vertOffset);
					indices.emplace_back(vertOffset + 1);
					indices.emplace_back(vertOffset + 2);

				} else {
					indices.emplace_back(vertOffset);
					indices.emplace_back(vertOffset + 2);
					indices.emplace_back(vertOffset + 1);
				}

				vertOffset++;
			}
		}
	}

	call.strokeCount = indices.size() - call.strokeOffset;

	// handle fills
	if (call.type == CallType::fill) {
		call.fillOffset = indices.size();
		vertices.emplace_back(glm::vec2(bounds[0], bounds[1]), glm::vec2(0.5f, 1.0f));
		vertices.emplace_back(glm::vec2(bounds[0], bounds[3]), glm::vec2(0.5f, 1.0f));
		vertices.emplace_back(glm::vec2(bounds[2], bounds[3]), glm::vec2(0.5f, 1.0f));
		vertices.emplace_back(glm::vec2(bounds[2], bounds[1]), glm::vec2(0.5f, 1.0f));

		indices.emplace_back(call.fillOffset);
		indices.emplace_back(call.fillOffset + 1);
		indices.emplace_back(call.fillOffset + 2);

		indices.emplace_back(call.fillOffset + 2);
		indices.emplace_back(call.fillOffset + 3);
		indices.emplace_back(call.fillOffset);
		call.fillCount = indices.size() - call.fillOffset;
	}

	call.uniformOffset = paintToUniforms(paint, scissor, width, fringe);
}


//
//	OtCanvas::renderStroke
//

void OtCanvas::renderStroke(NVGpaint* paint, NVGcompositeOperationState, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths) {
	// setup new rendering call
	auto& call = calls.emplace_back(Call{});
	call.type = CallType::stroke;
	call.image = paint->image;
	call.strokeOffset = indices.size();

	for (int p = 0; p < npaths; p++) {
		auto& path =  paths[p];

		if (path.nstroke > 0) {
			auto p = &path.stroke[0];
			size_t vertOffset = vertices.size();

			for (int i = 0; i < path.nstroke; i++, p++) {
				vertices.emplace_back(glm::vec2(p->x, p->y), glm::vec2(p->u, p->v));
			}

			for (int i = 2; i < path.nstroke; i++, p++) {
				if (i & 1) {
					indices.emplace_back(vertOffset);
					indices.emplace_back(vertOffset + 1);
					indices.emplace_back(vertOffset + 2);

				} else {
					indices.emplace_back(vertOffset);
					indices.emplace_back(vertOffset + 2);
					indices.emplace_back(vertOffset + 1);
				}

				vertOffset++;
			}
		}
	}

	call.strokeCount = indices.size() - call.strokeOffset;
	call.uniformOffset = paintToUniforms(paint, scissor, strokeWidth, fringe);
}


//
//	OtCanvas::renderTriangles
//

void OtCanvas::renderTriangles(NVGpaint* paint, NVGcompositeOperationState, NVGscissor* scissor, const NVGvertex* verts, int nverts, float fringe) {
	// setup new rendering call
	auto& call = calls.emplace_back(Call{});
	call.type = CallType::triangles;
	call.image = paint->image;
	call.fillOffset = indices.size();

	// copy vertex and index data for triangles
	auto index = vertices.size();
	auto p = verts;

	for (int i = 0; i < nverts; i++, p++) {
		vertices.emplace_back(glm::vec2(p->x, p->y), glm::vec2(p->u, p->v));
		indices.emplace_back(index++);
	}

	call.fillCount = indices.size() - call.fillOffset;
	call.uniformOffset = paintToUniforms(paint, scissor, 1.0f, fringe, true);
}


//
//	OtCanvas::paintToUniforms
//

size_t OtCanvas::paintToUniforms(NVGpaint* paint, NVGscissor* scissor, float width, float fringe, bool triangles) {
	float invxform[6];

	// create new uniform set
	auto index = fragmentUniforms.size();
	auto& uniforms = fragmentUniforms.emplace_back(FragmentUniforms{});

	// populate uniform values
	uniforms.innerColor = glm::vec4(
		paint->innerColor.r,
		paint->innerColor.g,
		paint->innerColor.b,
		paint->innerColor.a);

	uniforms.outerColor = glm::vec4(
		paint->outerColor.r,
		paint->outerColor.g,
		paint->outerColor.b,
		paint->outerColor.a);

	if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f) {
		uniforms.scissorExt.x = 1.0f;
		uniforms.scissorExt.y = 1.0f;
		uniforms.scissorScale.x = 1.0f;
		uniforms.scissorScale.y = 1.0f;

	} else {
		nvgTransformInverse(invxform, scissor->xform);

		uniforms.scissorMatCol1 = glm::vec4(invxform[0], invxform[1], 0.0f, 0.0f);
		uniforms.scissorMatCol2 = glm::vec4(invxform[2], invxform[3], 0.0f, 0.0f);
		uniforms.scissorMatCol3 = glm::vec4(invxform[4], invxform[5], 1.0f, 0.0f);

		uniforms.scissorExt.x = scissor->extent[0];
		uniforms.scissorExt.y = scissor->extent[1];
		uniforms.scissorScale.x = std::sqrt(scissor->xform[0] * scissor->xform[0] + scissor->xform[2] * scissor->xform[2]) / fringe;
		uniforms.scissorScale.y = std::sqrt(scissor->xform[1] * scissor->xform[1] + scissor->xform[3] * scissor->xform[3]) / fringe;
	}

	uniforms.extent = glm::vec2(paint->extent[0], paint->extent[1]);
	uniforms.strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;

	if (paint->image != 0) {
		// find texture
		auto entry = textures.find(paint->image);

		if (entry == textures.end()) {
			OtLogFatal("Can't find canvas texture with id {}", paint->image);
		}

		if (entry->second.flip) {
			float m1[6], m2[6];
			nvgTransformTranslate(m1, 0.0f, uniforms.extent.y * 0.5f);
			nvgTransformMultiply(m1, paint->xform);
			nvgTransformScale(m2, 1.0f, -1.0f);
			nvgTransformMultiply(m2, m1);
			nvgTransformTranslate(m1, 0.0f, -uniforms.extent.y * 0.5f);
			nvgTransformMultiply(m1, m2);
			nvgTransformInverse(invxform, m1);

		} else {
			nvgTransformInverse(invxform, paint->xform);
		}

		uniforms.shaderType = triangles ? textureShader : fillTextureShader;
		uniforms.texType = entry->second.texture.getFormat() == OtTexture::Format::r8 ? rTexture : rgbaTexture;

	} else {
		uniforms.shaderType = fillGradientShader;
		uniforms.radius = paint->radius;
		uniforms.feather = paint->feather;
		nvgTransformInverse(invxform, paint->xform);
	}

	uniforms.paintMatCol1 = glm::vec4(invxform[0], invxform[1], 0.0f, 0.0f);
	uniforms.paintMatCol2 = glm::vec4(invxform[2], invxform[3], 0.0f, 0.0f);
	uniforms.paintMatCol3 = glm::vec4(invxform[4], invxform[5], 1.0f, 0.0f);

	return index;
}


//
//	OtCanvas::setUniforms
//

void OtCanvas::setUniforms(OtRenderPass& pass, Call& call) {
	struct VertexUniforms {
		float width;
		float heigh;
	} vertexUniforms {
		width,
		height
	};

	pass.setVertexUniforms(0, &vertexUniforms, sizeof(VertexUniforms));
	pass.setFragmentUniforms(0, &fragmentUniforms[call.uniformOffset], sizeof(FragmentUniforms));

	if (call.image) {
		// find texture
		auto entry = textures.find(call.image);

		if (entry == textures.end()) {
			OtLogFatal("Can't find canvas texture with id {}", call.image);
		}

		pass.bindFragmentSampler(0, entry->second.sampler, entry->second.texture);

	} else {
		// submit a dummy texture
		OtTexture texture;
		pass.bindFragmentSampler(0, sampler, texture);
	}
}
