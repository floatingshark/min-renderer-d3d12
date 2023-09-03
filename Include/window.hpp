#pragma once
#include <iostream>
#include <string>
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace albedos {
	class Window {
	public:
		Window() { init_window(); }

	protected:
		const uint32_t	  WIDTH	 = 800;
		const uint32_t	  HEIGHT = 600;
		const std::string NAME	 = "albedo 0.1.0";
		GLFWwindow*		  window = nullptr;

	public:
		void init_window() {
			glfwSetErrorCallback(glfw_error_callback);
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			this->window = glfwCreateWindow(WIDTH, HEIGHT, NAME.c_str(), nullptr, nullptr);
		}
		inline GLFWwindow* get_window() { return window; }
		inline HWND		   get_hwnd() { return glfwGetWin32Window(this->window); }
		inline bool		   is_update() { return !glfwWindowShouldClose(window); }
		void			   update() { glfwPollEvents(); }
		void			   shutdown() {
			  glfwDestroyWindow(window);
			  glfwTerminate();
		}

		static void glfw_error_callback(int error, const char* description) {
			fprintf(stderr, "GLFW Error %d: %s\n", error, description);
		}
	};
} // namespace albedos