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
#include "OtRenderPass.h"


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
	params.renderViewport = [](void* ptr, float width, float height, float devicePixelRatio) { ((OtCanvas*) ptr)->renderViewport(width, height, devicePixelRatio); };
	params.renderCancel = [](void* ptr) { ((OtCanvas*) ptr)->renderCancel(); };
	params.renderFlush = [](void* ptr) { ((OtCanvas*) ptr)->renderFlush(); };
	params.renderFill = [](void* ptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, const float* bounds, const NVGpath* paths, int npaths) { ((OtCanvas*) ptr)->renderFill(paint, compositeOperation, scissor, fringe, bounds, paths, npaths); };
	params.renderStroke = [](void* ptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe, float strokeWidth, const NVGpath* paths, int npaths) { ((OtCanvas*) ptr)->renderStroke(paint, compositeOperation, scissor, fringe, strokeWidth, paths, npaths); };
	params.renderTriangles = [](void* ptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, const NVGvertex* verts, int nverts, float fringe) { ((OtCanvas*) ptr)->renderTriangles(paint, compositeOperation, scissor, verts, nverts, fringe); };
	params.renderDelete = [](void* ptr) { ((OtCanvas*) ptr)->renderDelete(); };
	params.userPtr = this;
	params.edgeAntiAlias = 1;
	context = nvgCreateInternal(& params);

	if (!context) {
		OtLogFatal("Internal error: can't create a new canvas context");
	}
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

	// setup rendering pass
	OtRenderPass pass;
	pass.start(framebuffer);
	// pass.setClear(framebuffer.hasColorTexture(), framebuffer.hasStencilTexture(), glm::vec4(0.0f));
	// pass.setViewMode(OtPass::sequential);
	// pass.setFrameBuffer(framebuffer);
	// pass.touch();

	// set fragment uniforms
	struct FragmentUniforms {
		float scissorMat[12]; // matrices are actually 3 vec4s
		float paintMat[12];
		glm::vec4 innerCol;
		glm::vec4 outerCol;
		glm::vec2 scissorExt;
		glm::vec2 scissorScale;
		glm::vec2 extent;
		float radius;
		float feather;
		float strokeMult;
		float strokeThr;
		int texType;
		int type;

	} fragmentUniforms;

	pass.setFragmentUniforms(0, &fragmentUniforms, sizeof(fragmentUniforms));

	// render the canvas
	nvgBeginFrame(context, width, height, 1.0);
	nvgScale(context, scale, scale);

	try {
		renderer();

	} catch (OtException& e) {
		nvgEndFrame(context);
		throw(e);
	}

	nvgEndFrame(context);
	pass.end();
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
	// create the texture
	auto [it, valid] = textures.emplace(nextTextureID++, Texture());
	auto id = it->first;
	auto& texture = it->second.texture;
	auto& sampler = it->second.sampler;
	auto format = (type == NVG_TEXTURE_RGBA) ? OtTexture::Format::rgba8 : OtTexture::Format::r8;

	if (data) {
		texture.load(w, h, format, (void*) data);

	} else {
		texture.update(w, h, format, OtTexture::Usage::sampler);
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

int OtCanvas::renderDeleteTexture(int texture) {
	// find and delete texture
	auto entry = textures.find(texture);

	if (entry != textures.end()) {
		textures.erase(texture);
	}

	return 1;
}


//
//	OtCanvas::renderUpdateTexture
//

int OtCanvas::renderUpdateTexture(int texture, int x, int y, int w, int h, const unsigned char* data) {
	// find texture
	auto entry = textures.find(texture);

	if (entry == textures.end()) {
		OtLogFatal("Can't find texture in canvas with id {}", texture);
	}

	// update the texture
	entry->second.texture.update(x, y, w, h, (void*) data);
	return 1;
}


//
//	OtCanvas::renderGetTextureSize
//

int OtCanvas::renderGetTextureSize(int texture, int* w, int* h) {
	// find texture
	auto entry = textures.find(texture);

	if (entry == textures.end()) {
		OtLogFatal("Can't find texture in canvas with id {}", texture);
	}

	// get the size
	*w = entry->second.texture.getWidth();
	*h = entry->second.texture.getHeight();
	return 1;
}


//
//	OtCanvas::renderViewport
//

void OtCanvas::renderViewport([[maybe_unused]] float width, [[maybe_unused]] float height, [[maybe_unused]] float devicePixelRatio) {
	// nothing to do
}


//
//	OtCanvas::renderCancel
//

void OtCanvas::renderCancel() {
	// nothing to do
}


//
//	OtCanvas::renderFlush
//

void OtCanvas::renderFlush() {
	// TODO
}


//
//	OtCanvas::renderFill
//

void OtCanvas::renderFill([[maybe_unused]] NVGpaint* paint, [[maybe_unused]] NVGcompositeOperationState compositeOperation, [[maybe_unused]] NVGscissor* scissor, [[maybe_unused]] float fringe, [[maybe_unused]] const float* bounds, [[maybe_unused]] const NVGpath* paths, [[maybe_unused]] int npaths) {
	// int indexCount = 0;
	// int strokeCount = 0;
	// int maxverts = maxVertCount(paths, npaths, &indexCount, &strokeCount) + 6;

	// if (vertices.size() + maxverts > std::numeric_limits<uint16_t>::max()) {
	// 	renderFlush(scissor);
	// }
}


//
//	OtCanvas::renderStroke
//

void OtCanvas::renderStroke([[maybe_unused]] NVGpaint* paint, [[maybe_unused]] NVGcompositeOperationState compositeOperation, [[maybe_unused]] NVGscissor* scissor, [[maybe_unused]] float fringe, [[maybe_unused]] float strokeWidth, [[maybe_unused]] const NVGpath* paths, [[maybe_unused]] int npaths) {
	// int strokeCount = 0;
	// int maxverts = maxVertCount(paths, npaths, nullptr, &strokeCount);

	// if (vertices.size() + maxverts > std::numeric_limits<uint16_t>::max()) {
	// 	renderFlush(scissor);
	// }
}


//
//	OtCanvas::renderTriangles
//

void OtCanvas::renderTriangles([[maybe_unused]] NVGpaint* paint, [[maybe_unused]] NVGcompositeOperationState compositeOperation, [[maybe_unused]] NVGscissor* scissor, [[maybe_unused]] const NVGvertex* verts, [[maybe_unused]] int nverts, [[maybe_unused]] float fringe) {
}


//
//	OtCanvas::renderDelete
//

void OtCanvas::renderDelete() {
}
