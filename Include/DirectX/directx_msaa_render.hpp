#pragma once

#include "entity.hpp"
#include "system_variables.hpp"
#include "window_manager.hpp"
#include "world.hpp"
#include <cassert>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <iostream>
#include <vector>
#include <wrl/client.h>

namespace albedo {
	class DirectXMSAARender {
	public:
		DirectXMSAARender(ID3D12Device* device) { construct(device); }
		~DirectXMSAARender() {}

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_rtv_msaa;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_dsv_msaa;
		Microsoft::WRL::ComPtr<ID3D12Resource>		 resource_msaa_color_texture;
		Microsoft::WRL::ComPtr<ID3D12Resource>		 resource_msaa_depth_texture;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_rtv_msaa;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_dsv_msaa;

		void populate_command_list_msaa(ID3D12Device* device, ID3D12GraphicsCommandList* command_list,
										ID3D12DescriptorHeap* descriptor_heap_cbv_srv, ID3D12Resource* resource_render,
										ID3D12Resource* resource_postprocess, D3D12_VIEWPORT viewport,
										D3D12_RECT rect_scissor) {
			HRESULT	  hr;
			const int MAX_CRV_SRV_BUFFER_SIZE = System::cbv_srv_buffer_size;

			D3D12_RESOURCE_BARRIER resource_barrier;
			ZeroMemory(&resource_barrier, sizeof(resource_barrier));
			resource_barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resource_barrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resource_barrier.Transition.pResource	= resource_msaa_color_texture.Get();
			resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
			resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;
			command_list->ResourceBarrier(1, &resource_barrier);

			const FLOAT clear_color[4] = {System::bg_color[0], System::bg_color[1], System::bg_color[2],
										  System::bg_color[3]};
			command_list->ClearRenderTargetView(handle_rtv_msaa, clear_color, 0, nullptr);
			command_list->ClearDepthStencilView(handle_dsv_msaa, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			command_list->RSSetViewports(1, &viewport);
			command_list->RSSetScissorRects(1, &rect_scissor);
			command_list->OMSetRenderTargets(1, &handle_rtv_msaa, TRUE, &handle_dsv_msaa);

			command_list->SetDescriptorHeaps(1, &descriptor_heap_cbv_srv);
			D3D12_CPU_DESCRIPTOR_HANDLE handle_cbv_srv = descriptor_heap_cbv_srv->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE handle_gpu_cbv_srv =
				descriptor_heap_cbv_srv->GetGPUDescriptorHandleForHeapStart();
			const UINT cbv_descriptor_size =
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (std::shared_ptr<albedo::Entity> entity : albedo::World::get_all_entities()) {
				command_list->SetGraphicsRootSignature(entity->directx_component->root_signature.Get());
				command_list->SetPipelineState(entity->directx_component->pipeline_state.Get());
				command_list->SetGraphicsRootDescriptorTable(0, handle_gpu_cbv_srv);
				entity->directx_component->draw(command_list, handle_cbv_srv);
				handle_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
				handle_gpu_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
			}

			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
			command_list->ResourceBarrier(1, &resource_barrier);

			if (!System::is_enabled_postprocess) {
				resource_barrier.Transition.pResource	= resource_render;
				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
				resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_RESOLVE_DEST;
				command_list->ResourceBarrier(1, &resource_barrier);

				command_list->ResolveSubresource(resource_render, 0, resource_msaa_color_texture.Get(), 0,
												 DXGI_FORMAT_R8G8B8A8_UNORM);

				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST;
				resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
				command_list->ResourceBarrier(1, &resource_barrier);
			} else {
				resource_barrier.Transition.pResource	= resource_postprocess;
				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
				resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_RESOLVE_DEST;
				command_list->ResourceBarrier(1, &resource_barrier);

				command_list->ResolveSubresource(resource_postprocess, 0, resource_msaa_color_texture.Get(), 0,
												 DXGI_FORMAT_R8G8B8A8_UNORM);

				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST;
				resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
				command_list->ResourceBarrier(1, &resource_barrier);
			}

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [Render] Closed");
		}

	private:
		void construct(ID3D12Device* device) {
			initialize_descriptor_heaps_msaa(device);
			initialize_msaa_resources_and_target_view(device);
		}
		void initialize_descriptor_heaps_msaa(ID3D12Device* device) {
			HRESULT hr;

			D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
			ZeroMemory(&rtv_heap_desc, sizeof(rtv_heap_desc));
			rtv_heap_desc.NumDescriptors = 1;
			rtv_heap_desc.Type			 = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtv_heap_desc.Flags			 = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtv_heap_desc.NodeMask		 = 0;
			hr = device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&descriptor_heap_rtv_msaa));
			assert(SUCCEEDED(hr) && "Create RTV Descriptor Heap[MSAA]");

