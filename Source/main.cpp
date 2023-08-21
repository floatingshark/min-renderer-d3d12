#include <iostream>
#include <cassert>
#include <vector>
#include <dxgi.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_dx12.h>
#include "window.hpp"
#include "mesh.hpp"
#include "constant.hpp"
#include "directx.hpp"
#include "ui.hpp"
#include "control.hpp"

int main()
{
	std::cout << "Begin Program" << std::endl;

	// Create Window
	std::shared_ptr<arabesques::Window> window = std::make_shared<arabesques::Window>();
	window->init_window();
	std::cout << "Prepared Window" << std::endl;

	// Create DirectX12 Context
	std::shared_ptr<arabesques::DirectXA> directx = std::make_shared<arabesques::DirectXA>(window->get_hwnd());
	directx->init_directx();
	std::cout << "Prepared DirectX12" << std::endl;

	// Initialize Default Object
	std::vector<arabesques::Mesh::Vertex> vertices;
	std::vector<int> indices;
	//arabesques::Mesh::create_plane(vertices, indices);
	arabesques::Mesh::create_cube(vertices, indices);
	directx->set_vertex_data(vertices, indices);

	// Initialize Constant
	std::shared_ptr<arabesques::Constant> constant = std::make_shared<arabesques::Constant>();
	directx->set_constant_data(constant->get_wvp());

	// Initialize Control
	std::shared_ptr<arabesques::Control> control = std::make_shared<arabesques::Control>();

	// Initialize UI
	std::shared_ptr<arabesques::UI> ui = std::make_shared<arabesques::UI>();
	ui->init_imgui();
	ui->init_imgui_glfw(window->get_window());
	ui->init_imgui_directX(directx->get_device().Get(), directx->get_num_frames(), directx->get_srv_heap().Get());

	// Update Loop
	while (window->update_flag())
	{
		ui->update();

		control->init_via_imgui();
		control->update();

		ui->render();

		constant->calculate_wvp();
		directx->set_constant_data(constant->get_wvp());

		window->update_window();

		directx->render();
	}

	// Terminate
	ui->shutdown();
	window->terminate();

	std::cout << "End Program" << std::endl;

	return 0;
}