#pragma once
#include <cassert>
#include <d3d12.h>
#include <fstream>
#include <iostream>
#include <vector>

namespace albedo {
	class Shaders {
	public:
		enum struct Type { Color, Phong, Skydome, Max };

	public:
		static const wchar_t* get_shader_name(albedo::Shaders::Type in_type) {
			switch (in_type) {
			case albedo::Shaders::Type::Color:
				return L"./Source/Shader/ColorShaders.hlsl";
			case albedo::Shaders::Type::Phong:
				return L"./Source/Shader/PhongShaders.hlsl";
			case albedo::Shaders::Type::Skydome:
				return L"./Source/Shader/SkydomeShaders.hlsl";
			default:
				return L"./Source/Shader/ColorShaders.hlsl";
			}
		}
	};
} // namespace albedo