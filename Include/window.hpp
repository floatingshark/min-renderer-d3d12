#pragma once
#include <iostream>
#include <string>
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <External/GLFW/glfw3.h>
#include <External/GLFW/glfw3native.h>
#include <global.hpp>

namespace albedo {
	class Window {
	public:
		Window() { init_window(); }

	protected:
		const std::string NAME		 = "albedo 0.1.0";
		GLFWwindow*		  window_ptr = nullptr;

	public:
		void init_window() {
			glfwSetErrorCallback(glfw_error_callback);
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			window_ptr = glfwCreateWindow(Global::window_width, Global::window_height, NAME.c_str(), nullptr, nullptr);
		}

		void update() {
			if (is_update()) {
				glfwPollEvents();
				update_control();
			}
		}
		void update_control() {
			ImGuiIO& io = ImGui::GetIO();

			if (io.WantCaptureMouse) {
				return;
			}

			// Mouse Cursor Position Update
			glm::vec3		pos	   = {Global::view_position[0], Global::view_position[1], Global::view_position[2]};
			const glm::vec3 lookat = {Global::view_lookat[0], Global::view_lookat[1], Global::view_lookat[2]};
			const glm::vec3 up	   = {Global::view_up[0], Global::view_up[1], Global::view_up[2]};

			if (ImGui::IsMouseDown(0)) {
				std::vector<float> move		= {io.MouseDelta.x, io.MouseDelta.y};
				glm::mat4		   rotate_x = glm::rotate(glm::mat4(1.f), -move[0] / 360.f, up);
				pos							= rotate_x * glm::vec4(pos, 1.f);

				glm::vec3 axis	   = glm::cross(up, pos);
				glm::mat4 rotate_y = glm::rotate(glm::mat4(1.f), -move[1] / 360.f, axis);
				pos				   = rotate_y * glm::vec4(pos, 1.f);
			}

			if (io.MouseWheel) {
				float	  wheel_size = io.MouseWheel;
				glm::vec3 zoom		 = lookat - pos;
				pos					 = pos + zoom * wheel_size / 10.f;
			}

			Global::view_position[0] = pos[0];
			Global::view_position[1] = pos[1];
			Global::view_position[2] = pos[2];
		}
		void shutdown() {
			glfwDestroyWindow(window_ptr);
			glfwTerminate();
		}

		inline GLFWwindow* get_window() { return window_ptr; }
		inline HWND		   get_hwnd() { return glfwGetWin32Window(this->window_ptr); }
		inline bool		   is_update() { return !glfwWindowShouldClose(window_ptr); }

		static void glfw_error_callback(int error, const char* description) {
			fprintf(stderr, "GLFW Error %d: %s\n", error, description);
		}
	};
} // namespace albedo