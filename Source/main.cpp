#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define MAIN_LOG(log_msg) std::cout << "[MAIN]" << log_msg << std::endl;
#undef _DEBUG

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
	MAIN_LOG("Begin Program");

	// Create Window
	std::shared_ptr<albedos::Window> window = std::make_shared<albedos::Window>();
	window->init_window();
	MAIN_LOG("Prepared Window");

	// Create DirectX12 Context
	std::shared_ptr<albedos::DirectXA> directx = std::make_shared<albedos::DirectXA>(window->get_hwnd());
	directx->init_directx();
	MAIN_LOG("Prepared DirectX12");

	// Initialize Constant
	std::shared_ptr<albedos::Constant> constant = std::make_shared<albedos::Constant>();

	// Initialize Control
	std::shared_ptr<albedos::Control> control = std::make_shared<albedos::Control>();

	// Initialize Default Object
	albedos::Object object_1 = albedos::Object(directx->get_device(), directx->get_cbv_srv_heap(), "Default Torus", albedos::Shape::Type::Torus);
	albedos::Object object_2 = albedos::Object(directx->get_device(), directx->get_cbv_srv_heap(), "Floor Plane", albedos::Shape::Type::Plane);
	object_2.position = {0.f, 0.f, -0.5f};
	object_2.scale = {3.f, 3.f, 1.f};
	//object_2.use_texture = true;
	std::vector<albedos::Object> scene_objects = {object_1, object_2};
	directx->set_objects(scene_objects);
	MAIN_LOG("Prepared Mesh Data");

	// Initialize UI
	std::shared_ptr<albedos::UI> ui = std::make_shared<albedos::UI>();
	ui->init_imgui();
	ui->init_imgui_glfw(window->get_window());
	ui->init_imgui_directX(directx->get_device(), directx->get_num_frames(), directx->get_imgui_heap());

	// Update Loop
	MAIN_LOG("Begin Update Loop");
	while (window->update_flag())
	{
		ui->update(scene_objects);

		control->init_via_imgui();
		control->update();

		ui->render();

		constant->calculate_scene();
		for (albedos::Object &object : scene_objects)
		{
			object.map_constant_buffer_1(constant->get_scene());
			object.map_constant_buffer_2(constant->get_local());
		}

		window->update_window();

		directx->render();
	}

	// Terminate
	ui->shutdown();
	window->terminate();

	MAIN_LOG("End Program");

	return 0;
}