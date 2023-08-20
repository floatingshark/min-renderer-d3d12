#include <iostream>
#include <cassert>
#include <vector>
#include <dxgi.h>
#include "window.hpp"
#include "mesh.hpp"
#include "param.hpp"
#include "directx.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_dx12.h"

int main()
{
	std::cout << "Begin Program" << std::endl;

	// Create Window
	std::shared_ptr<arabesque::Window> window = std::make_shared<arabesque::Window>();
	window->init_window();
	HWND hwnd = window->get_hwnd();
	assert(hwnd && "Failed to get HWND");
	std::cout << "Prepared Window" << std::endl;

	// Create DirectX12 Context
	std::shared_ptr<arabesque::DirectXA> directx = std::make_shared<arabesque::DirectXA>(hwnd);
	directx->init_directx();
	std::cout << "Initilized DirectX12" << std::endl;

	// Initialize Objects
	std::vector<arabesque::Mesh::Vertex> vertices;
	std::vector<int> indices;
	arabesque::Mesh::create_plane(vertices, indices);
	std::shared_ptr<arabesque::Param> parameter = std::make_shared<arabesque::Param>();
	directx->set_vertex_data(vertices, indices);
	directx->set_constant_data(parameter->get_constant());

	// Setup Deer Imgui Context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImGui::StyleColorsLight();
	// Setup Platform/Renderer Backends
	ImGui_ImplGlfw_InitForOther(window->get_window(), true);
	ImGui_ImplDX12_Init(directx->get_device().Get(), directx->get_num_frames(),
						DXGI_FORMAT_R8G8B8A8_UNORM, directx->get_srv_heap().Get(),
						directx->get_srv_heap()->GetCPUDescriptorHandleForHeapStart(),
						directx->get_srv_heap()->GetGPUDescriptorHandleForHeapStart());

	// Update Loop
	bool show_demo_window = true;
	while (window->update_flag())
	{
		// Start the Dear ImGui Frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Rendering
		ImGui::ShowDemoWindow(&show_demo_window);
		ImGui::Render();

		window->update_window();
		directx->render();
	}

	// Creanup Imgui
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	window->terminate();

	std::cout << "End program" << std::endl;

	return 0;
}