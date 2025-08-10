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

#include "OtDejaVu.h"
#include "OtFramework.h"


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
	ImGui_ImplSDL3_InitForSDLGPU(window);
	ImGui_ImplSDLGPU3_InitInfo init_info = {};
	init_info.Device = gpuDevice;
	init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpuDevice, window);
	init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
	ImGui_ImplSDLGPU3_Init(&init_info);

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
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	const bool is_minimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);

	if (swapchainTexture != nullptr && !is_minimized) {
		// setup and start a render pass
		ImGui_ImplSDLGPU3_PrepareDrawData(drawData, commandBuffer);

		SDL_GPUColorTargetInfo targetInfo = {};
		targetInfo.texture = swapchainTexture;
		targetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		targetInfo.store_op = SDL_GPU_STOREOP_STORE;
		targetInfo.mip_level = 0;
		targetInfo.layer_or_depth_plane = 0;
		targetInfo.cycle = false;
		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &targetInfo, 1, nullptr);

		// render ImGui
		ImGui_ImplSDLGPU3_RenderDrawData(drawData, commandBuffer, renderPass);
		SDL_EndGPURenderPass(renderPass);
	}
}


//
//	OtFramework::endIMGUI
//

void OtFramework::endIMGUI() {
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImGui::DestroyContext();
}


//
//	OtFramework::renderProfiler
//

void OtFramework::renderProfiler() {
	auto labelWith = ImGui::CalcTextSize("                         ").x;

	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Once);
	ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Framerate:"); ImGui::SameLine(labelWith); ImGui::Text("%.1f", 1000.0f / loopDuration);
	ImGui::Text("CPU [ms per frame]:"); ImGui::SameLine(labelWith); ImGui::Text("%0.2f", cpuTime);
	ImGui::Text("GPU [ms per frame]:"); ImGui::SameLine(labelWith); ImGui::Text("%0.2f", gpuTime);
	ImGui::Text("GPU wait [ms]:"); ImGui::SameLine(labelWith); ImGui::Text("%0.2f", gpuWaitTime);
	ImGui::Text("Backbuffer width:"); ImGui::SameLine(labelWith); ImGui::Text("%d", width);
	ImGui::Text("Backbuffer height:"); ImGui::SameLine(labelWith); ImGui::Text("%d", height);
	ImGui::Text("Anti-aliasing:"); ImGui::SameLine(labelWith); ImGui::Text("%d", antiAliasing);
	ImGui::End();
}
