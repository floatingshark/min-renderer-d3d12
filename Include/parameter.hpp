#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

namespace arabesque
{
	class Parameter
	{
	public:
		typedef struct Constant
		{
			glm::mat4x4 world;
			glm::mat4x4 view;
			glm::mat4x4 projection;
		} Constant;

		Parameter() { init_constant(); }
	protected:
		Constant Const;

	public:
		void init_constant()
		{
			Const.world = glm::mat4(1.f);
			Const.world = glm::translate(Const.world, glm::vec3(0.5f, 0.0f, 0.0f));
			Const.world = glm::rotate(Const.world, glm::pi<float>() * 0.05f, glm::vec3(0.f, 1.f, 0.f)); 
			Const.world = glm::scale(Const.world, glm::vec3(0.7f));
			Const.view = glm::lookAt(glm::vec3(0.f, 0.f, 0.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
			Const.projection = glm::perspective(60.0f, 4.0f / 3.0f, 0.01f, 100.f);
			std::cout << glm::to_string(Const.projection) << std::endl;
		}
		Parameter::Constant get_constant()
		{
			return Const;
		}
	};
}