//	ObjectTalk Scripting Language
//	Copyright (c) 1993-2025 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#include "OtDejaVu.h"
#include "OtFramework.h"
#include "OtGpu.h"


//
//	OtFramework::initIMGUI
//

void OtFramework::initIMGUI() {
	// setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// deactivate Dear ImGui automatic .ini file handling
	auto& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	// windows can only be dragged using the title bar
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	// tweak default style
	ImGui::StyleColorsDark();
	auto& style = ImGui::GetStyle();
	style.HoverDelayShort = 0.5f;
	style.HoverDelayNormal = 1.0f;

	style.HoverFlagsForTooltipMouse =
		ImGuiHoveredFlags_Stationary |
		ImGuiHoveredFlags_DelayNormal |
		ImGuiHoveredFlags_AllowWhenDisabled;

	// setup platform/renderer backends
	auto& gpu = OtGpu::instance();
	ImGui_ImplSDL3_InitForSDLGPU(gpu.window);

	ImGui_ImplSDLGPU3_InitInfo initInfo{
		.Device = gpu.device,
		.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu.device, gpu.window),
		.MSAASamples = SDL_GPU_SAMPLECOUNT_1
	};

	ImGui_ImplSDLGPU3_Init(&initInfo);

	// setup our font
	ImFontConfig config;
	std::memcpy(config.Name, "DejaVu", 7);
	config.FontDataOwnedByAtlas = false;
	config.OversampleH = 1;
	config.OversampleV = 1;
	io.Fonts->Clear();
	io.Fonts->AddFontFromMemoryCompressedTTF((void*) &OtDejaVu, OtDejaVuSize, 15.0f, &config);
}


//
//	OtFramework::eventIMGUI
//

void OtFramework::eventIMGUI(SDL_Event& event) {
	ImGui_ImplSDL3_ProcessEvent(&event);
}


//
//	OtFramework::startFrameIMGUI
//

void OtFramework::startFrameIMGUI() {
	// start the Dear ImGui frame
	ImGui_ImplSDLGPU3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}


//
//	OtFramework::endFrameIMGUI
//

void OtFramework::endFrameIMGUI() {
	// render debug windows (if required)
	if (ImGui::IsKeyPressed(ImGuiKey_F16)) {
		profiler = !profiler;

	} else if (ImGui::IsKeyPressed(ImGuiKey_F17)) {
		metrics = !metrics;

	} else if (ImGui::IsKeyPressed(ImGuiKey_F18)) {
		demo = !demo;
	}

	if (profiler) {
		renderProfiler();
	}

	if (metrics) {
		ImGui::ShowMetricsWindow();
	}

	if (demo) {
		ImGui::ShowDemoWindow();
	}

	// render to the screen
	auto& gpu = OtGpu::instance();
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	const bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);

	if (gpu.swapchainTexture != nullptr && !isMinimized) {
		// run Dear ImGui copy pass
		ImGui_ImplSDLGPU3_PrepareDrawData(drawData, gpu.pipelineCommandBuffer);

		// setup Dear ImGui render target
		SDL_GPUColorTargetInfo targetInfo{
			.texture = gpu.swapchainTexture,
			.mip_level = 0,
			.layer_or_depth_plane = 0,
			.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f},
			.load_op = SDL_GPU_LOADOP_CLEAR,
			.store_op = SDL_GPU_STOREOP_STORE,
			.resolve_texture = nullptr,
			.resolve_mip_level = 0,
			.resolve_layer = 0,
			.cycle = false,
			.cycle_resolve_texture = false,
			.padding1 = 0,
			.padding2 = 0
		};

		// run Dear ImGui render pass
		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(gpu.pipelineCommandBuffer, &targetInfo, 1, nullptr);
		ImGui_ImplSDLGPU3_RenderDrawData(drawData, gpu.pipelineCommandBuffer, renderPass);
		SDL_EndGPURenderPass(renderPass);
	}
}


//
//	OtFramework::endIMGUI
//

void OtFramework::endIMGUI() {
	ImGui_ImplSDLGPU3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}


//
//	OtFramework::renderProfiler
//

void OtFramework::renderProfiler() {
	auto& gpu = OtGpu::instance();
	auto labelWith = ImGui::CalcTextSize("                         ").x;

	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Once);
	ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Framerate:"); ImGui::SameLine(labelWith); ImGui::Text("%.1f", 1000.0f / loopDuration);
	ImGui::Text("CPU [ms per frame]:"); ImGui::SameLine(labelWith); ImGui::Text("%0.2f", cpuTime);
	ImGui::Text("GPU [ms per frame]:"); ImGui::SameLine(labelWith); ImGui::Text("%0.2f", gpuTime);
	ImGui::Text("GPU wait [ms]:"); ImGui::SameLine(labelWith); ImGui::Text("%0.2f", gpuWaitTime);
	ImGui::Text("Back buffer width:"); ImGui::SameLine(labelWith); ImGui::Text("%d", gpu.width);
	ImGui::Text("Back buffer height:"); ImGui::SameLine(labelWith); ImGui::Text("%d", gpu.height);
	ImGui::Text("Anti-aliasing:"); ImGui::SameLine(labelWith); ImGui::Text("%d", antiAliasing);
	ImGui::End();
}
