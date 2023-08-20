#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

namespace arabesque
{
	class Param
	{
	public:
		typedef struct Constant
		{
			glm::mat4x4 world;
			glm::mat4x4 view;
			glm::mat4x4 projection;
		} Constant;

		Param() { init_constant(); }
	protected:
		Constant contant;

	public:
		void init_constant()
		{
			contant.world = glm::mat4(1.f);
			contant.world = glm::translate(contant.world, glm::vec3(0.5f, 0.0f, 0.0f));
			contant.world = glm::rotate(contant.world, glm::pi<float>() * 0.1f, glm::vec3(0.f, 1.f, 0.f)); 
			contant.world = glm::scale(contant.world, glm::vec3(0.7f));
			contant.view = glm::lookAt(glm::vec3(0.f, 0.f, 0.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
			contant.projection = glm::perspective(60.0f, 4.0f / 3.0f, 0.01f, 100.f);
		}
		Param::Constant& get_constant()
		{
			return contant;
		}
	};
}