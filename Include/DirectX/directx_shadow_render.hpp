#pragma once

#include "entity.hpp"
#include "system_variables.hpp"
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
	class DirectXShadowRender {
	public:
		DirectXShadowRender(ID3D12Device* device, ID3D12RootSignature* root_signature) {
			construct(device, root_signature);
		}
		~DirectXShadowRender() {}

		D3D12_VIEWPORT								 viewport_shadow;
		D3D12_RECT									 rect_scissor_shadow;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_shadow;
		Microsoft::WRL::ComPtr<ID3D12Resource>		 resource_shadow;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_shadow;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>	 pipeline_state_shadow;
		const wchar_t* SHADER_NAME_SHADOW_MAPPING = L"./Source/Shader/ShadowMappingShaders.hlsl";

		void populate_command_list_shadow(ID3D12Device* device, ID3D12GraphicsCommandList* command_list,
										  ID3D12DescriptorHeap* descriptor_heap_cbv_srv,
										  ID3D12RootSignature*	root_signature) {
			HRESULT	   hr;
			const UINT max_cbv_srv_buffer_size = System::cbv_srv_buffer_size;

			D3D12_RESOURCE_BARRIER resource_barrier;
			ZeroMemory(&resource_barrier, sizeof(resource_barrier));
			resource_barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resource_barrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resource_barrier.Transition.pResource	= resource_shadow.Get();
			resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
			resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_DEPTH_WRITE;
			command_list->ResourceBarrier(1, &resource_barrier);

			command_list->ClearDepthStencilView(handle_shadow, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			command_list->RSSetViewports(1, &viewport_shadow);
			command_list->RSSetScissorRects(1, &rect_scissor_shadow);
			command_list->OMSetRenderTargets(0, nullptr, TRUE, &handle_shadow);

			command_list->SetDescriptorHeaps(1, &descriptor_heap_cbv_srv);
			D3D12_CPU_DESCRIPTOR_HANDLE handle_cpu = descriptor_heap_cbv_srv->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE handle_gpu = descriptor_heap_cbv_srv->GetGPUDescriptorHandleForHeapStart();
			const UINT					descriptor_size =
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			command_list->SetGraphicsRootSignature(root_signature);
			command_list->SetPipelineState(pipeline_state_shadow.Get());

			for (std::shared_ptr<albedo::Entity> entity : albedo::World::get_entities()) {
				command_list->SetGraphicsRootDescriptorTable(0, handle_gpu);
				entity->directx_component->draw(command_list, handle_cpu);
				handle_cpu.ptr += max_cbv_srv_buffer_size * descriptor_size;
				handle_gpu.ptr += max_cbv_srv_buffer_size * descriptor_size;
			}

			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_GENERIC_READ;
			command_list->ResourceBarrier(1, &resource_barrier);

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [Shadow Mapping]");
		}

	private:
		void construct(ID3D12Device* device, ID3D12RootSignature* root_signature) {
			initialize_viewport();
			initialize_descriptor_heaps_shadow(device);
			initialize_shadow_resource_and_view(device);
			initialize_pipeline_state_shadow(device, root_signature);
		}

		void initialize_viewport() {
			viewport_shadow.TopLeftX = 0.f;
			viewport_shadow.TopLeftY = 0.f;
			viewport_shadow.Width	 = 1024.0f;
			viewport_shadow.Height	 = 1024.0f;
			viewport_shadow.MinDepth = 0.f;
			viewport_shadow.MaxDepth = 1.f;

			rect_scissor_shadow.top	   = 0;
			rect_scissor_shadow.left   = 0;
			rect_scissor_shadow.right  = 1024;
			rect_scissor_shadow.bottom = 1024;
		}
		void initialize_descriptor_heaps_shadow(ID3D12Device* device) {
			HRESULT hr;

			D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc{};
			descriptor_heap_desc.NumDescriptors = 1;
			descriptor_heap_desc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			descriptor_heap_desc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			descriptor_heap_desc.NodeMask		= 0;
			hr = device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&descriptor_heap_shadow));
			assert(SUCCEEDED(hr) && "Create Descriptro Heap for Shadow");
		}
		void initialize_shadow_resource_and_view(ID3D12Device* device) {
			HRESULT				  hr;
			D3D12_HEAP_PROPERTIES heap_properties{};
			D3D12_RESOURCE_DESC	  resource_desc{};
			D3D12_CLEAR_VALUE	  clear_value{};

			heap_properties.Type				 = D3D12_HEAP_TYPE_DEFAULT;
			heap_properties.CPUPageProperty		 = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_properties.CreationNodeMask	 = 0;
			heap_properties.VisibleNodeMask		 = 0;

			resource_desc.Dimension			 = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resource_desc.Width				 = 1024;
			resource_desc.Height			 = 1024;
			resource_desc.DepthOrArraySize	 = 1;
			resource_desc.MipLevels			 = 0;
			resource_desc.Format			 = DXGI_FORMAT_R32_TYPELESS;
			resource_desc.Layout			 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resource_desc.SampleDesc.Count	 = 1;
			resource_desc.SampleDesc.Quality = 0;
			resource_desc.Flags				 = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			clear_value.Format				 = DXGI_FORMAT_D32_FLOAT;
			clear_value.DepthStencil.Depth	 = 1.0f;
			clear_value.DepthStencil.Stencil = 0;

			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_GENERIC_READ, &clear_value,
												 IID_PPV_ARGS(&resource_shadow));
			assert(SUCCEEDED(hr) && "Create Resource[Shadow Mapping]");

			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
			dsv_desc.ViewDimension		= D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Format				= DXGI_FORMAT_D32_FLOAT;
			dsv_desc.Texture2D.MipSlice = 0;
			dsv_desc.Flags				= D3D12_DSV_FLAG_NONE;
			handle_shadow				= descriptor_heap_shadow->GetCPUDescriptorHandleForHeapStart();
			device->CreateDepthStencilView(resource_shadow.Get(), &dsv_desc, handle_shadow);
		}
		void initialize_pipeline_state_shadow(ID3D12Device* device, ID3D12RootSignature* root_signature) {
			HRESULT hr;

			UINT							 compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader;
			Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader;

			hr = D3DCompileFromFile(SHADER_NAME_SHADOW_MAPPING, nullptr, nullptr, "VSShadowMap", "vs_5_0",
									compile_flags, 0, &vertex_shader, nullptr);

			D3D12_INPUT_ELEMENT_DESC desc_input_elements[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc{};

			pipeline_state_desc.VS.pShaderBytecode = vertex_shader->GetBufferPointer();
			pipeline_state_desc.VS.BytecodeLength  = vertex_shader->GetBufferSize();

			pipeline_state_desc.InputLayout.pInputElementDescs = desc_input_elements;
			pipeline_state_desc.InputLayout.NumElements		   = _countof(desc_input_elements);

			pipeline_state_desc.SampleDesc.Count   = 1;
			pipeline_state_desc.SampleDesc.Quality = 0;
			pipeline_state_desc.SampleMask		   = UINT_MAX;

			pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipeline_state_desc.pRootSignature		  = root_signature;

			pipeline_state_desc.RasterizerState.CullMode			  = D3D12_CULL_MODE_NONE; // D3D12_CULL_MODE_BACK;
			pipeline_state_desc.RasterizerState.FillMode			  = D3D12_FILL_MODE_SOLID;
			pipeline_state_desc.RasterizerState.FrontCounterClockwise = FALSE;
			pipeline_state_desc.RasterizerState.DepthBias			  = 0;
			pipeline_state_desc.RasterizerState.DepthBiasClamp		  = 0;
			pipeline_state_desc.RasterizerState.SlopeScaledDepthBias  = 0;
			pipeline_state_desc.RasterizerState.DepthClipEnable		  = TRUE;
			pipeline_state_desc.RasterizerState.ConservativeRaster	  = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			pipeline_state_desc.RasterizerState.AntialiasedLineEnable = FALSE;
			pipeline_state_desc.RasterizerState.MultisampleEnable	  = FALSE;

			pipeline_state_desc.BlendState.RenderTarget[0].BlendEnable			 = FALSE;
			pipeline_state_desc.BlendState.RenderTarget[0].SrcBlend				 = D3D12_BLEND_ONE;
			pipeline_state_desc.BlendState.RenderTarget[0].DestBlend			 = D3D12_BLEND_ZERO;
			pipeline_state_desc.BlendState.RenderTarget[0].BlendOp				 = D3D12_BLEND_OP_ADD;
			pipeline_state_desc.BlendState.RenderTarget[0].SrcBlendAlpha		 = D3D12_BLEND_ONE;
			pipeline_state_desc.BlendState.RenderTarget[0].DestBlendAlpha		 = D3D12_BLEND_ZERO;
			pipeline_state_desc.BlendState.RenderTarget[0].BlendOpAlpha			 = D3D12_BLEND_OP_ADD;
			pipeline_state_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			pipeline_state_desc.BlendState.RenderTarget[0].LogicOpEnable		 = FALSE;
			pipeline_state_desc.BlendState.RenderTarget[0].LogicOp				 = D3D12_LOGIC_OP_CLEAR;
			pipeline_state_desc.BlendState.AlphaToCoverageEnable				 = FALSE;
			pipeline_state_desc.BlendState.IndependentBlendEnable				 = FALSE;

			pipeline_state_desc.DepthStencilState.DepthEnable			  = TRUE; // 深度テストあり
			pipeline_state_desc.DepthStencilState.DepthFunc				  = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			pipeline_state_desc.DepthStencilState.DepthWriteMask		  = D3D12_DEPTH_WRITE_MASK_ALL;
			pipeline_state_desc.DepthStencilState.StencilEnable			  = FALSE; // ステンシルテストなし
			pipeline_state_desc.DepthStencilState.StencilReadMask		  = D3D12_DEFAULT_STENCIL_READ_MASK;
			pipeline_state_desc.DepthStencilState.StencilWriteMask		  = D3D12_DEFAULT_STENCIL_WRITE_MASK;
			pipeline_state_desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.FrontFace.StencilPassOp	   = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.FrontFace.StencilFunc		   = D3D12_COMPARISON_FUNC_ALWAYS;
			pipeline_state_desc.DepthStencilState.BackFace.StencilFailOp	   = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.BackFace.StencilPassOp	   = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.BackFace.StencilFunc		   = D3D12_COMPARISON_FUNC_ALWAYS;
			pipeline_state_desc.DSVFormat									   = DXGI_FORMAT_D32_FLOAT;

			hr = device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline_state_shadow));
			assert(SUCCEEDED(hr) && "Create Pipeline State[Shadow Mapping]");
		}
	};
} // namespace albedo