#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define MAIN_LOG(log_msg) std::cout << "[Main]" << log_msg << std::endl;
// #undef _DEBUG

#include "constant.hpp"
#include "directx.hpp"
#include "object.hpp"
#include "scene.hpp"
#include "shape.hpp"
#include "ui.hpp"
#include "window.hpp"
#include <External/imgui/imgui.h>
#include <External/imgui/imgui_impl_dx12.h>
#include <External/imgui/imgui_impl_glfw.h>
#include <cassert>
#include <dxgi.h>
#include <iostream>
#include <vector>

int main() {
	MAIN_LOG("============================= Begin Program =============================");

	std::unique_ptr<albedo::Window> window = std::make_unique<albedo::Window>();
	MAIN_LOG("Prepared Window");

	std::unique_ptr<albedo::DirectXA> directx = std::make_unique<albedo::DirectXA>(window->get_hwnd());
	MAIN_LOG("Prepared DirectX12");

	std::unique_ptr<albedo::Constant> constant = std::make_unique<albedo::Constant>();
	MAIN_LOG("Prepared Constant Datum");

	std::unique_ptr<albedo::Scene> scene =
		std::make_unique<albedo::Scene>(directx->get_device(), directx->get_cbv_srv_heap());
	MAIN_LOG("Prepared Scene Datum");

	std::unique_ptr<albedo::UI> ui = std::make_unique<albedo::UI>();
	MAIN_LOG("Prepared UI");

	ui->init(window->get_window(), directx->get_device(), directx->get_num_frames(), directx->get_imgui_heap());
	MAIN_LOG("Initialized ImGui for DirectX");

	directx->set_render_objects(scene->render_objects);
	directx->set_render_skydome(scene->skydome_object);
	ui->set_render_objects(scene->render_objects);
	ui->set_skydome_object(scene->skydome_object);
	MAIN_LOG("Initialized Object to Renderer");

	while (window->is_update()) {
		ui->update();
		window->update();
		constant->update();

		for (std::shared_ptr<albedo::Object> object : scene->render_objects) {
			object->update_directx_constant_resources(constant->get_scene(), constant->get_local());
		}
		scene->skydome_object->update_directx_constant_resources(constant->get_scene(), constant->get_local());

		ui->render();
		directx->render();
	}

	ui->shutdown();
	window->shutdown();

	MAIN_LOG("============================= End Program =============================");

	return 0;
}