#pragma once

#include "entity.hpp"
#include "system_variables.hpp"
#include "window_manager.hpp"
#include "world.hpp"
#include <External/imgui/imgui_impl_dx12.h>
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
	class DirectXImGuiRender {
	public:
		DirectXImGuiRender(ID3D12Device* device) { construct(device); }

		static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_imgui;

		void populate_command_list_imgui(ID3D12GraphicsCommandList* command_list, ID3D12Resource* resource_render,
										 D3D12_CPU_DESCRIPTOR_HANDLE handle_rtv) {
			HRESULT hr;

			D3D12_RESOURCE_BARRIER resource_barrier;
			ZeroMemory(&resource_barrier, sizeof(resource_barrier));
			resource_barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resource_barrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resource_barrier.Transition.pResource	= resource_render;
			resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;
			command_list->ResourceBarrier(1, &resource_barrier);

			command_list->OMSetRenderTargets(1, &handle_rtv, TRUE, nullptr);
			// command_list->OMSetRenderTargets(1, &handle_rtv[rtv_index], TRUE, &handle_dsv);
			// command_list->SetGraphicsRootSignature(root_signature.Get());
			// command_list->SetPipelineState(pipeline_state_render.Get());

			command_list->SetDescriptorHeaps(1, descriptor_heap_imgui.GetAddressOf());
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list);

			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
			command_list->ResourceBarrier(1, &resource_barrier);

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [ImGui] Closed");
		}

	private:
		void construct(ID3D12Device* device) { initialize_descriptor_heap(device); }
		void initialize_descriptor_heap(ID3D12Device* device) {
			HRESULT					   hr;
			D3D12_DESCRIPTOR_HEAP_DESC imgui_heap_desc;
			imgui_heap_desc.NumDescriptors = 1;
			imgui_heap_desc.Type		   = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			imgui_heap_desc.Flags		   = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			imgui_heap_desc.NodeMask	   = 0;
			hr = device->CreateDescriptorHeap(&imgui_heap_desc, IID_PPV_ARGS(&descriptor_heap_imgui));
			assert(SUCCEEDED(hr) && "Create ImGui Descriptor Heap");
		}
	};
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXImGuiRender::descriptor_heap_imgui = nullptr;
} // namespace albedo