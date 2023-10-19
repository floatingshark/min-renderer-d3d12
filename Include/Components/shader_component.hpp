#pragma once
#include <cassert>
#include <d3d12.h>
#include <fstream>
#include <iostream>
#include <vector>

namespace albedo {
	class ShaderComponent {
	public:
		enum struct ShaderType { Color, Phong, Skydome, Max };

		ShaderType type = ShaderType::Color;

	public:
		static const wchar_t* get_shader_name(albedo::ShaderComponent::ShaderType in_type) {
			switch (in_type) {
			case albedo::ShaderComponent::ShaderType::Color:
				return L"./Source/Shader/ColorShaders.hlsl";
			case albedo::ShaderComponent::ShaderType::Phong:
				return L"./Source/Shader/PhongShaders.hlsl";
			case albedo::ShaderComponent::ShaderType::Skydome:
				return L"./Source/Shader/SkydomeShaders.hlsl";
			default:
				return L"./Source/Shader/ColorShaders.hlsl";
			}
		}
	};
} // namespace albedo