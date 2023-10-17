#pragma once
#include <External/glm/glm.hpp>
#include <External/glm/gtc/matrix_transform.hpp>
#include <External/glm/gtx/string_cast.hpp>
#include <External/imgui/imgui.h>
#include <iostream>
#include <string>
#include <vector>

namespace albedo {
	class CameraManager {
	public:
		CameraManager() {}

		static float camera_position[3];
		static float camera_lookat[3];
		static float camera_up[3];
		static float projection_FOV;
		static float projection_near;
		static float projection_far;

		void update() { update_camera_control(); }

	private:
		void update_camera_control() {
			ImGuiIO& io = ImGui::GetIO();

			if (io.WantCaptureMouse) {
				return;
			}

			glm::vec3		pos	   = {camera_position[0], camera_position[1], camera_position[2]};
			const glm::vec3 lookat = {camera_lookat[0], camera_lookat[1], camera_lookat[2]};
			const glm::vec3 up	   = {camera_up[0], camera_up[1], camera_up[2]};

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

			camera_position[0] = pos[0];
			camera_position[1] = pos[1];
			camera_position[2] = pos[2];
		}
	};
	float CameraManager::camera_position[3] = {-0.f, -6.f, 4.f};
	float CameraManager::camera_lookat[3]	= {0.f, 0.f, 1.5f};
	float CameraManager::camera_up[3]		= {0.f, 0.f, 1.f};
	float CameraManager::projection_FOV		= 60.f;
	float CameraManager::projection_near	= 0.01f;
	float CameraManager::projection_far		= 1000.f;
} // namespace albedo