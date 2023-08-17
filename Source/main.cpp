#include <iostream>
#include <cassert>
#include "window.hpp"
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

	// Update Loop
	window->update_window();

	std::cout << "End program" << std::endl;

	return 0;
}