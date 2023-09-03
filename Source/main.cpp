#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define MAIN_LOG(log_msg) std::cout << "[MAIN]" << log_msg << std::endl;
// #undef _DEBUG

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

	std::shared_ptr<albedos::Window> window = std::make_shared<albedos::Window>();
	MAIN_LOG("Prepared Window");

	std::shared_ptr<albedos::DirectXA> directx = std::make_shared<albedos::DirectXA>(window->get_hwnd());
	MAIN_LOG("Prepared DirectX12");

	std::shared_ptr<albedos::Constant> constant = std::make_shared<albedos::Constant>();
	std::shared_ptr<albedos::Control> control = std::make_shared<albedos::Control>();

	albedos::Object object_1 = albedos::Object(directx->get_device(), directx->get_cbv_srv_heap(), "Default Torus", albedos::Shape::Type::Torus);
	albedos::Object object_2 = albedos::Object(directx->get_device(), directx->get_cbv_srv_heap(), "Floor Plane", albedos::Shape::Type::Plane);
	object_2.position = {0.f, 0.f, -0.5f};
	object_2.scale = {3.f, 3.f, 1.f};
	std::vector<albedos::Object> scene_objects = {object_1, object_2};
	directx->init_render_objects(scene_objects);
	MAIN_LOG("Prepared Mesh Data");

	std::shared_ptr<albedos::UI> ui = std::make_shared<albedos::UI>();
	ui->init_UI_directX(window->get_window(), directx->get_device(), directx->get_num_frames(), directx->get_imgui_heap());
	MAIN_LOG("Prepared ImGui DirectX");

	// Update Loop
	MAIN_LOG("Begin Update Loop");
	while (window->update_flag())
	{
		ui->update(scene_objects);
		window->update_window();
		control->update();
		constant->update_scene();
		for (albedos::Object &object : scene_objects)
		{
			object.update_constant_buffer_1(constant->get_scene());
			object.update_constant_buffer_2(constant->get_local());
		}
		ui->render();
		directx->render();
	}

	ui->shutdown();
	window->terminate();

	MAIN_LOG("End Program");

	return 0;
}