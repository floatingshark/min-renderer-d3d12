#pragma once
#include <iostream>
#include <string>
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

namespace arabesque
{
	class Window
	{
	protected:
		const uint32_t WIDTH = 800;
		const uint32_t HEIGHT = 600;
		const std::string NAME = "Arabesque";
		GLFWwindow *window = nullptr;

	public:
		void init_window()
		{
			glfwSetErrorCallback(glfw_error_callback);
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			this->window = glfwCreateWindow(WIDTH, HEIGHT, "Arabesque", nullptr, nullptr);
		}
		inline GLFWwindow *get_window()
		{
			return window;
		}
		inline HWND get_hwnd()
		{
			return glfwGetWin32Window(this->window);
		}
		bool update_flag()
		{
			return !glfwWindowShouldClose(window);
		}
		void update_window()
		{
			glfwPollEvents();
		}
		void terminate()
		{
			glfwDestroyWindow(window);
			glfwTerminate();
		}

		static void glfw_error_callback(int error, const char *description)
		{
			fprintf(stderr, "GLFW Error %d: %s\n", error, description);
		}
	};
}