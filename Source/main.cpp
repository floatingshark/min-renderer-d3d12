#include <iostream>
#include <cassert>
#include <vector>
#include "window.hpp"
#include "mesh.hpp"
#include "parameter.hpp"
#include "directx.hpp"

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
	std::shared_ptr<arabesque::Parameter> parameter = std::make_shared<arabesque::Parameter>();
	directx->set_vertex_data(vertices, indices);
	directx->set_constant_data(parameter->get_constant());

	// Update Loop
	while (window->update_flag())
	{
		window->update_window();
		directx->render();
	}

	std::cout << "End program" << std::endl;

	return 0;
}