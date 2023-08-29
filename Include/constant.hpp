#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "global.hpp"

namespace albedos
{
	class Constant
	{
	public:
		typedef struct Scene
		{
			glm::mat4x4 view;
			glm::mat4x4 projection;
			glm::vec4 view_pos;
			glm::vec4 light_pos;
			glm::vec4 light_ambient;
			float light_intensity;
		} Scene;

		typedef struct Local
		{
			glm::mat4x4 world;
			int use_texture;
			float specular; // Specular Power
		} Local;

		Constant() { init(); }

	protected:
		Scene scene;
		Local local;

	public:
		void init()
		{

			scene.view = glm::lookAt(glm::vec3(0.f, 0.f, 0.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
			scene.projection = glm::perspective(60.0f, 4.0f / 3.0f, 0.01f, 100.f);
			scene.light_pos = glm::vec4(0.f, 10.f, 0.f, 1.f);
			scene.view_pos = glm::vec4(0.f, 0.f, 0.5f, 1.f);

			local.world = glm::mat4(1.f);
			local.use_texture = (int)false;
			local.specular = 1.f;
		}
		void calculate_scene()
		{
			scene.view = glm::lookAt(
				glm::vec3(Global::view_position[0], Global::view_position[1], Global::view_position[2]),
				glm::vec3(Global::view_lookat[0], Global::view_lookat[1], Global::view_lookat[2]),
				glm::vec3(Global::view_up[0], Global::view_up[1], Global::view_up[2]));
			scene.projection = glm::perspective(
				glm::radians(Global::projection_FOV),
				4.f / 3.f,
				0.01f,
				100.f);
			scene.light_pos = glm::vec4(Global::light_position[0], Global::light_position[1], Global::light_position[2], 1.f);
			scene.view_pos = glm::vec4(Global::view_position[0], Global::view_position[1], Global::view_position[2], 1.f);
			scene.light_ambient = glm::vec4(Global::light_ambient[0], Global::light_ambient[1], Global::light_ambient[2], Global::light_ambient[3]);
			scene.light_intensity = Global::light_intensity;
		}
		inline Constant::Scene &get_scene()
		{
			return scene;
		}
		inline Constant::Local &get_local()
		{
			return local;
		}
	};
}