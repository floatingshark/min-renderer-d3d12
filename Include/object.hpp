#pragma once
#include "shape.hpp"
#include "texture.hpp"
#include <cassert>
#include <d3d12.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vector>
#include <wrl/client.h>

namespace albedos {
	class Object {
	public:
		Object(ID3D12Device* device, ID3D12DescriptorHeap* heap_cbv) : device(device), descriptor_heap_cbv(heap_cbv) {
			init_vertex();
			init_texture();
			init_directx_buffer();
		};

	private:
		UINT CBUFFER_SIZE = 512;
		UINT TEXTURE_SIZE = 1024;

	protected:
		ID3D12Device*										device;
		ID3D12DescriptorHeap*								descriptor_heap_cbv;
		Microsoft::WRL::ComPtr<ID3D12Resource>				vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				index_buffer;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> constant_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				texture_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				shadow_buffer;
		std::vector<Shape::Vertex>							vertex_data;
		std::vector<int>									index_data;
		std::vector<byte>									texture_data;

	public:
		std::string	  name;
		glm::vec3	  position		   = {0.f, 0.f, 0.f};
		glm::vec3	  rotation		   = {0.f, 0.f, 0.f};
		glm::vec3	  scale			   = {1.f, 1.f, 1.f};
		Texture::Type texture_type	   = Texture::Type::Monochrome;
		float		  texture_color[4] = {1.f, 1.f, 1.f, 1.f};
		float		  specular_power   = 100.f;

	public:
		void init_vertex() { albedos::Shape::create_plane(vertex_data, index_data); }
		void init_texture() {
			byte color[4] = {255, 255, 255, 255};
			texture_data  = albedos::Texture::create_monochromatic(TEXTURE_SIZE, color);
		}
		void init_directx_buffer() {
			// Create Buffers
			HRESULT				  hr;
			D3D12_HEAP_PROPERTIES heap_properties{};
			D3D12_RESOURCE_DESC	  resource_desc{};

			heap_properties.Type				 = D3D12_HEAP_TYPE_UPLOAD;
			heap_properties.CPUPageProperty		 = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_properties.CreationNodeMask	 = 1;
			heap_properties.VisibleNodeMask		 = 1;

			resource_desc.Dimension			 = D3D12_RESOURCE_DIMENSION_BUFFER;
			resource_desc.Width				 = sizeof(Shape::Vertex) * vertex_data.size();
			resource_desc.Height			 = 1;
			resource_desc.DepthOrArraySize	 = 1;
			resource_desc.MipLevels			 = 1;
			resource_desc.Format			 = DXGI_FORMAT_UNKNOWN;
			resource_desc.Layout			 = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resource_desc.SampleDesc.Count	 = 1;
			resource_desc.SampleDesc.Quality = 0;

			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												 IID_PPV_ARGS(&vertex_buffer));
			assert(SUCCEEDED(hr) && "Create Committed Resource Vertex Buffer");

			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												 IID_PPV_ARGS(&index_buffer));
			assert(SUCCEEDED(hr) && "Create Committed Resource Index Buffer");

