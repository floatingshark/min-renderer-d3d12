#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cassert>
#include <vector>
#include <dxgi.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_dx12.h>
#include "window.hpp"
#include "constant.hpp"
#include "control.hpp"
#include "directx.hpp"
#include "object.hpp"
#include "shape.hpp"
#include "ui.hpp"


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

	// Initialize Constant
	std::shared_ptr<arabesques::Constant> constant = std::make_shared<arabesques::Constant>();

	// Initialize Control
	std::shared_ptr<arabesques::Control> control = std::make_shared<arabesques::Control>();

	// Initialize Default Object
	arabesques::Object object_1 = arabesques::Object(directx->get_device(), "Default Torus", arabesques::Shape::Type::Torus);
	arabesques::Object object_2 = arabesques::Object(directx->get_device(), "Floor Plane", arabesques::Shape::Type::Plane);
	object_2.position = {0.f, 0.f, -0.5f};
	object_2.scale = {3.f, 3.f, 1.f};
	std::vector<arabesques::Object> scene_objects = {object_1, object_2};
	directx->set_objects(scene_objects);
	std::cout << "Prepared Mesh Data" << std::endl;

	// Initialize UI
	std::shared_ptr<arabesques::UI> ui = std::make_shared<arabesques::UI>();
	ui->init_imgui();
	ui->init_imgui_glfw(window->get_window());
	ui->init_imgui_directX(directx->get_device(), directx->get_num_frames(), directx->get_srv_heap());

	// Update Loop
	while (window->update_flag())
	{
		ui->update(scene_objects);

		control->init_via_imgui();
		control->update();

		ui->render();

		constant->calculate_wvp();
		constant->calculate_light();
		for (arabesques::Object &object : scene_objects)
		{
			object.set_constant_buffer_1(constant->get_wvp());
			object.set_constant_buffer_2(constant->get_light());
		}

		window->update_window();

		directx->render();
	}

	// Terminate
	ui->shutdown();
	window->terminate();

	std::cout << "End Program" << std::endl;

	return 0;
}