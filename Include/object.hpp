#pragma once
#include <iostream>
#include <cassert>
#include <vector>
#include <wrl/client.h>
#include <d3d12.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "shape.hpp"
#include "texture.hpp"

namespace arabesques
{
	class Object
	{
	public:
		Object(ID3D12Device *device, ID3D12DescriptorHeap *cbv_heap, std::string name, Shape::Type type = Shape::Type::Torus)
			: device(device), cbv_heap(cbv_heap), name(name)
		{
			init_vertex(type);
			init_texture();
			init_directx_buffer(device, cbv_heap);
		};

	private:
		ID3D12Device *device;
		ID3D12DescriptorHeap *cbv_heap;

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> constant_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> tex_buffer;
		std::vector<Shape::Vertex> vertices;
		std::vector<int> indices;
		std::vector<byte> texture;
		UINT texture_size = 1024;

	public:
		std::string name;
		int use_texture = 0;
		glm::vec3 position = {0.f, 0.f, 0.f};
		glm::vec3 rotation = {0.f, 0.f, 0.f};
		glm::vec3 scale = {1.f, 1.f, 1.f};

	public:
		void init_vertex(Shape::Type type = Shape::Type::Torus)
		{
			switch (type)
			{
			case Shape::Type::Plane:
				arabesques::Shape::create_plane(vertices, indices);
				break;
			case Shape::Type::Cube:
				arabesques::Shape::create_cube(vertices, indices);
				break;
			case Shape::Type::Torus:
				arabesques::Shape::create_torus(vertices, indices);
				break;
			default:
				arabesques::Shape::create_torus(vertices, indices);
				break;
			}
		}
		void init_texture()
		{
			texture = arabesques::Texture::create_checker(texture_size, 4);
		}
		void init_directx_buffer(ID3D12Device *device, ID3D12DescriptorHeap *cbv_heap)
		{
			// Create Buffers
			HRESULT hr;
			D3D12_HEAP_PROPERTIES heap_properties{};
			D3D12_RESOURCE_DESC resource_desc{};

			heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
			heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_properties.CreationNodeMask = 1;
			heap_properties.VisibleNodeMask = 1;

			resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resource_desc.Width = sizeof(Shape::Vertex) * vertices.size();
			resource_desc.Height = 1;
			resource_desc.DepthOrArraySize = 1;
			resource_desc.MipLevels = 1;
			resource_desc.Format = DXGI_FORMAT_UNKNOWN;
			resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resource_desc.SampleDesc.Count = 1;
			resource_desc.SampleDesc.Quality = 0;

			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer));
			assert(SUCCEEDED(hr) && "Create Vertex Buffer");

			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&index_buffer));
			assert(SUCCEEDED(hr) && "Create Index Buffer");

			resource_desc.Width = 256;
			constexpr int cbuff_size = 2;
			constant_buffer.resize(cbuff_size);
			for (int i = 0; i < 2; i++)
			{
				hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constant_buffer[i]));
				assert(SUCCEEDED(hr) && "Create Constant Buffer");
			}

			heap_properties.Type = D3D12_HEAP_TYPE_CUSTOM;
			heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
			heap_properties.CreationNodeMask = 0;
			heap_properties.VisibleNodeMask = 0;
			resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resource_desc.Width = texture_size;
			resource_desc.Height = texture_size;
			resource_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&tex_buffer));
			assert(SUCCEEDED(hr) && "Create Texture Buffer");

			// Init Buffers
			void *Mapped;
			const UINT size_vertices = sizeof(Shape::Vertex) * vertices.size();
			const size_t size_indices = sizeof(int) * indices.size();

			hr = vertex_buffer->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Fialed Vertex Buffer Mapping");
			CopyMemory(Mapped, vertices.data(), size_vertices);
			vertex_buffer->Unmap(0, nullptr);
			Mapped = nullptr;

			hr = index_buffer->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Failed Index Buffer Mapping");
			CopyMemory(Mapped, indices.data(), size_indices);
			index_buffer->Unmap(0, nullptr);
			Mapped = nullptr;

			const D3D12_BOX box = {0, 0, 0, texture_size, texture_size, 1};
			hr = tex_buffer->WriteToSubresource(0, &box, texture.data(), 4 * texture_size, 4 * texture_size * texture_size);
			assert(SUCCEEDED(hr) && "Write to Subrecource");
		};
		void pre_draw_directx(ID3D12GraphicsCommandList *command_list, int index, int num_buffers)
		{
			UINT size_vertices = sizeof(Shape::Vertex) * vertices.size();
			const size_t size_indices = sizeof(int) * indices.size();
			D3D12_VERTEX_BUFFER_VIEW vertex_view{};
			D3D12_INDEX_BUFFER_VIEW index_view{};

			// Vertex View
			vertex_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
			vertex_view.StrideInBytes = sizeof(Shape::Vertex); // Size of 1 Vertex
			vertex_view.SizeInBytes = size_vertices;		   // Size of All Vertices

			// Index View
			index_view.BufferLocation = index_buffer->GetGPUVirtualAddress();
			index_view.Format = DXGI_FORMAT_R32_UINT;
			index_view.SizeInBytes = size_indices;

			// Consntant Buffer View
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbuff_desc = {};
			D3D12_CPU_DESCRIPTOR_HANDLE cbv_handle = cbv_heap->GetCPUDescriptorHandleForHeapStart();
			UINT cbv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			cbuff_desc.BufferLocation = constant_buffer[0]->GetGPUVirtualAddress();
			cbuff_desc.SizeInBytes = 256;
			cbv_handle.ptr += index * num_buffers * cbv_descriptor_size;
			device->CreateConstantBufferView(&cbuff_desc, cbv_handle);
			cbuff_desc.BufferLocation = constant_buffer[1]->GetGPUVirtualAddress();
			cbv_handle.ptr += cbv_descriptor_size;
			device->CreateConstantBufferView(&cbuff_desc, cbv_handle);

			// Texture Bufferring
			D3D12_SHADER_RESOURCE_VIEW_DESC tex_desc{};
			tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			tex_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			tex_desc.Texture2D.MipLevels = 1;
			tex_desc.Texture2D.MostDetailedMip = 0;
			tex_desc.Texture2D.PlaneSlice = 0;
			tex_desc.Texture2D.ResourceMinLODClamp = 0.0F;
			tex_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			cbv_handle.ptr += cbv_descriptor_size;
			device->CreateShaderResourceView(tex_buffer.Get(), &tex_desc, cbv_handle);

			command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			command_list->IASetVertexBuffers(0, 1, &vertex_view);
			command_list->IASetIndexBuffer(&index_view);
		}
		void draw_directx(ID3D12GraphicsCommandList *command_list)
		{
			command_list->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
		}

		void map_constant_buffer_1(Constant::WVP wvp)
		{
			HRESULT hr;
			void *Mapped;

			calc_wvp(wvp);

			hr = constant_buffer[0]->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Constant Buffer Mappded[WVP]");
			CopyMemory(Mapped, &wvp, sizeof(wvp));
			constant_buffer[0]->Unmap(0, nullptr);
			Mapped = nullptr;
		}
		void map_constant_buffer_2(Constant::Material &material)
		{
			HRESULT hr;
			void *Mapped;

			calc_material(material);

			hr = constant_buffer[1]->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Constant Buffer Mappded[Material]");
			CopyMemory(Mapped, &material, sizeof(material));
			constant_buffer[1]->Unmap(0, nullptr);
			Mapped = nullptr;
		}

	protected:
		void calc_wvp(Constant::WVP &wvp)
		{
			wvp.world = glm::translate(wvp.world, position);
			wvp.world = glm::rotate(wvp.world, rotation[0], {1.f, 0.f, 0.f});
			wvp.world = glm::rotate(wvp.world, rotation[1], {0.f, 1.f, 0.f});
			wvp.world = glm::rotate(wvp.world, rotation[2], {0.f, 0.f, 1.f});
			wvp.world = glm::scale(wvp.world, scale);
		}
		void calc_material(Constant::Material &mat)
		{
			mat.use_texture = (int)use_texture;
		}
	};
}