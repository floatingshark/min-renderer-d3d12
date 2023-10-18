#pragma once
#include "directx_constant.hpp"
#include "entity_directx.hpp"
#include "entity_shaders.hpp"
#include "entity_shapes.hpp"
#include "entity_texture.hpp"
#include "system_variables.hpp"
#include <External/glm/glm.hpp>
#include <cassert>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>
#include <wrl/client.h>

namespace albedo {
	class Entity {
	public:
		Entity() {
			albedo::Shape::create_plane(vertex_data, index_data);
			byte color[4] = {255, 255, 255, 255};
			texture_data  = albedo::Texture::create_monochromatic(System::texture_size, color);
		};

		std::shared_ptr<EntityDirectX> entity_directx = nullptr;

		std::string			  name				 = "";
		albedo::Shaders::Type shader_type		 = albedo::Shaders::Type::Color;
		albedo::Texture::Type texture_type		 = albedo::Texture::Type::Monochrome;
		float				  texture_color[4]	 = {1.f, 1.f, 1.f, 1.f};
		std::string			  texture_file_name	 = "";
		std::string			  cube_map_file_name = "";

		std::vector<Shape::Vertex>	   vertex_data;
		std::vector<int>			   index_data;
		std::vector<byte>			   texture_data;
		std::vector<std::vector<byte>> cube_map_data;

		glm::vec3 position		 = {0.f, 0.f, 0.f};
		glm::vec3 rotation		 = {0.f, 0.f, 0.f};
		glm::vec3 scale			 = {1.f, 1.f, 1.f};
		float	  specular_power = 100.f;

	public:
		void set_shadow_buffer(ID3D12Resource* in_resource) { entity_directx->shadow_resource = in_resource; }
		void set_vertex_type(albedo::Shape::Type in_type) {
			vertex_data.clear();
			index_data.clear();

			switch (in_type) {
			case Shape::Type::Plane:
				albedo::Shape::create_plane(vertex_data, index_data);
				break;
			case Shape::Type::Cube:
				albedo::Shape::create_cube(vertex_data, index_data);
				break;
			case Shape::Type::Torus:
				albedo::Shape::create_torus(vertex_data, index_data);
				break;
			default:
				break;
			}
		}
		void set_texture_type(albedo::Texture::Type in_type) {
			texture_data.clear();
			texture_type = in_type;

			HRESULT	  hr;
			const int checker_num = 8;
			byte	  color[4]	  = {(byte)(texture_color[0] * 255.f), (byte)(texture_color[1] * 255.f),
									 (byte)(texture_color[2] * 255.f), (byte)(texture_color[3] * 255.f)};

			switch (in_type) {
			case Texture::Type::Monochrome:
				texture_data = Texture::create_monochromatic(System::texture_size, color);
				break;
			case Texture::Type::CheckerBoard:
				texture_data = Texture::create_checker_board(System::texture_size, color, checker_num);
				break;
			case Texture::Type::Image:
				Texture::read_bmp_file(texture_file_name.c_str(), texture_data);
				break;
			default:
				break;
			}
		}
		void set_cubemap_data(const char* in_file_name) {
			HRESULT hr;

			const UINT width  = System::texture_size;
			const UINT height = System::texture_size;

			cube_map_data.clear();
			Texture::read_bmp_cube_file(in_file_name, cube_map_data, width);
		}

		void change_shader(albedo::Shaders::Type in_type) {
			shader_type = in_type;
			if (entity_directx) {
				entity_directx->initialize_pipeline_state(shader_type);
			}
		}
		void change_render_pipeline() {
			if (entity_directx) {
				entity_directx->initialize_pipeline_state(shader_type);
			}
		}

		void update() {
			if (entity_directx) {
				entity_directx->constant.update_world();
				albedo::DirectXConstant::Local local;
				local.model			 = glm::mat4(1.f);
				local.model			 = glm::translate(local.model, position);
				local.model			 = glm::rotate(local.model, rotation[0], {1.f, 0.f, 0.f});
				local.model			 = glm::rotate(local.model, rotation[1], {0.f, 1.f, 0.f});
				local.model			 = glm::rotate(local.model, rotation[2], {0.f, 0.f, 1.f});
				local.model			 = glm::scale(local.model, scale);
				local.specular_power = specular_power;
				entity_directx->constant.update_local(local);

				entity_directx->update_constant_resources();
			}
		}

		void init_directx_contexts(ID3D12Device* in_device, ID3D12DescriptorHeap* in_heap_cbv) {
			entity_directx						= std::make_shared<EntityDirectX>();
			entity_directx->device				= in_device;
			entity_directx->descriptor_heap_cbv = in_heap_cbv;
			entity_directx->initialize_resources(vertex_data, index_data, texture_data);
			entity_directx->initialize_root_signature();
			entity_directx->initialize_pipeline_state(shader_type);
		};
	};
} // namespace albedo