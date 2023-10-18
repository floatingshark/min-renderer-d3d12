#pragma once
#include "camera_manager.hpp"
#include "system_variables.hpp"
#include <External/glm/glm.hpp>
#include <External/glm/gtc/matrix_transform.hpp>
#include <External/glm/gtx/string_cast.hpp>
#include <cassert>
#include <iostream>
#include <vector>
#include <window_manager.hpp>

namespace albedo {
	class DirectXConstant {
	public:
		typedef struct World {
			glm::mat4x4 view_matrix;
			glm::mat4x4 projection_matrix;
			glm::vec4	view_position;
			glm::vec4	light_position;
			glm::vec4	light_ambient;
			glm::mat4x4 light_view_matrix;
			glm::mat4x4 light_projection_matrix;
			float		light_intensity;
			int			is_enabled_shadow_mapping;
			float		shadow_mapping_bias;
		} World;

		typedef struct Local {
			glm::mat4x4 model;
			float		specular_power;
		} Local;

		DirectXConstant() { init(); }

		World world;
		Local local;

	public:
		void init() {
			world.view_matrix =
				glm::lookAt(glm::vec3(0.f, 0.f, 0.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
			world.projection_matrix			= glm::perspective(60.0f, 4.0f / 3.0f, 0.01f, 100.f);
			world.view_position				= glm::vec4(0.f, 0.f, 0.5f, 1.f);
			world.light_position			= glm::vec4(0.f, 10.f, 0.f, 1.f);
			world.light_ambient				= {0.f, 0.f, 0.f, 0.f};
			world.light_view_matrix			= glm::mat4(1.f);
			world.light_projection_matrix	= glm::mat4(1.f);
			world.light_intensity			= 1.f;
			world.is_enabled_shadow_mapping = false;
			world.shadow_mapping_bias		= 0.005f;

			local.model			 = glm::mat4(1.f);
			local.specular_power = 1.f;
		}

		void update_world() {
			world.view_matrix = glm::lookAt(
				glm::vec3(albedo::CameraManager::camera_position[0], albedo::CameraManager::camera_position[1],
						  albedo::CameraManager::camera_position[2]),
				glm::vec3(albedo::CameraManager::camera_lookat[0], albedo::CameraManager::camera_lookat[1],
						  albedo::CameraManager::camera_lookat[2]),
				glm::vec3(albedo::CameraManager::camera_up[0], albedo::CameraManager::camera_up[1],
						  albedo::CameraManager::camera_up[2]));
			world.projection_matrix = glm::perspective(
				glm::radians(albedo::CameraManager::projection_FOV),
				((float)albedo::WindowManager::window_width / (float)albedo::WindowManager::window_height),
				albedo::CameraManager::projection_near, albedo::CameraManager::projection_far);
			world.view_position =
				glm::vec4(albedo::CameraManager::camera_position[0], albedo::CameraManager::camera_position[1],
						  albedo::CameraManager::camera_position[2], 1.f);

			world.light_position =
				glm::vec4(System::light_position[0], System::light_position[1], System::light_position[2], 1.f);
			world.light_ambient = glm::vec4(System::light_ambient[0], System::light_ambient[1],
											System::light_ambient[2], System::light_ambient[3]);
			world.light_view_matrix =
				glm::lookAt(glm::vec3(System::light_position[0], System::light_position[1], System::light_position[2]),
							glm::vec3(albedo::CameraManager::camera_lookat[0], albedo::CameraManager::camera_lookat[1],
									  albedo::CameraManager::camera_lookat[2]),
							glm::vec3(albedo::CameraManager::camera_up[0], albedo::CameraManager::camera_up[1],
									  -albedo::CameraManager::camera_up[2]));
			world.light_projection_matrix = glm::ortho<float>(-10, 10, -10, 10, -100, 100);
			world.light_intensity		  = System::light_intensity;

			world.is_enabled_shadow_mapping = System::is_enabled_shadow_mapping;
			world.shadow_mapping_bias		= System::shadow_mapping_bias;
		}
		void update_local(Local in_local) {
			local.model			 = in_local.model;
			local.specular_power = in_local.specular_power;
		}
	};
} // namespace albedo