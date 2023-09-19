#pragma once
#include "object.hpp"
#include "shaders.hpp"
#include "shape.hpp"
#include "texture.hpp"
#include <d3d12.h>
#include <iostream>
#include <vector>

namespace albedo {
	class Scene {
	public:
		Scene(ID3D12Device* device, ID3D12DescriptorHeap* heap) : device(device), heap(heap) {
			create_scene_1();
			create_skydome();
		};

	private:
		ID3D12Device*		  device;
		ID3D12DescriptorHeap* heap;

	public:
		std::vector<std::shared_ptr<albedo::Object>> render_objects;
		std::shared_ptr<albedo::Object>			  skydome_object;

	private:
		/**
		 * Scene 1
		 * 1 Torus and 1 Plane
		 */
		void create_scene_1() {
			std::shared_ptr<albedo::Object> object_1 = std::make_shared<albedo::Object>(device, heap);
			std::shared_ptr<albedo::Object> object_2 = std::make_shared<albedo::Object>(device, heap);

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

			render_objects = {object_1, object_2};
		}
		/**
		 * Skydome exists everty scene
		 */
		void create_skydome() {
			skydome_object			 = std::make_shared<albedo::Object>(device, heap);
			skydome_object->name	 = "Sky Cube";
			skydome_object->position = {0.f, 0.f, 0.f};
			skydome_object->scale	 = {30.f, 30.f, 30.f};
			skydome_object->set_vertex_data(albedo::Shape::Type::Cube);
			skydome_object->set_cubemap_data("Resource/studio_garden_4k.bmp");
			skydome_object->reset_directx_shader(albedo::Shaders::Type::Skydome);
		}
	};
} // namespace albedo