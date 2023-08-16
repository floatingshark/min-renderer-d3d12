#include <iostream>
#include <cassert>
#include "window.hpp"
#include "directx.hpp"

int main()
{
	std::cout << "Begin program" << std::endl;

	// Create window
	std::shared_ptr<arabesque::Window> window = std::make_shared<arabesque::Window>();
	window->init_window();
	HWND hwnd = window->get_hwnd();
	assert(hwnd && "Failed to get HWND");

	// Create DirectX12 context
	std::shared_ptr<arabesque::DirectX> directx = std::make_shared<arabesque::DirectX>();
	directx->init_directx();

	window->update_window();

	std::cout << "End program" << std::endl;

	return 0;
}