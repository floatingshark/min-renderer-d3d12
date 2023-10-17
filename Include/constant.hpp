#pragma once
#include "global.hpp"
#include <External/glm/glm.hpp>
#include <External/glm/gtc/matrix_transform.hpp>
#include <External/glm/gtx/string_cast.hpp>
#include <cassert>
#include <iostream>
#include <vector>

namespace albedo {
	class Constant {
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
			glm::mat4x4 world;
			float		specular_power;
		} Local;

		Constant() { init(); }

	protected:
		World scene;
		Local local;

	public:
		void init() {

			scene.view_matrix =
				glm::lookAt(glm::vec3(0.f, 0.f, 0.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
			scene.projection_matrix			= glm::perspective(60.0f, 4.0f / 3.0f, 0.01f, 100.f);
			scene.view_position				= glm::vec4(0.f, 0.f, 0.5f, 1.f);
			scene.light_position			= glm::vec4(0.f, 10.f, 0.f, 1.f);
			scene.light_ambient				= {0.f, 0.f, 0.f, 0.f};
			scene.light_view_matrix			= glm::mat4(1.f);
			scene.light_projection_matrix	= glm::mat4(1.f);
			scene.light_intensity			= 1.f;
			scene.is_enabled_shadow_mapping = false;
			scene.shadow_mapping_bias		= 0.005f;

			local.world			 = glm::mat4(1.f);
			local.specular_power = 1.f;
		}
		void update() {
			scene.view_matrix =
				glm::lookAt(glm::vec3(Global::view_position[0], Global::view_position[1], Global::view_position[2]),
							glm::vec3(Global::view_lookat[0], Global::view_lookat[1], Global::view_lookat[2]),
							glm::vec3(Global::view_up[0], Global::view_up[1], Global::view_up[2]));
			scene.projection_matrix = glm::perspective(glm::radians(Global::projection_FOV), ((float)Global::window_width / (float)Global::window_height),
													   Global::projection_near, Global::projection_far);
			scene.view_position =
				glm::vec4(Global::view_position[0], Global::view_position[1], Global::view_position[2], 1.f);

			scene.light_position =
				glm::vec4(Global::light_position[0], Global::light_position[1], Global::light_position[2], 1.f);
			scene.light_ambient = glm::vec4(Global::light_ambient[0], Global::light_ambient[1],
											Global::light_ambient[2], Global::light_ambient[3]);
			scene.light_view_matrix =
				glm::lookAt(glm::vec3(Global::light_position[0], Global::light_position[1], Global::light_position[2]),
							glm::vec3(Global::view_lookat[0], Global::view_lookat[1], Global::view_lookat[2]),
							glm::vec3(Global::view_up[0], Global::view_up[1], -Global::view_up[2]));
			scene.light_projection_matrix = glm::ortho<float>(-10, 10, -10, 10, -100, 100);
			scene.light_intensity		  = Global::light_intensity;

			scene.is_enabled_shadow_mapping = Global::is_enabled_shadow_mapping;
			scene.shadow_mapping_bias		= Global::shadow_mapping_bias;
		}

		inline Constant::World& get_scene() { return scene; }
		inline Constant::Local& get_local() { return local; }
	};
} // namespace albedo