			D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc;
			ZeroMemory(&dsv_heap_desc, sizeof(dsv_heap_desc));
			dsv_heap_desc.NumDescriptors = 1;
			dsv_heap_desc.Type			 = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsv_heap_desc.Flags			 = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsv_heap_desc.NodeMask		 = 0;
			hr = device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&descriptor_heap_dsv_msaa));
			assert(SUCCEEDED(hr) && "Create DSV Descriptor Heap[MSAA]");
		}
		void initialize_msaa_resources_and_target_view(ID3D12Device* device) {
			HRESULT hr;

			D3D12_HEAP_PROPERTIES heap_properties{};
			D3D12_RESOURCE_DESC	  resource_desc{};
			D3D12_CLEAR_VALUE	  clear_value{};
			heap_properties.Type				 = D3D12_HEAP_TYPE_DEFAULT;
			heap_properties.CPUPageProperty		 = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_properties.CreationNodeMask	 = 0;
			heap_properties.VisibleNodeMask		 = 0;

			resource_desc.Dimension			 = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resource_desc.Width				 = albedo::WindowManager::window_width;
			resource_desc.Height			 = albedo::WindowManager::window_height;
			resource_desc.DepthOrArraySize	 = 1;
			resource_desc.MipLevels			 = 1;
			resource_desc.Format			 = DXGI_FORMAT_R8G8B8A8_UNORM;
			resource_desc.Layout			 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resource_desc.SampleDesc.Count	 = 4;
			resource_desc.SampleDesc.Quality = 0;
			resource_desc.Flags				 = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			clear_value.Format	 = DXGI_FORMAT_R8G8B8A8_UNORM;
			clear_value.Color[0] = System::bg_color[0];
			clear_value.Color[1] = System::bg_color[1];
			clear_value.Color[2] = System::bg_color[2];
			clear_value.Color[3] = System::bg_color[3];

			// Create Render Texture Resource
			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_RESOLVE_SOURCE, &clear_value,
												 IID_PPV_ARGS(&resource_msaa_color_texture));
			assert(SUCCEEDED(hr) && "Create Committed Resorce[MSAA Color]");

			resource_desc.Format = DXGI_FORMAT_R32_TYPELESS;
			resource_desc.Flags	 = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_CLEAR_VALUE clear_value_depth{};
			clear_value_depth.Format			   = DXGI_FORMAT_D32_FLOAT;
			clear_value_depth.DepthStencil.Depth   = 1.0f;
			clear_value_depth.DepthStencil.Stencil = 0;

			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value_depth,
												 IID_PPV_ARGS(&resource_msaa_depth_texture));
			assert(SUCCEEDED(hr) && "Create Committed Resorce[MSAA Depth]");

			D3D12_RENDER_TARGET_VIEW_DESC rtv_desc_msaa{};
			rtv_desc_msaa.Format		= DXGI_FORMAT_R8G8B8A8_UNORM;
			rtv_desc_msaa.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc_msaa{};
			dsv_desc_msaa.Format		= DXGI_FORMAT_D32_FLOAT;
			dsv_desc_msaa.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

			handle_rtv_msaa = descriptor_heap_rtv_msaa->GetCPUDescriptorHandleForHeapStart();
			device->CreateRenderTargetView(resource_msaa_color_texture.Get(), &rtv_desc_msaa, handle_rtv_msaa);
			handle_dsv_msaa = descriptor_heap_dsv_msaa->GetCPUDescriptorHandleForHeapStart();
			device->CreateDepthStencilView(resource_msaa_depth_texture.Get(), &dsv_desc_msaa, handle_dsv_msaa);
		}
	};
} // namespace albedo