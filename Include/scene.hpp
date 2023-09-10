#pragma once
#include "object.hpp"
#include "shape.hpp"
#include "texture.hpp"
#include <d3d12.h>
#include <iostream>
#include <vector>

namespace albedos {
	class Scene {
	public:
		Scene(ID3D12Device* device, ID3D12DescriptorHeap* heap) : device(device), heap(heap) {
			create_scene_1();
			create_skydome();
		};

		ID3D12Device*								  device;
		ID3D12DescriptorHeap*						  heap;
		std::vector<std::shared_ptr<albedos::Object>> render_objects;
		std::shared_ptr<albedos::Object>			  skydome;

		/**
		 * Scene 1
		 * 1 Torus and 1 Plane
		 */
		void create_scene_1() {
			std::shared_ptr<albedos::Object> object_1 = std::make_shared<albedos::Object>(device, heap);
			std::shared_ptr<albedos::Object> object_2 = std::make_shared<albedos::Object>(device, heap);

			object_1->name	   = "Scene1 Torus";
			object_1->position = {0.f, 0.f, 1.5f};
			object_1->rotation = {0.3f, 0.2f, 0.f};
			object_1->set_vertex_data(albedos::Shape::Type::Torus);

			object_2->name				= "Scene1 Plane";
			object_2->scale				= {5.f, 5.f, 1.f};
			object_2->texture_color[0]	= 0.7f;
			object_2->texture_color[1]	= 0.7f;
			object_2->texture_color[2]	= 0.7f;
			object_2->texture_file_name = "Resource/polystyrene_diff_1k.bmp";
			object_2->set_texture_data(albedos::Texture::Type::Image);

			render_objects = {object_1, object_2};
		}

		void create_skydome() {
			skydome			  = std::make_shared<albedos::Object>(device, heap);
			skydome->name	  = "Sky Cube";
			skydome->position = {0.f, 0.f, 0.f};
			skydome->scale	  = {30.f, 30.f, 30.f};
			skydome->set_vertex_data(albedos::Shape::Type::Cube);
			skydome->set_cubemap_data("Resource/studio_garden_4k.bmp");
		}
	};
} // namespace albedos