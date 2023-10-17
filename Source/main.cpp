#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define MAIN_LOG(log_msg) std::cout << "[Main]" << log_msg << std::endl;
// #undef _DEBUG

#include "constant.hpp"
#include "directx.hpp"
#include "entity.hpp"
#include "world.hpp"
#include "shape.hpp"
#include "gui.hpp"
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

	std::unique_ptr<albedo::DirectXA> directx = std::make_unique<albedo::DirectXA>();
	MAIN_LOG("Prepared DirectX12");

	std::unique_ptr<albedo::Constant> constant = std::make_unique<albedo::Constant>();
	MAIN_LOG("Prepared Constant Datum");

	std::unique_ptr<albedo::World> world = std::make_unique<albedo::World>();
	MAIN_LOG("Prepared World Datum");

	std::unique_ptr<albedo::GUI> gui = std::make_unique<albedo::GUI>();
	MAIN_LOG("Prepared GUI");


	directx->set_render_objects(world->get_entities());
	directx->set_render_skydome(world->get_skydome_entity());
	MAIN_LOG("Initialized Entity to Renderer");

	while (window->should_update()) {
		gui->update();
		window->update();
		constant->update();

		for (std::shared_ptr<albedo::Entity> object : world->get_entities()) {
			object->update_directx_constant_resources(constant->get_scene(), constant->get_local());
		}
		world->get_skydome_entity()->update_directx_constant_resources(constant->get_scene(), constant->get_local());

		gui->render();
		directx->render();
	}

	gui->shutdown();
	window->shutdown();

	MAIN_LOG("============================= End Program =============================");

	return 0;
}