#pragma once
#include <iostream>
#include <string>
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include "system_variables.hpp"
#include <External/GLFW/glfw3.h>
#include <External/GLFW/glfw3native.h>
#include <External/imgui/imgui.h>

namespace albedo {
	class WindowManager {
	public:
		WindowManager() { construct_window(); }

		static GLFWwindow* window_ptr;
		static HWND		   hwnd;
		static int		   window_width;
		static int		   window_height;

	protected:
		const std::string NAME = "albedo 0.1";

	public:
		void construct_window() {
			glfwSetErrorCallback(glfw_error_callback);
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			window_ptr = glfwCreateWindow(window_width, window_height, NAME.c_str(), nullptr, nullptr);
			hwnd	   = glfwGetWin32Window(this->window_ptr);
		}

		void update() {
			if (should_update()) {
				glfwPollEvents();
			}
		}
		void shutdown() {
			glfwDestroyWindow(window_ptr);
			glfwTerminate();
		}

		inline bool should_update() { return !glfwWindowShouldClose(window_ptr); }
		static void glfw_error_callback(int error, const char* description) {
			fprintf(stderr, "GLFW Error %d: %s\n", error, description);
		}
	};
	GLFWwindow* albedo::WindowManager::window_ptr	 = nullptr;
	HWND		albedo::WindowManager::hwnd			 = nullptr;
	int			albedo::WindowManager::window_width	 = 1000;
	int			albedo::WindowManager::window_height = 600;

} // namespace albedo