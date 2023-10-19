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
	class DirectXPostprocessRender {
	public:
		DirectXPostprocessRender(ID3D12Device* device) { construct(device); }
		~DirectXPostprocessRender() {}

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_rtv_render_texture;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_cbv_srv_postprocess;
		Microsoft::WRL::ComPtr<ID3D12Resource>		 resource_render_texture;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_rtv_render_texture;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_cbv_srv_postprocess;
		Microsoft::WRL::ComPtr<ID3D12RootSignature>	 root_signature_postprocess;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>	 pipeline_state_postprocess;
		const wchar_t* SHADER_NAME_POSTPROCESS = L"./Source/Shader/PostprocessShaders.hlsl";

		void populate_command_list_postprocess(ID3D12Device* device, ID3D12GraphicsCommandList* command_list,
											   ID3D12Resource* resource_render, D3D12_CPU_DESCRIPTOR_HANDLE handle_rtv,
											   D3D12_VIEWPORT viewport, D3D12_RECT rect_scissor) {
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

			const FLOAT clear_color[4] = {System::bg_color[0], System::bg_color[1], System::bg_color[2],
										  System::bg_color[3]};
			command_list->ClearRenderTargetView(handle_rtv, clear_color, 0, nullptr);

			command_list->RSSetViewports(1, &viewport);
			command_list->RSSetScissorRects(1, &rect_scissor);
			command_list->OMSetRenderTargets(1, &handle_rtv, TRUE, nullptr);

			command_list->SetGraphicsRootSignature(root_signature_postprocess.Get());
			command_list->SetPipelineState(pipeline_state_postprocess.Get());

			command_list->SetDescriptorHeaps(1, descriptor_heap_cbv_srv_postprocess.GetAddressOf());
			D3D12_GPU_DESCRIPTOR_HANDLE cbv_srv_gpu_handle =
				descriptor_heap_cbv_srv_postprocess->GetGPUDescriptorHandleForHeapStart();
			command_list->SetGraphicsRootDescriptorTable(0, cbv_srv_gpu_handle);

			// Shader Resource View for Render Texture
			D3D12_SHADER_RESOURCE_VIEW_DESC render_texture_desc{};
			render_texture_desc.Format						  = DXGI_FORMAT_R8G8B8A8_UNORM;
			render_texture_desc.ViewDimension				  = D3D12_SRV_DIMENSION_TEXTURE2D;
			render_texture_desc.Texture2D.MipLevels			  = 1;
			render_texture_desc.Texture2D.MostDetailedMip	  = 0;
			render_texture_desc.Texture2D.PlaneSlice		  = 0;
			render_texture_desc.Texture2D.ResourceMinLODClamp = 0.0F;
			render_texture_desc.Shader4ComponentMapping		  = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			D3D12_CPU_DESCRIPTOR_HANDLE handle_srv_cbv =
				descriptor_heap_cbv_srv_postprocess->GetCPUDescriptorHandleForHeapStart();
			device->CreateShaderResourceView(resource_render_texture.Get(), &render_texture_desc, handle_srv_cbv);

			command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			command_list->DrawInstanced(6, 1, 0, 0);

			resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			resource_barrier.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
			command_list->ResourceBarrier(1, &resource_barrier);

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [Postprocess] Closed");
		}

	private:
		void construct(ID3D12Device* device) {
			initialize_descriptor_heap_postprocess(device);
			initialize_postprocess_resource_and_view(device);
			initialize_root_signature_postprocess(device);
			initialize_pipeline_state_postprocess(device);
		}
		void initialize_descriptor_heap_postprocess(ID3D12Device* device) {
			HRESULT hr;

			// Descriptor Heap for Drawing Render Texture
			D3D12_DESCRIPTOR_HEAP_DESC rtv_render_target_heap_desc;
			rtv_render_target_heap_desc.NumDescriptors = 1;
			rtv_render_target_heap_desc.Type		   = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtv_render_target_heap_desc.Flags		   = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtv_render_target_heap_desc.NodeMask	   = 0;
			hr										   = device->CreateDescriptorHeap(&rtv_render_target_heap_desc,
																					  IID_PPV_ARGS(&descriptor_heap_rtv_render_texture));
			assert(SUCCEEDED(hr) && "Create RTV Descriptor Heap[Postprocess]");

			// Descriptor Heap for SRV CBV in Postprocessing context
			D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_postprocess_heap_desc;
			cbv_srv_postprocess_heap_desc.NumDescriptors = 1;
			cbv_srv_postprocess_heap_desc.Type			 = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbv_srv_postprocess_heap_desc.Flags			 = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbv_srv_postprocess_heap_desc.NodeMask		 = 0;
			hr											 = device->CreateDescriptorHeap(&cbv_srv_postprocess_heap_desc,
																						IID_PPV_ARGS(&descriptor_heap_cbv_srv_postprocess));
			assert(SUCCEEDED(hr) && "Create CBV SRV UAV Descriptor Heap[Postprocess]");
		}
		void initialize_postprocess_resource_and_view(ID3D12Device* device) {
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
			resource_desc.Width				 = albedo::WindowManager::window_width;
			resource_desc.Height			 = albedo::WindowManager::window_height;
			resource_desc.DepthOrArraySize	 = 1;
			resource_desc.MipLevels			 = 1;
			resource_desc.Format			 = DXGI_FORMAT_R8G8B8A8_UNORM;
			resource_desc.Layout			 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resource_desc.SampleDesc.Count	 = 1;
			resource_desc.SampleDesc.Quality = 0;
			resource_desc.Flags				 = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			clear_value.Format	 = DXGI_FORMAT_R8G8B8A8_UNORM;
			clear_value.Color[0] = System::bg_color[0];
			clear_value.Color[1] = System::bg_color[1];
			clear_value.Color[2] = System::bg_color[2];
			clear_value.Color[3] = System::bg_color[3];

			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_PRESENT, &clear_value,
												 IID_PPV_ARGS(&resource_render_texture));
			assert(SUCCEEDED(hr) && "Create Committed Resorce[Postprocess]");

			handle_rtv_render_texture = descriptor_heap_rtv_render_texture->GetCPUDescriptorHandleForHeapStart();
			device->CreateRenderTargetView(resource_render_texture.Get(), nullptr, handle_rtv_render_texture);

			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
			srv_desc.Format					 = DXGI_FORMAT_R8G8B8A8_UNORM;
			srv_desc.Texture2D.MipLevels	 = 1;
			srv_desc.ViewDimension			 = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			handle_cbv_srv_postprocess = descriptor_heap_cbv_srv_postprocess->GetCPUDescriptorHandleForHeapStart();
			device->CreateShaderResourceView(resource_render_texture.Get(), &srv_desc, handle_cbv_srv_postprocess);
		}
		void initialize_root_signature_postprocess(ID3D12Device* device) {
			HRESULT							 hr;
			D3D12_ROOT_PARAMETER			 root_parameters[1];
			D3D12_STATIC_SAMPLER_DESC		 sampler_desc[1]	 = {};
			D3D12_ROOT_SIGNATURE_DESC		 root_signature_desc = {};
			Microsoft::WRL::ComPtr<ID3DBlob> root_signature_blob;
			Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

			ZeroMemory(&root_parameters[0], sizeof(root_parameters[0]));
			ZeroMemory(&sampler_desc, sizeof(sampler_desc));
			ZeroMemory(&root_signature_desc, sizeof(root_signature_desc));

			D3D12_DESCRIPTOR_RANGE ranges[1];
			ranges[0].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			ranges[0].NumDescriptors					= 1;
			ranges[0].BaseShaderRegister				= 0;
			ranges[0].RegisterSpace						= 0;
			ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			root_parameters[0].ParameterType					   = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			root_parameters[0].DescriptorTable.NumDescriptorRanges = 1;
			root_parameters[0].DescriptorTable.pDescriptorRanges   = &ranges[0];
			root_parameters[0].ShaderVisibility					   = D3D12_SHADER_VISIBILITY_ALL;

			sampler_desc[0].Filter			 = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			sampler_desc[0].AddressU		 = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler_desc[0].AddressV		 = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler_desc[0].AddressW		 = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler_desc[0].MipLODBias		 = 0.0f;
			sampler_desc[0].MaxAnisotropy	 = 0;
			sampler_desc[0].ComparisonFunc	 = D3D12_COMPARISON_FUNC_NEVER;
			sampler_desc[0].BorderColor		 = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			sampler_desc[0].MinLOD			 = 0;
			sampler_desc[0].MaxLOD			 = D3D12_FLOAT32_MAX;
			sampler_desc[0].ShaderRegister	 = 0;
			sampler_desc[0].RegisterSpace	 = 0;
			sampler_desc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			root_signature_desc.Flags			  = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			root_signature_desc.NumParameters	  = _countof(root_parameters);
			root_signature_desc.pParameters		  = root_parameters;
			root_signature_desc.NumStaticSamplers = 1;
			root_signature_desc.pStaticSamplers	  = sampler_desc;

			hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &root_signature_blob,
											 &error_blob);
			assert(SUCCEEDED(hr) && "Serialize Root Signature[Postprocess]");

			hr = device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(),
											 root_signature_blob->GetBufferSize(),
											 IID_PPV_ARGS(&root_signature_postprocess));
			assert(SUCCEEDED(hr) && "Create Root Signature[Postprocess]");
		}
		void initialize_pipeline_state_postprocess(ID3D12Device* device) {
			HRESULT hr;

			UINT							 compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_postprocess;
			Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_postprocess;

			// Stop When Shader Compile Error
			hr = D3DCompileFromFile(SHADER_NAME_POSTPROCESS, nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0,
									&vertex_shader_postprocess, nullptr);
			hr = D3DCompileFromFile(SHADER_NAME_POSTPROCESS, nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0,
									&pixel_shader_postprocess, nullptr);

			D3D12_INPUT_ELEMENT_DESC desc_input_elements[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc{};

			pipeline_state_desc.VS.pShaderBytecode			   = vertex_shader_postprocess->GetBufferPointer();
			pipeline_state_desc.VS.BytecodeLength			   = vertex_shader_postprocess->GetBufferSize();
			pipeline_state_desc.PS.pShaderBytecode			   = pixel_shader_postprocess->GetBufferPointer();
			pipeline_state_desc.PS.BytecodeLength			   = pixel_shader_postprocess->GetBufferSize();
			pipeline_state_desc.InputLayout.pInputElementDescs = desc_input_elements;
			pipeline_state_desc.InputLayout.NumElements		   = _countof(desc_input_elements);

			pipeline_state_desc.NumRenderTargets = 1;
			pipeline_state_desc.RTVFormats[0]	 = DXGI_FORMAT_R8G8B8A8_UNORM;

			pipeline_state_desc.SampleDesc.Count   = 1;
			pipeline_state_desc.SampleDesc.Quality = 0;
			pipeline_state_desc.SampleMask		   = UINT_MAX;

			pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipeline_state_desc.pRootSignature		  = root_signature_postprocess.Get();

			pipeline_state_desc.RasterizerState.CullMode			  = D3D12_CULL_MODE_NONE;
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

			hr = device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline_state_postprocess));
			assert(SUCCEEDED(hr) && "Create Graphics Pipeline State[Postprocess]");
		}
	};
} // namespace albedo