#pragma once
#include "entity.hpp"
#include "entity_shaders.hpp"
#include "entity_shapes.hpp"
#include "texture.hpp"
#include <d3d12.h>
#include <iostream>
#include <vector>

namespace albedo {
	class World {
	public:
		World(ID3D12Device* device, ID3D12DescriptorHeap* heap) : device(device), heap(heap) {
			create_scene_1();
			create_skydome();
		};

		static std::vector<std::shared_ptr<albedo::Entity>> get_entities() { return entities; }
		static std::shared_ptr<albedo::Entity>				get_skydome_entity() { return skydome_entity; }

		ID3D12Device*		  device;
		ID3D12DescriptorHeap* heap;

	private:
		static std::vector<std::shared_ptr<albedo::Entity>> entities;
		static std::shared_ptr<albedo::Entity>				skydome_entity;

		void create_scene_1() {
			std::shared_ptr<albedo::Entity> object_1 = std::make_shared<albedo::Entity>(device, heap);
			std::shared_ptr<albedo::Entity> object_2 = std::make_shared<albedo::Entity>(device, heap);

			object_1->name	   = "Scene1 Torus";
			object_1->position = {0.f, 0.f, 1.5f};
			object_1->rotation = {0.3f, 0.2f, 0.f};
			object_1->set_vertex_data(albedo::Shape::Type::Torus);
			object_1->reset_directx_shader(albedo::Shaders::Type::Phong);

			object_2->name				= "Scene1 Plane";
			object_2->scale				= {5.f, 5.f, 1.f};
			object_2->texture_color[0]	= 0.7f;
			object_2->texture_color[1]	= 0.7f;
			object_2->texture_color[2]	= 0.7f;
			object_2->texture_file_name = "Resource/polystyrene_diff_1k.bmp";
			object_2->set_texture_data(albedo::Texture::Type::Image);
			object_2->reset_directx_shader(albedo::Shaders::Type::Phong);

			entities = {object_1, object_2};
		}
		void create_skydome() {
			skydome_entity			 = std::make_shared<albedo::Entity>(device, heap);
			skydome_entity->name	 = "Sky Cube";
			skydome_entity->position = {0.f, 0.f, 0.f};
			skydome_entity->scale	 = {30.f, 30.f, 30.f};
			skydome_entity->set_vertex_data(albedo::Shape::Type::Cube);
			skydome_entity->set_cubemap_data("Resource/studio_garden_4k.bmp");
			skydome_entity->reset_directx_shader(albedo::Shaders::Type::Skydome);
		}
	};

	std::vector<std::shared_ptr<albedo::Entity>> World::entities;
	std::shared_ptr<albedo::Entity>				 World::skydome_entity;
} // namespace albedo