			resource_desc.Width		 = CBUFFER_SIZE;
			constexpr int cbuff_size = 2;
			constant_buffer.resize(cbuff_size);
			for (int i = 0; i < 2; i++) {
				hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
													 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
													 IID_PPV_ARGS(&constant_buffer[i]));
				assert(SUCCEEDED(hr) && "Create Committed Resource Constant Buffer");
			}

			heap_properties.Type				 = D3D12_HEAP_TYPE_CUSTOM;
			heap_properties.CPUPageProperty		 = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
			heap_properties.CreationNodeMask	 = 0;
			heap_properties.VisibleNodeMask		 = 0;
			resource_desc.Dimension				 = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resource_desc.Width					 = TEXTURE_SIZE;
			resource_desc.Height				 = TEXTURE_SIZE;
			resource_desc.Format				 = DXGI_FORMAT_R8G8B8A8_UNORM;
			resource_desc.Layout				 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												 IID_PPV_ARGS(&texture_buffer));
			assert(SUCCEEDED(hr) && "Create Committed Resource Texture Buffer");

			// Init Buffers
			void*		 Mapped;
			const UINT	 size_vertices = sizeof(Shape::Vertex) * vertex_data.size();
			const size_t size_indices  = sizeof(int) * index_data.size();

			hr = vertex_buffer->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Fialed Vertex Buffer Mapping");
			CopyMemory(Mapped, vertex_data.data(), size_vertices);
			vertex_buffer->Unmap(0, nullptr);
			Mapped = nullptr;

			hr = index_buffer->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Failed Index Buffer Mapping");
			CopyMemory(Mapped, index_data.data(), size_indices);
			index_buffer->Unmap(0, nullptr);
			Mapped = nullptr;

			const D3D12_BOX box = {0, 0, 0, TEXTURE_SIZE, TEXTURE_SIZE, 1};
			hr					= texture_buffer->WriteToSubresource(0, &box, texture_data.data(), 4 * TEXTURE_SIZE,
																	 4 * TEXTURE_SIZE * TEXTURE_SIZE);
			assert(SUCCEEDED(hr) && "Write to Subrecource");
		};

		// Create Resource Views and Draw
		void update_draw_directx(ID3D12GraphicsCommandList* command_list, int index, int num_buffers) {
			UINT					 size_vertices = sizeof(Shape::Vertex) * vertex_data.size();
			const size_t			 size_indices  = sizeof(int) * index_data.size();
			D3D12_VERTEX_BUFFER_VIEW vertex_view{};
			D3D12_INDEX_BUFFER_VIEW	 index_view{};

			// Vertex View
			vertex_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
			vertex_view.StrideInBytes  = sizeof(Shape::Vertex); // Size of 1 Vertex
			vertex_view.SizeInBytes	   = size_vertices;			// Size of All Vertices

			// Index View
			index_view.BufferLocation = index_buffer->GetGPUVirtualAddress();
			index_view.Format		  = DXGI_FORMAT_R32_UINT;
			index_view.SizeInBytes	  = size_indices;

			// Consntant Buffer View Scene (Index 1)
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbuff_desc = {};
			D3D12_CPU_DESCRIPTOR_HANDLE		cbv_handle = descriptor_heap_cbv->GetCPUDescriptorHandleForHeapStart();
			UINT cbv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			cbuff_desc.BufferLocation = constant_buffer[0]->GetGPUVirtualAddress();
			cbuff_desc.SizeInBytes	  = CBUFFER_SIZE;
			cbv_handle.ptr += index * num_buffers * cbv_descriptor_size;
			device->CreateConstantBufferView(&cbuff_desc, cbv_handle);
			// Consntant Buffer View Local (Index 2)
			cbuff_desc.BufferLocation = constant_buffer[1]->GetGPUVirtualAddress();
			cbv_handle.ptr += cbv_descriptor_size;
			device->CreateConstantBufferView(&cbuff_desc, cbv_handle);
			// Shader Resource View Texture (Index 3)
			D3D12_SHADER_RESOURCE_VIEW_DESC tex_desc{};
			tex_desc.Format						   = DXGI_FORMAT_R8G8B8A8_UNORM;
			tex_desc.ViewDimension				   = D3D12_SRV_DIMENSION_TEXTURE2D;
			tex_desc.Texture2D.MipLevels		   = 1;
			tex_desc.Texture2D.MostDetailedMip	   = 0;
			tex_desc.Texture2D.PlaneSlice		   = 0;
			tex_desc.Texture2D.ResourceMinLODClamp = 0.0F;
			tex_desc.Shader4ComponentMapping	   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			cbv_handle.ptr += cbv_descriptor_size;
			device->CreateShaderResourceView(texture_buffer.Get(), &tex_desc, cbv_handle);
			// Shader Resource View Depth Texture (Index 4)
			tex_desc.Format = DXGI_FORMAT_R32_FLOAT;
			cbv_handle.ptr += cbv_descriptor_size;
			device->CreateShaderResourceView(shadow_buffer.Get(), &tex_desc, cbv_handle);

			command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			command_list->IASetVertexBuffers(0, 1, &vertex_view);
			command_list->IASetIndexBuffer(&index_view);

			command_list->DrawIndexedInstanced(index_data.size(), 1, 0, 0, 0);
		}
		// Constant Buffer 1 supports Scene Mutual Variables
		void update_constant_buffer_1(Constant::Scene scene) {
			HRESULT hr;
			void*	Mapped;

			//calculate_scene(scene);

			hr = constant_buffer[0]->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Constant Buffer Mappded[Scene]");
			CopyMemory(Mapped, &scene, sizeof(scene));
			constant_buffer[0]->Unmap(0, nullptr);
			Mapped = nullptr;
		}
		// Constant Buffer 2 supports Object Unique Variables
		void update_constant_buffer_2(Constant::Local object) {
			HRESULT hr;
			void*	Mapped;

			calculate_local(object);

			hr = constant_buffer[1]->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Constant Buffer Mappded[Object]");
			CopyMemory(Mapped, &object, sizeof(object));
			constant_buffer[1]->Unmap(0, nullptr);
			Mapped = nullptr;
		}

		void set_shadow_buffer(ID3D12Resource* in_buffer) { shadow_buffer = in_buffer; }
		void set_vertex_data(Shape::Type in_type) {
			vertex_data.clear();
			index_data.clear();

			switch (in_type) {
			case Shape::Type::Plane:
				albedos::Shape::create_plane(vertex_data, index_data);
				break;
			case Shape::Type::Cube:
				albedos::Shape::create_cube(vertex_data, index_data);
				break;
			case Shape::Type::Torus:
				albedos::Shape::create_torus(vertex_data, index_data);
				break;
			default:
				break;
			}

			init_directx_buffer();
		}
		void set_texture_data(Texture::Type in_type) {
			texture_data.clear();
			texture_type = in_type;
			
			HRESULT	  hr;
			const int checker_num = 8;
			byte	  color[4]	  = {(byte)(texture_color[0] * 255.f), (byte)(texture_color[1] * 255.f),
									 (byte)(texture_color[2] * 255.f), (byte)(texture_color[3] * 255.f)};

			switch (in_type) {
			case Texture::Type::Monochrome:
				texture_data = Texture::create_monochromatic(TEXTURE_SIZE, color);
				break;
			case Texture::Type::CheckBoard:
				texture_data = Texture::create_checker_board(TEXTURE_SIZE, color, checker_num);
				break;
			default:
				break;
			}

			const D3D12_BOX box = {0, 0, 0, TEXTURE_SIZE, TEXTURE_SIZE, 1};
			hr					= texture_buffer->WriteToSubresource(0, &box, texture_data.data(), 4 * TEXTURE_SIZE,
																	 4 * TEXTURE_SIZE * TEXTURE_SIZE);
			assert(SUCCEEDED(hr) && "Write to Subrecource");
		}

	protected:
		//void calculate_scene(Constant::Scene& scene) {}
		void calculate_local(Constant::Local& local) {
			local.world			 = glm::translate(local.world, position);
			local.world			 = glm::rotate(local.world, rotation[0], {1.f, 0.f, 0.f});
			local.world			 = glm::rotate(local.world, rotation[1], {0.f, 1.f, 0.f});
			local.world			 = glm::rotate(local.world, rotation[2], {0.f, 0.f, 1.f});
			local.world			 = glm::scale(local.world, scale);
			local.specular_power = specular_power;
		}
	};
} // namespace albedos