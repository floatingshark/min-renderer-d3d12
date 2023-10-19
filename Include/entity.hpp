#pragma once
#include "Components/directx_component.hpp"
#include "Components/map_component.hpp"
#include "Components/shader_component.hpp"
#include "Components/shape_component.hpp"
#include "DirectX/directx_constant.hpp"
#include "system_variables.hpp"
#include <External/glm/glm.hpp>
#include <cassert>
#include <iostream>
#include <vector>
#include <wrl/client.h>

namespace albedo {
	class Entity {
	public:
		Entity() {
			shape_component	 = std::make_shared<ShapeComponent>();
			map_component	 = std::make_shared<MapComponent>();
			shader_component = std::make_shared<ShaderComponent>();
		};

		std::shared_ptr<ShapeComponent>	  shape_component	= nullptr;
		std::shared_ptr<MapComponent>	  map_component		= nullptr;
		std::shared_ptr<ShaderComponent>  shader_component	= nullptr;
		std::shared_ptr<DirectXComponent> directx_component = nullptr;

		std::string name = "";

		glm::vec3 position		 = {0.f, 0.f, 0.f};
		glm::vec3 rotation		 = {0.f, 0.f, 0.f};
		glm::vec3 scale			 = {1.f, 1.f, 1.f};
		float	  specular_power = 100.f;

	public:
		void set_vertex_type(albedo::ShapeComponent::Type in_type) {
			shape_component->vertex_data.clear();
			shape_component->index_data.clear();

			switch (in_type) {
			case ShapeComponent::Type::Plane:
				albedo::ShapeComponent::create_plane(shape_component->vertex_data, shape_component->index_data);
				break;
			case ShapeComponent::Type::Cube:
				albedo::ShapeComponent::create_cube(shape_component->vertex_data, shape_component->index_data);
				break;
			case ShapeComponent::Type::Torus:
				albedo::ShapeComponent::create_torus(shape_component->vertex_data, shape_component->index_data);
				break;
			default:
				break;
			}
		}
		void set_texture_type(albedo::MapComponent::TextureType in_type) {
			map_component->texture_data.clear();
			map_component->texture_type = in_type;

			switch (in_type) {
			case MapComponent::TextureType::Monochrome:
				map_component->texture_data =
					MapComponent::create_monochromatic(map_component->map_size, map_component->map_color);
				break;
			case MapComponent::TextureType::Image:
				MapComponent::read_bmp_file(map_component->texture_file_name.c_str(), map_component->texture_data);
				break;
			default:
				break;
			}
		}
		void set_cubemap_data(const char* in_file_name) {
			const int width = map_component->map_size;
			map_component->cube_map_data.clear();
			MapComponent::read_bmp_cube_file(in_file_name, map_component->cube_map_data, width);
		}

		void change_shader(albedo::ShaderComponent::ShaderType in_type) {
			shader_component->type = in_type;
			if (directx_component) {
				directx_component->initialize_pipeline_state(in_type);
			}
		}
		void change_render_pipeline() {
			if (directx_component) {
				directx_component->initialize_pipeline_state(shader_component->type);
			}
		}

		void update() {
			if (directx_component) {
				directx_component->constant.update_world();
				albedo::DirectXConstant::Local local;
				local.model			 = glm::mat4(1.f);
				local.model			 = glm::translate(local.model, position);
				local.model			 = glm::rotate(local.model, rotation[0], {1.f, 0.f, 0.f});
				local.model			 = glm::rotate(local.model, rotation[1], {0.f, 1.f, 0.f});
				local.model			 = glm::rotate(local.model, rotation[2], {0.f, 0.f, 1.f});
				local.model			 = glm::scale(local.model, scale);
				local.specular_power = specular_power;
				directx_component->constant.update_local(local);

				directx_component->update_constant_resources();
			}
		}

		void init_directx_contexts(ID3D12Device* in_device, ID3D12DescriptorHeap* in_heap_cbv) {
			directx_component					   = std::make_shared<DirectXComponent>();
			directx_component->device			   = in_device;
			directx_component->descriptor_heap_cbv = in_heap_cbv;
			directx_component->initialize_resources(shape_component->vertex_data, shape_component->index_data,
													map_component->texture_data);
			directx_component->initialize_root_signature();
			directx_component->initialize_pipeline_state(shader_component->type);
		};
		void init_directx_shadow_buffer(ID3D12Resource* in_resource) {
			if (directx_component) {
				directx_component->shadow_resource = in_resource;
			}
		}
	};
} // namespace albedo