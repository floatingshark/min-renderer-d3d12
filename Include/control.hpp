#pragma once
#define GLM_FORCE_SWIZZLE
#include "global.hpp"
#include <External/glm/glm.hpp>
#include <External/glm/gtc/matrix_transform.hpp>
#include <External/glm/gtx/string_cast.hpp>
#include <External/imgui/imgui.h>
#include <External/imgui/imgui_impl_glfw.h>
#include <cassert>
#include <iostream>
#include <vector>

namespace albedo {
	class Control {
	protected:
		bool			   capture_mouse;
		float			   delta_time;
		std::vector<float> move;
		float			   wheel;
		bool			   left_click_down;
		bool			   left_click_released;
		bool			   wheel_click;

	public:
		void update_imgui_contexts() {
			ImGuiIO& io			= ImGui::GetIO();
			capture_mouse		= io.WantCaptureMouse;
			delta_time			= io.DeltaTime;
			move				= {io.MouseDelta.x, io.MouseDelta.y};
			wheel				= io.MouseWheel;
			left_click_down		= ImGui::IsMouseDown(0);
			left_click_released = ImGui::IsMouseReleased(0);
			wheel_click			= ImGui::IsMouseDown(2);
		}
		void update() {
			update_imgui_contexts();

			if (capture_mouse) {
				return;
			}

			// Mouse Cursor Position Update
			glm::vec3		pos	   = {Global::view_position[0], Global::view_position[1], Global::view_position[2]};
			const glm::vec3 lookat = {Global::view_lookat[0], Global::view_lookat[1], Global::view_lookat[2]};
			const glm::vec3 up	   = {Global::view_up[0], Global::view_up[1], Global::view_up[2]};
			if (left_click_down) {
				glm::mat4 rotate_x = glm::rotate(glm::mat4(1.f), -move[0] / 360.f, up);
				pos				   = rotate_x * glm::vec4(pos, 1.f);

				glm::vec3 axis	   = glm::cross(up, pos);
				glm::mat4 rotate_y = glm::rotate(glm::mat4(1.f), -move[1] / 360.f, axis);
				pos				   = rotate_y * glm::vec4(pos, 1.f);
			}

			// Mouse Wheel Update
			if (wheel) {
				glm::vec3 zoom = lookat - pos;
				pos			   = pos + zoom * wheel / 10.f;
			}

			Global::view_position[0] = pos[0];
			Global::view_position[1] = pos[1];
			Global::view_position[2] = pos[2];
		}
	};
} // namespace albedo