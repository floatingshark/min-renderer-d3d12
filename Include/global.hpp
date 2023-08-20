#pragma once
#include <iostream>
#include <cassert>
#include <vector>

namespace arabesques
{
	class Global
	{
	public:
		static float color[4];
		static float view_position[3];
		static float lookat[3];
		static float up[3];
		static float FOV;
	};
	float Global::color[4] = {0.725f, 0.788f, 0.788f, 1.f};
	float Global::view_position[3] = {0.f, 0.f, 0.5f};
	float Global::lookat[3] = {0.f, 0.f, 0.f};
	float Global::up[3] = {0.f, -1.f, 0.f};
	float Global::FOV = 60.f;
}