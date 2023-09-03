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
		Scene(ID3D12Device* device, ID3D12DescriptorHeap* heap) : device(device), heap(heap) { create_scene_1(); };

		ID3D12Device*				 device;
		ID3D12DescriptorHeap*		 heap;
		std::vector<albedos::Object> objects;

		/** 
		 * Scene 1
		 * 1 Torus and 1 Plane
		 */
		void create_scene_1() {
			albedos::Object object_1 = albedos::Object(device, heap);
			albedos::Object object_2 = albedos::Object(device, heap);

			object_1.name	  = "Default Torus";
			object_1.position = {0.f, 0.f, 1.0f};
			object_1.set_vertex_data(albedos::Shape::Type::Torus);

			object_2.name			  = "Default Plane";
			object_2.scale			  = {3.f, 3.f, 1.f};
			object_2.texture_color[0] = 0.7f;
			object_2.texture_color[1] = 0.7f;
			object_2.texture_color[2] = 0.7f;
			object_2.set_texture_data(albedos::Texture::Type::CheckBoard);

			objects = {object_1, object_2};
		}
	};
} // namespace albedos