#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define MAIN_LOG(log_msg) std::cout << "[Main]" << log_msg << std::endl;
// #undef _DEBUG

#include "constant.hpp"
#include "control.hpp"
#include "directx.hpp"
#include "object.hpp"
#include "scene.hpp"
#include "shape.hpp"
#include "ui.hpp"
#include "window.hpp"
#include <cassert>
#include <dxgi.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_glfw.h>
#include <iostream>
#include <vector>

int main() {
	MAIN_LOG("============================= Begin Program =============================");

	std::unique_ptr<albedos::Window> window = std::make_unique<albedos::Window>();
	MAIN_LOG("Prepared Window");

	std::unique_ptr<albedos::DirectXA> directx = std::make_unique<albedos::DirectXA>(window->get_hwnd());
	MAIN_LOG("Prepared DirectX12");

	std::unique_ptr<albedos::Constant> constant = std::make_unique<albedos::Constant>();
	MAIN_LOG("Prepared Constant Datum");

	std::unique_ptr<albedos::Control> control = std::make_unique<albedos::Control>();
	MAIN_LOG("Prepared User Control");

	std::unique_ptr<albedos::Scene> scene =
		std::make_unique<albedos::Scene>(directx->get_device(), directx->get_cbv_srv_heap());
	MAIN_LOG("Prepared Scene Datum");

	std::unique_ptr<albedos::UI> ui = std::make_unique<albedos::UI>();
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
		control->update();
		constant->update();

		for (std::shared_ptr<albedos::Object> object : scene->render_objects) {
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