#pragma once
#define DIRECTX_LOG(log_msg) std::cout << "[DirectX12]" << log_msg << std::endl;
#define DIRECTX_ASSERT(hr, log_msg) assert(SUCCEEDED(hr) && log_msg);

#include "constant.hpp"
#include "global.hpp"
#include "object.hpp"
#include "shape.hpp"
#include <cassert>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_glfw.h>
#include <iostream>
#include <vector>
#include <wrl/client.h>

namespace albedo {
	class DirectXA {
	public:
		DirectXA(HWND h) : hwnd(h) { init_directx(); }
		~DirectXA() {}

		UINT			  WIDTH						 = 800;
		UINT			  HEIGHT					 = 600;
		static const UINT NUM_FRAMES_IN_FLIGHT		 = 2;
		static const bool USE_WARP_DEVICE			 = false;
		static const UINT MAX_OBJECT_SIZE			 = 5;
		static const UINT MAX_CRV_SRV_BUFFER_SIZE	 = 5;
		const wchar_t*	  SHADER_NAME_COLOR			 = L"./Source/Shader/ColorShaders.hlsl";
		const wchar_t*	  SHADER_NAME_SHADOW_MAPPING = L"./Source/Shader/ShadowMappingShaders.hlsl";
		const wchar_t*	  SHADER_NAME_POSTPROCESS	 = L"./Source/Shader/PostprocessShaders.hlsl";
		const wchar_t*	  SHADER_NAME_SKYDOME		 = L"./Source/Shader/SkydomeShaders.hlsl";

	private:
		HWND		   hwnd;
		UINT64		   frame_number = 1;
		UINT		   rtv_index	= 0;
		D3D12_VIEWPORT viewport;
		D3D12_VIEWPORT viewport_shadow;
		D3D12_RECT	   rect_scissor;
		D3D12_RECT	   rect_scissor_shadow;

		std::vector<albedo::Object*>	 render_objects;
		std::shared_ptr<albedo::Object> skydome_object;

		Microsoft::WRL::ComPtr<IDXGIFactory4>			  factory;
		Microsoft::WRL::ComPtr<ID3D12Device>			  device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>		  command_queue;
		Microsoft::WRL::ComPtr<ID3D12Fence>				  queue_fence;
		HANDLE											  handle_fence_event;
		IDXGISwapChain3*								  swap_chain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	  descriptor_heap_rtv;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	  descriptor_heap_cbv_srv;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	  descriptor_heap_dsv;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	  descriptor_heap_imgui;
		Microsoft::WRL::ComPtr<ID3D12Resource>			  resource_render[NUM_FRAMES_IN_FLIGHT];
		Microsoft::WRL::ComPtr<ID3D12Resource>			  resource_depth;
		D3D12_CPU_DESCRIPTOR_HANDLE						  handle_rtv[NUM_FRAMES_IN_FLIGHT];
		D3D12_CPU_DESCRIPTOR_HANDLE						  handle_dsv;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	  command_allocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;
		Microsoft::WRL::ComPtr<ID3D12RootSignature>		  root_signature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>		  pipeline_state_render;

		// For Baking Depth to Texture
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_shadow;
		Microsoft::WRL::ComPtr<ID3D12Resource>		 resource_shadow;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_shadow;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>	 pipeline_state_shadow;

		// For Baking Render to Texture and Postprocessing
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_rtv_render_texture;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_cbv_srv_postprocess;
		Microsoft::WRL::ComPtr<ID3D12Resource>		 resource_render_texture;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_rtv_render_texture;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_cbv_srv_postprocess;
		Microsoft::WRL::ComPtr<ID3D12RootSignature>	 root_signature_postprocess;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>	 pipeline_state_postprocess;

		// For MSAA
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_rtv_msaa;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_dsv_msaa;
		Microsoft::WRL::ComPtr<ID3D12Resource>		 resource_msaa_color_texture;
		Microsoft::WRL::ComPtr<ID3D12Resource>		 resource_msaa_depth_texture;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_rtv_msaa;
		D3D12_CPU_DESCRIPTOR_HANDLE					 handle_dsv_msaa;

		// Diferred Rendering
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_rtv_diferred;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_cbv_srv_uav_diferred;

	private:
		// Initialize DirectX12 Functions
		void init_directx() {
			init_viewport();
			DIRECTX_LOG("Initialized Viewport");
			init_device();
			DIRECTX_LOG("Initialized Device");
			init_command_queue();
			DIRECTX_LOG("Initialized CommandQueue");
			init_swap_chain();
			DIRECTX_LOG("Initialized Swapchain");
			init_descriptor_heaps();
			DIRECTX_LOG("Initialized Descriptor Heaps");
			init_render_target_view();
			DIRECTX_LOG("Initialized Render Targets");
			init_depth_stencil_view();
			DIRECTX_LOG("Initialized Depth Buffer");
			init_command_list();
			DIRECTX_LOG("Initialized Command List");
			init_root_signature();
			DIRECTX_LOG("Initialized Root Signature");
			init_pipeline_state_render();
			DIRECTX_LOG("Initialized Pipeline States");

			init_descriptor_heaps_shadow();
			DIRECTX_LOG("Initialized Descriptor Heaps[Shadow Mapping]");
			init_shadow_resource_and_target_view();
			DIRECTX_LOG("Initialized Resources and Target Views[Shadow Mapping]");
			init_pipeline_state_shadow();
			DIRECTX_LOG("Initialized Pipeline States[Shadow Mapping]");

			init_descriptor_heap_postprocess();
			DIRECTX_LOG("Initialized Descriptor Heaps[Postprocess]");
			init_postprocess_resource_and_target_view();
			DIRECTX_LOG("Initialized Render Resorces and Target Views[Postprocess]");
			init_root_signature_postprocess();
			DIRECTX_LOG("Initialized Root Signature[Postprocess]");
			init_pipeline_state_postprocess();
			DIRECTX_LOG("Initialized Pipeline States[Postporcess]");

			init_descriptor_heaps_msaa();
			DIRECTX_LOG("Initialized Descriptor Heaps[MSAA]");
			init_msaa_resources_and_target_view();
			DIRECTX_LOG("Initialized Resources and Target Views[MSAA]");

			init_descriptor_heaps_deferred();
			DIRECTX_LOG("Initialized Descriptor Heaps[Diferred]");
		}
		void init_viewport() {
			viewport.TopLeftX = 0.f;
			viewport.TopLeftY = 0.f;
			viewport.Width	  = (FLOAT)WIDTH;
			viewport.Height	  = (FLOAT)HEIGHT;
			viewport.MinDepth = 0.f;
			viewport.MaxDepth = 1.f;

			rect_scissor.top	= 0;
			rect_scissor.left	= 0;
			rect_scissor.right	= WIDTH;
			rect_scissor.bottom = HEIGHT;

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
		void init_device() {
			HRESULT hr;
			UINT	flags_DXGI_factory = 0;

#if _DEBUG
			//  Enable Debug Layer
			Microsoft::WRL::ComPtr<ID3D12Debug> debug = nullptr;
			hr										  = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
			DIRECTX_ASSERT(hr, "Get D3D12 Debug Layer");
			debug->EnableDebugLayer();
			flags_DXGI_factory |= DXGI_CREATE_FACTORY_DEBUG;

			// Enable GBV
			// Microsoft::WRL::ComPtr<ID3D12Debug3> debug_3;
			// debug.As(&debug_3);
			// debug_3->SetEnableGPUBasedValidation(true);
#endif

			// Create DXGI Factory
			hr = CreateDXGIFactory2(flags_DXGI_factory, IID_PPV_ARGS(&factory));
			DIRECTX_ASSERT(hr, "Failed to create DXGI Factory");

			// Create D3D12 device
			if (USE_WARP_DEVICE) {
				Microsoft::WRL::ComPtr<IDXGIAdapter> warp_adapter;
				this->factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter));
				hr = D3D12CreateDevice(warp_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
			} else {
				Microsoft::WRL::ComPtr<IDXGIAdapter1> hardware_adapter;
				get_hardware_adaptor(factory.Get(), &hardware_adapter);
				hr = D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
			}
			DIRECTX_ASSERT(hr, "Create Device");

#if _DEBUG
			// D3D12InfoQueue1 is supported on Windows 11
			Microsoft::WRL::ComPtr<ID3D12InfoQueue1> info_queue;
			if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&info_queue)))) {
				D3D12MessageFunc message_func = [](D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity,
												   D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext) {
					(void)pContext;

					if (Severity == 3) {
						return;
					}
					std::cout << "[" << ID << "]"
							  << "[" << Category << "]"
							  << "[" << Severity << "]" << pDescription << std::endl;
				};
				D3D12_MESSAGE_CALLBACK_FLAGS message_flag =
					D3D12_MESSAGE_CALLBACK_FLAGS::D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS;
				DWORD message_cookie;
				hr = info_queue->RegisterMessageCallback(message_func, message_flag, nullptr, &message_cookie);
				DIRECTX_ASSERT(hr, "Register Message Callback");
			}
#endif
		}
		void init_command_queue() {
			HRESULT					 hr;
			D3D12_COMMAND_QUEUE_DESC queue_desc = {};
			queue_desc.Flags					= D3D12_COMMAND_QUEUE_FLAG_NONE;
			queue_desc.Type						= D3D12_COMMAND_LIST_TYPE_DIRECT;
			queue_desc.Priority					= 0;
			queue_desc.NodeMask					= 0;
			hr = this->device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue));
			assert(SUCCEEDED(hr) && "Create Command Queue");

			handle_fence_event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
			assert(handle_fence_event && "Create Fence Event Handle");
			hr = this->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queue_fence));
			assert(SUCCEEDED(hr) && "Create Fence");
		}
		void init_swap_chain() {
			HRESULT hr;

			DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
			ZeroMemory(&swapchain_desc, sizeof(swapchain_desc));
			swapchain_desc.BufferCount		 = NUM_FRAMES_IN_FLIGHT;
			swapchain_desc.BufferDesc.Width	 = WIDTH;
			swapchain_desc.BufferDesc.Height = HEIGHT;
			swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapchain_desc.SwapEffect		 = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapchain_desc.OutputWindow		 = hwnd;
			swapchain_desc.SampleDesc.Count	 = 1;
			swapchain_desc.Windowed			 = TRUE;

			IDXGISwapChain* temp_swap_chain;
			hr = factory->CreateSwapChain(command_queue.Get(), &swapchain_desc, &temp_swap_chain);
			assert(SUCCEEDED(hr) && "Create Swap Chain");
			temp_swap_chain->QueryInterface(&swap_chain);
			temp_swap_chain->Release();
			temp_swap_chain = nullptr;
		}
		void init_descriptor_heaps() {
			HRESULT hr;

			// Render Target
			D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
			ZeroMemory(&rtv_heap_desc, sizeof(rtv_heap_desc));
			rtv_heap_desc.NumDescriptors = 2;
			rtv_heap_desc.Type			 = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtv_heap_desc.Flags			 = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtv_heap_desc.NodeMask		 = 0;
			hr = device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&descriptor_heap_rtv));
			assert(SUCCEEDED(hr) && "Create RTV Descriptor Heap");

			// Constant Buffers and Shader Resources
			D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_heap_desc;
			cbv_srv_heap_desc.NumDescriptors = MAX_CRV_SRV_BUFFER_SIZE * MAX_OBJECT_SIZE;
			cbv_srv_heap_desc.Type			 = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbv_srv_heap_desc.Flags			 = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbv_srv_heap_desc.NodeMask		 = 0;
			hr = device->CreateDescriptorHeap(&cbv_srv_heap_desc, IID_PPV_ARGS(&descriptor_heap_cbv_srv));
			assert(SUCCEEDED(hr) && "Create CBV SRV UAV Descriptor Heap");

			// Depth Stencil
			D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc;
			ZeroMemory(&dsv_heap_desc, sizeof(dsv_heap_desc));
			dsv_heap_desc.NumDescriptors = 1;
			dsv_heap_desc.Type			 = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsv_heap_desc.Flags			 = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsv_heap_desc.NodeMask		 = 0;
			hr = device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&descriptor_heap_dsv));
			assert(SUCCEEDED(hr) && "Create DSV Descriptor Heap");

			// ImGui Rendering
			D3D12_DESCRIPTOR_HEAP_DESC imgui_heap_desc;
			imgui_heap_desc.NumDescriptors = 1;
			imgui_heap_desc.Type		   = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			imgui_heap_desc.Flags		   = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			imgui_heap_desc.NodeMask	   = 0;
			hr = device->CreateDescriptorHeap(&imgui_heap_desc, IID_PPV_ARGS(&descriptor_heap_imgui));
			assert(SUCCEEDED(hr) && "Create ImGui Descriptor Heap");
		}
		void init_render_target_view() {
			HRESULT hr;
			UINT	rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
				hr = swap_chain->GetBuffer(i, IID_PPV_ARGS(&resource_render[i]));
				assert(SUCCEEDED(hr) && "Get RTV Buffer");

				handle_rtv[i] = descriptor_heap_rtv->GetCPUDescriptorHandleForHeapStart();
				handle_rtv[i].ptr += rtv_descriptor_size * i;
				device->CreateRenderTargetView(resource_render[i].Get(), nullptr, handle_rtv[i]);
			}
		}
		void init_depth_stencil_view() {
			HRESULT hr;

			D3D12_RESOURCE_DESC	  depth_desc;
			D3D12_HEAP_PROPERTIES heap_props;
			D3D12_CLEAR_VALUE	  clear_value;
			ZeroMemory(&depth_desc, sizeof(depth_desc));
			ZeroMemory(&heap_props, sizeof(heap_props));
			ZeroMemory(&clear_value, sizeof(clear_value));

			depth_desc.Dimension			 = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depth_desc.Width				 = 1024;
			depth_desc.Height				 = 1024;
			depth_desc.DepthOrArraySize		 = 1;
			depth_desc.MipLevels			 = 0;
			depth_desc.Format				 = DXGI_FORMAT_R32_TYPELESS;
			depth_desc.Layout				 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depth_desc.SampleDesc.Count		 = 1;
			depth_desc.SampleDesc.Quality	 = 0;
			depth_desc.Flags				 = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			heap_props.Type					 = D3D12_HEAP_TYPE_DEFAULT;
			heap_props.CPUPageProperty		 = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_props.MemoryPoolPreference	 = D3D12_MEMORY_POOL_UNKNOWN;
			heap_props.CreationNodeMask		 = 0;
			heap_props.VisibleNodeMask		 = 0;
			clear_value.Format				 = DXGI_FORMAT_D32_FLOAT;
			clear_value.DepthStencil.Depth	 = 1.0f;
			clear_value.DepthStencil.Stencil = 0;
			hr = device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &depth_desc,
												 D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value,
												 IID_PPV_ARGS(&resource_depth));
			assert(SUCCEEDED(hr) && "Create Committed Resource");

			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
			ZeroMemory(&dsv_desc, sizeof(dsv_desc));
			dsv_desc.ViewDimension		= D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Format				= DXGI_FORMAT_D32_FLOAT;
			dsv_desc.ViewDimension		= D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Texture2D.MipSlice = 0;
			dsv_desc.Flags				= D3D12_DSV_FLAG_NONE;
			handle_dsv					= descriptor_heap_dsv->GetCPUDescriptorHandleForHeapStart();
			device->CreateDepthStencilView(resource_depth.Get(), &dsv_desc, handle_dsv);
		}
		void init_command_list() {
			HRESULT h_result;

			h_result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator));
			assert(SUCCEEDED(h_result) && "Create Command Allocator");

			h_result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), nullptr,
												 IID_PPV_ARGS(&command_list));
			assert(SUCCEEDED(h_result) && "Create Command List");
		}
		void init_root_signature() {
			HRESULT							 h_result;
			D3D12_ROOT_PARAMETER			 root_parameters[1];
			D3D12_STATIC_SAMPLER_DESC		 sampler_desc[2]	 = {};
			D3D12_ROOT_SIGNATURE_DESC		 root_signature_desc = {};
			Microsoft::WRL::ComPtr<ID3DBlob> root_signature_blob;
			Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

			ZeroMemory(&root_parameters[0], sizeof(root_parameters[0]));
			ZeroMemory(&sampler_desc, sizeof(sampler_desc));
			ZeroMemory(&root_signature_desc, sizeof(root_signature_desc));

			D3D12_DESCRIPTOR_RANGE ranges[2];
			ranges[0].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			ranges[0].NumDescriptors					= 2;
			ranges[0].BaseShaderRegister				= 0;
			ranges[0].RegisterSpace						= 0;
			ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			ranges[1].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			ranges[1].NumDescriptors					= 3;
			ranges[1].BaseShaderRegister				= 0;
			ranges[1].RegisterSpace						= 0;
			ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			root_parameters[0].ParameterType					   = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			root_parameters[0].DescriptorTable.NumDescriptorRanges = 2;
			root_parameters[0].DescriptorTable.pDescriptorRanges   = &ranges[0];
			root_parameters[0].ShaderVisibility					   = D3D12_SHADER_VISIBILITY_ALL;

			// Static Sampler for Texture
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

			// Static Sampler for Shadow Map
			sampler_desc[1].Filter			 = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			sampler_desc[1].AddressU		 = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler_desc[1].AddressV		 = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler_desc[1].AddressW		 = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler_desc[1].MipLODBias		 = 0.0f;
			sampler_desc[1].MaxAnisotropy	 = 16;
			sampler_desc[1].ComparisonFunc	 = D3D12_COMPARISON_FUNC_NEVER;
			sampler_desc[1].BorderColor		 = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			sampler_desc[1].MinLOD			 = 0.0f;
			sampler_desc[1].MaxLOD			 = D3D12_FLOAT32_MAX;
			sampler_desc[1].ShaderRegister	 = 1;
			sampler_desc[1].RegisterSpace	 = 0;
			sampler_desc[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			root_signature_desc.Flags			  = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			root_signature_desc.NumParameters	  = _countof(root_parameters);
			root_signature_desc.pParameters		  = root_parameters;
			root_signature_desc.NumStaticSamplers = 2;
			root_signature_desc.pStaticSamplers	  = sampler_desc;

			h_result = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1,
												   &root_signature_blob, &error_blob);
			assert(SUCCEEDED(h_result) && "Serialize Root Signature");

			h_result = device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(),
												   root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&root_signature));
			assert(SUCCEEDED(h_result) && "Create Root Signature");
		}
		void init_pipeline_state_render() {
			HRESULT hr;

			UINT							 compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader;
			Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader;

			// Stop When Shader Compile Error
			hr = D3DCompileFromFile(SHADER_NAME_COLOR, nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0,
									&vertex_shader, nullptr);
			DIRECTX_ASSERT(hr, "Compile Vertex Shader");
			hr = D3DCompileFromFile(SHADER_NAME_COLOR, nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0,
									&pixel_shader, nullptr);
			DIRECTX_ASSERT(hr, "Compile Pixel Shader");

			// Vertex Layout
			D3D12_INPUT_ELEMENT_DESC desc_input_elements[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipline_state_desc;
			ZeroMemory(&pipline_state_desc, sizeof(pipline_state_desc));

			// Shader Settings
			pipline_state_desc.VS.pShaderBytecode = vertex_shader->GetBufferPointer();
			pipline_state_desc.VS.BytecodeLength  = vertex_shader->GetBufferSize();
			pipline_state_desc.HS.pShaderBytecode = nullptr;
			pipline_state_desc.HS.BytecodeLength  = 0;
			pipline_state_desc.DS.pShaderBytecode = nullptr;
			pipline_state_desc.DS.BytecodeLength  = 0;
			pipline_state_desc.GS.pShaderBytecode = nullptr;
			pipline_state_desc.GS.BytecodeLength  = 0;
			pipline_state_desc.PS.pShaderBytecode = pixel_shader->GetBufferPointer();
			pipline_state_desc.PS.BytecodeLength  = pixel_shader->GetBufferSize();

			// Input Layout Settings
			pipline_state_desc.InputLayout.pInputElementDescs = desc_input_elements;
			pipline_state_desc.InputLayout.NumElements		  = _countof(desc_input_elements);

			// Sample Settings
			pipline_state_desc.SampleDesc.Count	  = 1;
			pipline_state_desc.SampleDesc.Quality = 0;
			pipline_state_desc.SampleMask		  = UINT_MAX;

			// Render Target Settings
			pipline_state_desc.NumRenderTargets = 1;
			pipline_state_desc.RTVFormats[0]	= DXGI_FORMAT_R8G8B8A8_UNORM;

			// Primitive Topology Type Settings
			pipline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			// Root Signature
			pipline_state_desc.pRootSignature = root_signature.Get();

			// Rasterizer State Settings
			pipline_state_desc.RasterizerState.CullMode				 = D3D12_CULL_MODE_NONE;
			pipline_state_desc.RasterizerState.FillMode				 = D3D12_FILL_MODE_SOLID;
			pipline_state_desc.RasterizerState.FrontCounterClockwise = FALSE;
			pipline_state_desc.RasterizerState.DepthBias			 = 0;
			pipline_state_desc.RasterizerState.DepthBiasClamp		 = 0;
			pipline_state_desc.RasterizerState.SlopeScaledDepthBias	 = 0;
			pipline_state_desc.RasterizerState.DepthClipEnable		 = TRUE;
			pipline_state_desc.RasterizerState.ConservativeRaster	 = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			pipline_state_desc.RasterizerState.AntialiasedLineEnable = FALSE;
			pipline_state_desc.RasterizerState.MultisampleEnable	 = FALSE;

			// Blend State Settings
			for (int i = 0; i < static_cast<int>(_countof(pipline_state_desc.BlendState.RenderTarget)); ++i) {
				pipline_state_desc.BlendState.RenderTarget[i].BlendEnable			= FALSE;
				pipline_state_desc.BlendState.RenderTarget[i].SrcBlend				= D3D12_BLEND_ONE;
				pipline_state_desc.BlendState.RenderTarget[i].DestBlend				= D3D12_BLEND_ZERO;
				pipline_state_desc.BlendState.RenderTarget[i].BlendOp				= D3D12_BLEND_OP_ADD;
				pipline_state_desc.BlendState.RenderTarget[i].SrcBlendAlpha			= D3D12_BLEND_ONE;
				pipline_state_desc.BlendState.RenderTarget[i].DestBlendAlpha		= D3D12_BLEND_ZERO;
				pipline_state_desc.BlendState.RenderTarget[i].BlendOpAlpha			= D3D12_BLEND_OP_ADD;
				pipline_state_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				pipline_state_desc.BlendState.RenderTarget[i].LogicOpEnable			= FALSE;
				pipline_state_desc.BlendState.RenderTarget[i].LogicOp				= D3D12_LOGIC_OP_CLEAR;
			}
			pipline_state_desc.BlendState.AlphaToCoverageEnable	 = FALSE;
			pipline_state_desc.BlendState.IndependentBlendEnable = FALSE;

			// Depth Stencil State Settings
			pipline_state_desc.DepthStencilState.DepthEnable	  = TRUE; // Enable Depth Test
			pipline_state_desc.DepthStencilState.DepthFunc		  = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			pipline_state_desc.DepthStencilState.DepthWriteMask	  = D3D12_DEPTH_WRITE_MASK_ALL;
			pipline_state_desc.DepthStencilState.StencilEnable	  = FALSE; // Disable Stencil Test
			pipline_state_desc.DepthStencilState.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
			pipline_state_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

			pipline_state_desc.DepthStencilState.FrontFace.StencilFailOp	  = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.FrontFace.StencilPassOp	  = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.FrontFace.StencilFunc		  = D3D12_COMPARISON_FUNC_ALWAYS;

			pipline_state_desc.DepthStencilState.BackFace.StencilFailOp		 = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.BackFace.StencilPassOp		 = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.BackFace.StencilFunc		 = D3D12_COMPARISON_FUNC_ALWAYS;

			pipline_state_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			hr = device->CreateGraphicsPipelineState(&pipline_state_desc, IID_PPV_ARGS(&pipeline_state_render));
			assert(SUCCEEDED(hr) && "Create Graphics Pipeline State");
		}
		// Initialize Functions for Depth Texture
		void init_descriptor_heaps_shadow() {
			HRESULT hr;

			D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc{};
			descriptor_heap_desc.NumDescriptors = 1;
			descriptor_heap_desc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			descriptor_heap_desc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			descriptor_heap_desc.NodeMask		= 0;
			hr = device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&descriptor_heap_shadow));
			assert(SUCCEEDED(hr) && "Create Descriptro Heap for Shadow");
		}
		void init_shadow_resource_and_target_view() {
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
			DIRECTX_ASSERT(hr, "Create Comitted Resource[Shadow]");

			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
			dsv_desc.ViewDimension		= D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Format				= DXGI_FORMAT_D32_FLOAT;
			dsv_desc.Texture2D.MipSlice = 0;
			dsv_desc.Flags				= D3D12_DSV_FLAG_NONE;
			handle_shadow				= descriptor_heap_shadow->GetCPUDescriptorHandleForHeapStart();
			device->CreateDepthStencilView(resource_shadow.Get(), &dsv_desc, handle_shadow);
			DIRECTX_ASSERT(hr, "Create Depth Stencil View[Shadow]");
		}
		void init_pipeline_state_shadow() {
			HRESULT hr;

			UINT							 compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader;
			Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader;

			hr = D3DCompileFromFile(SHADER_NAME_SHADOW_MAPPING, nullptr, nullptr, "VSShadowMap", "vs_5_0",
									compile_flags, 0, &vertex_shader, nullptr);
			DIRECTX_ASSERT(hr, "Compile Vertex Shader");

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
			pipeline_state_desc.pRootSignature		  = root_signature.Get();

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
			DIRECTX_ASSERT(hr, "Create Graphics Pipeline State[Shadow]");
		}
		// Initialize Functions for Postprocess
		void init_descriptor_heap_postprocess() {
			HRESULT hr;

			// Descriptor Heap for Drawing Render Texture
			D3D12_DESCRIPTOR_HEAP_DESC rtv_render_target_heap_desc;
			rtv_render_target_heap_desc.NumDescriptors = 1;
			rtv_render_target_heap_desc.Type		   = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtv_render_target_heap_desc.Flags		   = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtv_render_target_heap_desc.NodeMask	   = 0;
			hr										   = device->CreateDescriptorHeap(&rtv_render_target_heap_desc,
																					  IID_PPV_ARGS(&descriptor_heap_rtv_render_texture));
			assert(SUCCEEDED(hr) && "Create RTV Descriptor Heap[Render Texture]");

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
		void init_postprocess_resource_and_target_view() {
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
			resource_desc.Width				 = WIDTH;
			resource_desc.Height			 = HEIGHT;
			resource_desc.DepthOrArraySize	 = 1;
			resource_desc.MipLevels			 = 1;
			resource_desc.Format			 = DXGI_FORMAT_R8G8B8A8_UNORM;
			resource_desc.Layout			 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resource_desc.SampleDesc.Count	 = 1;
			resource_desc.SampleDesc.Quality = 0;
			resource_desc.Flags				 = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			clear_value.Format	 = DXGI_FORMAT_R8G8B8A8_UNORM;
			clear_value.Color[0] = Global::bg_color[0];
			clear_value.Color[1] = Global::bg_color[1];
			clear_value.Color[2] = Global::bg_color[2];
			clear_value.Color[3] = Global::bg_color[3];

			// Create Render Texture Resource
			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_PRESENT, &clear_value,
												 IID_PPV_ARGS(&resource_render_texture));
			DIRECTX_ASSERT(hr, "Create Comitted Resource[Render Texture]");

			// DIRECTX_ASSERT(hr, "Get Buffer[Render Texture]");
			handle_rtv_render_texture = descriptor_heap_rtv_render_texture->GetCPUDescriptorHandleForHeapStart();
			device->CreateRenderTargetView(resource_render_texture.Get(), nullptr, handle_rtv_render_texture);

			// Create Shader Resource View for Postprocessing
			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
			srv_desc.Format					 = DXGI_FORMAT_R8G8B8A8_UNORM;
			srv_desc.Texture2D.MipLevels	 = 1;
			srv_desc.ViewDimension			 = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			handle_cbv_srv_postprocess = descriptor_heap_cbv_srv_postprocess->GetCPUDescriptorHandleForHeapStart();
			device->CreateShaderResourceView(resource_render_texture.Get(), &srv_desc, handle_cbv_srv_postprocess);
		}
		void init_root_signature_postprocess() {
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
		void init_pipeline_state_postprocess() {
			HRESULT hr;

			UINT							 compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_postprocess;
			Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_postprocess;

			// Stop When Shader Compile Error
			hr = D3DCompileFromFile(SHADER_NAME_POSTPROCESS, nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0,
									&vertex_shader_postprocess, nullptr);
			DIRECTX_ASSERT(hr, "Compile Vertex Shader[Postprocess]");
			hr = D3DCompileFromFile(SHADER_NAME_POSTPROCESS, nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0,
									&pixel_shader_postprocess, nullptr);
			DIRECTX_ASSERT(hr, "Compile Pixel Shader[Postprocess]");

			D3D12_INPUT_ELEMENT_DESC desc_input_elements[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc{};

			pipeline_state_desc.VS.pShaderBytecode = vertex_shader_postprocess->GetBufferPointer();
			pipeline_state_desc.VS.BytecodeLength  = vertex_shader_postprocess->GetBufferSize();
			pipeline_state_desc.PS.pShaderBytecode = pixel_shader_postprocess->GetBufferPointer();
			pipeline_state_desc.PS.BytecodeLength  = pixel_shader_postprocess->GetBufferSize();

			pipeline_state_desc.InputLayout.pInputElementDescs = desc_input_elements;
			pipeline_state_desc.InputLayout.NumElements		   = _countof(desc_input_elements);

			// Render Target Settings
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
			DIRECTX_ASSERT(hr, "Create Graphics Pipeline State[Postprocess]");
		}
		// Initialize Functions for MSAA
		void init_descriptor_heaps_msaa() {
			HRESULT hr;

			// Render Target
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
		void init_msaa_resources_and_target_view() {
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
			resource_desc.Width				 = WIDTH;
			resource_desc.Height			 = HEIGHT;
			resource_desc.DepthOrArraySize	 = 1;
			resource_desc.MipLevels			 = 1;
			resource_desc.Format			 = DXGI_FORMAT_R8G8B8A8_UNORM;
			resource_desc.Layout			 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resource_desc.SampleDesc.Count	 = 4;
			resource_desc.SampleDesc.Quality = 0;
			resource_desc.Flags				 = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			clear_value.Format	 = DXGI_FORMAT_R8G8B8A8_UNORM;
			clear_value.Color[0] = Global::bg_color[0];
			clear_value.Color[1] = Global::bg_color[1];
			clear_value.Color[2] = Global::bg_color[2];
			clear_value.Color[3] = Global::bg_color[3];

			// Create Render Texture Resource
			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_RESOLVE_SOURCE, &clear_value,
												 IID_PPV_ARGS(&resource_msaa_color_texture));
			DIRECTX_ASSERT(hr, "Create Comitted Resource[MSAA Color Texture]");

			resource_desc.Format = DXGI_FORMAT_R32_TYPELESS;
			resource_desc.Flags	 = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_CLEAR_VALUE clear_value_depth{};
			clear_value_depth.Format			   = DXGI_FORMAT_D32_FLOAT;
			clear_value_depth.DepthStencil.Depth   = 1.0f;
			clear_value_depth.DepthStencil.Stencil = 0;

			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value_depth,
												 IID_PPV_ARGS(&resource_msaa_depth_texture));
			DIRECTX_ASSERT(hr, "Create Comitted Resource[MSAA Depth Texture]");

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
		// Initialize Functions for Diferred Rendering
		void init_descriptor_heaps_deferred() {
			HRESULT hr;

			D3D12_DESCRIPTOR_HEAP_DESC desc_rtv = {};
			desc_rtv.Flags						= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc_rtv.NumDescriptors				= 3;
			desc_rtv.Type						= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc_rtv.NodeMask					= 0;
			hr = device->CreateDescriptorHeap(&desc_rtv, IID_PPV_ARGS(&descriptor_heap_rtv_diferred));
			DIRECTX_ASSERT(hr, "Create Descriptor Heap (RTV Diferred)");

			D3D12_DESCRIPTOR_HEAP_DESC desc_cbv_srv_uav = {};
			desc_cbv_srv_uav.Flags						= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			desc_cbv_srv_uav.NumDescriptors				= 5;
			desc_cbv_srv_uav.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // SRV,UAVのデスクリプタをこのヒープに作る
			desc_cbv_srv_uav.NodeMask = 0;
			hr = device->CreateDescriptorHeap(&desc_cbv_srv_uav, IID_PPV_ARGS(&descriptor_heap_cbv_srv_uav_diferred));
			DIRECTX_ASSERT(hr, "Create Descriptor Heap (CBV SRV UAV Diferred)");
		}
		void init_diferred_resources_and_target_view() {}


	public:
		void render() {
			HRESULT hr;

			ID3D12CommandList* const p_command_list = command_list.Get();

			if (Global::is_enabled_shadow_mapping) {
				populate_command_list_shadow();
				execute_command_list(p_command_list);
			}

			if (!Global::is_enabled_msaa) {
				populate_command_list_render();
				execute_command_list(p_command_list);
			} else {

				populate_command_list_msaa();
				execute_command_list(p_command_list);
			}

			if (Global::is_enabled_postprocess) {
				populate_command_list_postprocess();
				execute_command_list(p_command_list);
			}

			populate_command_list_imgui();
			execute_command_list(p_command_list);

			hr = swap_chain->Present(1, 0);
			assert(SUCCEEDED(hr) && "Swapchain Present");

			rtv_index = swap_chain->GetCurrentBackBufferIndex();
		}

		void set_render_objects(std::vector<std::shared_ptr<albedo::Object>> in_objects) {
			render_objects.clear();
			for (std::shared_ptr<albedo::Object> obj : in_objects) {
				render_objects.push_back(obj.get());
				obj->set_shadow_buffer(resource_shadow.Get());
			}
		}
		void set_render_skydome(std::shared_ptr<albedo::Object> in_object) { skydome_object = in_object; }

		inline UINT64				 get_num_frames() { return NUM_FRAMES_IN_FLIGHT; }
		inline ID3D12Device*		 get_device() { return device.Get(); }
		inline ID3D12DescriptorHeap* get_cbv_srv_heap() { return descriptor_heap_cbv_srv.Get(); }
		inline ID3D12DescriptorHeap* get_imgui_heap() { return descriptor_heap_imgui.Get(); }

	private:
		void populate_command_list_shadow() {
			HRESULT hr;

			set_resource_barrier(resource_shadow.Get(), D3D12_RESOURCE_STATE_GENERIC_READ,
								 D3D12_RESOURCE_STATE_DEPTH_WRITE);

			command_list->ClearDepthStencilView(handle_shadow, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			command_list->RSSetViewports(1, &viewport_shadow);
			command_list->RSSetScissorRects(1, &rect_scissor_shadow);
			command_list->OMSetRenderTargets(0, nullptr, TRUE, &handle_shadow);

			command_list->SetDescriptorHeaps(1, descriptor_heap_cbv_srv.GetAddressOf());
			D3D12_CPU_DESCRIPTOR_HANDLE handle_cbv_srv = descriptor_heap_cbv_srv->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE handle_gpu_cbv_srv =
				descriptor_heap_cbv_srv->GetGPUDescriptorHandleForHeapStart();
			const UINT cbv_descriptor_size =
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			command_list->SetGraphicsRootSignature(root_signature.Get());
			command_list->SetPipelineState(pipeline_state_shadow.Get());
			for (albedo::Object* object : render_objects) {
				command_list->SetGraphicsRootDescriptorTable(0, handle_gpu_cbv_srv);
				object->update_directx_resource_views_and_draw(command_list.Get(), handle_cbv_srv);
				handle_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
				handle_gpu_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
			}

			set_resource_barrier(resource_shadow.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
								 D3D12_RESOURCE_STATE_GENERIC_READ);

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [Depth] Closed");
		}
		void populate_command_list_render() {
			HRESULT	   hr;
			const bool flag_postprocess = Global::is_enabled_postprocess;

			set_resource_barrier(resource_render[rtv_index].Get(), D3D12_RESOURCE_STATE_PRESENT,
								 D3D12_RESOURCE_STATE_RENDER_TARGET);
			if (flag_postprocess) {
				set_resource_barrier(resource_render_texture.Get(), D3D12_RESOURCE_STATE_PRESENT,
									 D3D12_RESOURCE_STATE_RENDER_TARGET);
			}

			const FLOAT clear_color[4] = {Global::bg_color[0], Global::bg_color[1], Global::bg_color[2],
										  Global::bg_color[3]};
			command_list->ClearRenderTargetView(handle_rtv[rtv_index], clear_color, 0, nullptr);
			command_list->ClearDepthStencilView(handle_dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			if (flag_postprocess) {
				command_list->ClearRenderTargetView(handle_rtv_render_texture, clear_color, 0, nullptr);
			}

			command_list->RSSetViewports(1, &viewport);
			command_list->RSSetScissorRects(1, &rect_scissor);
			if (Global::is_enabled_postprocess) {
				command_list->OMSetRenderTargets(1, &handle_rtv_render_texture, TRUE, &handle_dsv);
			} else {
				command_list->OMSetRenderTargets(1, &handle_rtv[rtv_index], TRUE, &handle_dsv);
			}

			command_list->SetDescriptorHeaps(1, descriptor_heap_cbv_srv.GetAddressOf());
			D3D12_CPU_DESCRIPTOR_HANDLE handle_cbv_srv = descriptor_heap_cbv_srv->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE handle_gpu_cbv_srv =
				descriptor_heap_cbv_srv->GetGPUDescriptorHandleForHeapStart();
			const UINT cbv_descriptor_size =
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (albedo::Object* object : render_objects) {
				command_list->SetGraphicsRootSignature(object->get_directx_root_signature());
				command_list->SetPipelineState(object->get_directx_pipeline_state());
				command_list->SetGraphicsRootDescriptorTable(0, handle_gpu_cbv_srv);
				object->update_directx_resource_views_and_draw(command_list.Get(), handle_cbv_srv);
				handle_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
				handle_gpu_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
			}

			if (skydome_object && Global::is_enabled_skydome) {
				command_list->SetPipelineState(skydome_object->get_directx_pipeline_state());
				command_list->SetGraphicsRootDescriptorTable(0, handle_gpu_cbv_srv);
				skydome_object->update_directx_resource_views_and_draw(command_list.Get(), handle_cbv_srv);
				handle_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
				handle_gpu_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
			}

			if (flag_postprocess) {
				set_resource_barrier(resource_render_texture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
									 D3D12_RESOURCE_STATE_PRESENT);
			}
			set_resource_barrier(resource_render[rtv_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
								 D3D12_RESOURCE_STATE_PRESENT);

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [Render] Closed");
		}
		void populate_command_list_msaa() {
			HRESULT hr;

			set_resource_barrier(resource_msaa_color_texture.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
								 D3D12_RESOURCE_STATE_RENDER_TARGET);

			const FLOAT clear_color[4] = {Global::bg_color[0], Global::bg_color[1], Global::bg_color[2],
										  Global::bg_color[3]};
			command_list->ClearRenderTargetView(handle_rtv_msaa, clear_color, 0, nullptr);
			command_list->ClearDepthStencilView(handle_dsv_msaa, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			command_list->RSSetViewports(1, &viewport);
			command_list->RSSetScissorRects(1, &rect_scissor);
			command_list->OMSetRenderTargets(1, &handle_rtv_msaa, TRUE, &handle_dsv_msaa);

			command_list->SetDescriptorHeaps(1, descriptor_heap_cbv_srv.GetAddressOf());
			D3D12_CPU_DESCRIPTOR_HANDLE handle_cbv_srv = descriptor_heap_cbv_srv->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE handle_gpu_cbv_srv =
				descriptor_heap_cbv_srv->GetGPUDescriptorHandleForHeapStart();
			const UINT cbv_descriptor_size =
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (albedo::Object* object : render_objects) {
				command_list->SetGraphicsRootSignature(object->get_directx_root_signature());
				command_list->SetPipelineState(object->get_directx_pipeline_state());
				command_list->SetGraphicsRootDescriptorTable(0, handle_gpu_cbv_srv);
				object->update_directx_resource_views_and_draw(command_list.Get(), handle_cbv_srv);
				handle_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
				handle_gpu_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
			}

			if (skydome_object && Global::is_enabled_skydome) {
				command_list->SetGraphicsRootSignature(root_signature.Get());
				command_list->SetPipelineState(skydome_object->get_directx_pipeline_state());
				command_list->SetGraphicsRootDescriptorTable(0, handle_gpu_cbv_srv);
				skydome_object->update_directx_resource_views_and_draw(command_list.Get(), handle_cbv_srv);
				handle_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
				handle_gpu_cbv_srv.ptr += MAX_CRV_SRV_BUFFER_SIZE * cbv_descriptor_size;
			}

			set_resource_barrier(resource_msaa_color_texture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
								 D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

			if (!Global::is_enabled_postprocess) {
				set_resource_barrier(resource_render[rtv_index].Get(), D3D12_RESOURCE_STATE_PRESENT,
									 D3D12_RESOURCE_STATE_RESOLVE_DEST);
				command_list->ResolveSubresource(resource_render[rtv_index].Get(), 0, resource_msaa_color_texture.Get(),
												 0, DXGI_FORMAT_R8G8B8A8_UNORM);
				set_resource_barrier(resource_render[rtv_index].Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST,
									 D3D12_RESOURCE_STATE_PRESENT);
			} else {
				set_resource_barrier(resource_render_texture.Get(), D3D12_RESOURCE_STATE_PRESENT,
									 D3D12_RESOURCE_STATE_RESOLVE_DEST);
				command_list->ResolveSubresource(resource_render_texture.Get(), 0, resource_msaa_color_texture.Get(), 0,
												 DXGI_FORMAT_R8G8B8A8_UNORM);
				set_resource_barrier(resource_render_texture.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST,
									 D3D12_RESOURCE_STATE_PRESENT);
			}

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [Render] Closed");
		}
		void populate_command_list_postprocess() {
			HRESULT hr;

			set_resource_barrier(resource_render[rtv_index].Get(), D3D12_RESOURCE_STATE_PRESENT,
								 D3D12_RESOURCE_STATE_RENDER_TARGET);

			const FLOAT clear_color[4] = {Global::bg_color[0], Global::bg_color[1], Global::bg_color[2],
										  Global::bg_color[3]};
			command_list->ClearRenderTargetView(handle_rtv[rtv_index], clear_color, 0, nullptr);

			command_list->RSSetViewports(1, &viewport);
			command_list->RSSetScissorRects(1, &rect_scissor);
			command_list->OMSetRenderTargets(1, &handle_rtv[rtv_index], TRUE, nullptr);

			command_list->SetGraphicsRootSignature(root_signature_postprocess.Get());
			command_list->SetPipelineState(pipeline_state_postprocess.Get());

			command_list->SetDescriptorHeaps(1, descriptor_heap_cbv_srv_postprocess.GetAddressOf());
			D3D12_GPU_DESCRIPTOR_HANDLE cbv_srv_gpu_handle =
				descriptor_heap_cbv_srv_postprocess->GetGPUDescriptorHandleForHeapStart();
			/*UINT cbv_srv_descriptor_size =
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);*/
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

			set_resource_barrier(resource_render[rtv_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
								 D3D12_RESOURCE_STATE_PRESENT);

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [Postprocess] Closed");
		}
		void populate_command_list_imgui() {
			HRESULT hr;

			set_resource_barrier(resource_render[rtv_index].Get(), D3D12_RESOURCE_STATE_PRESENT,
								 D3D12_RESOURCE_STATE_RENDER_TARGET);

			command_list->OMSetRenderTargets(1, &handle_rtv[rtv_index], TRUE, &handle_dsv);
			command_list->SetGraphicsRootSignature(root_signature.Get());
			command_list->SetPipelineState(pipeline_state_render.Get());

			command_list->SetDescriptorHeaps(1, descriptor_heap_imgui.GetAddressOf());
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());

			set_resource_barrier(resource_render[rtv_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
								 D3D12_RESOURCE_STATE_PRESENT);

			hr = command_list->Close();
			assert(SUCCEEDED(hr) && "Command List [ImGui] Closed");
		}

		void get_hardware_adaptor(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter) {
			*ppAdapter = nullptr;
			for (UINT adapterIndex = 0;; ++adapterIndex) {
				IDXGIAdapter1* pAdapter = nullptr;
				if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter)) {
					// No more adapters to enumerate.
					break;
				}

				// Check to see if the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
					*ppAdapter = pAdapter;
					return;
				}
				pAdapter->Release();
			}
		}
		void set_resource_barrier(ID3D12Resource* in_resource, D3D12_RESOURCE_STATES in_before_state,
								  D3D12_RESOURCE_STATES in_after_state) {
			D3D12_RESOURCE_BARRIER resource_barrier;
			ZeroMemory(&resource_barrier, sizeof(resource_barrier));
			resource_barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resource_barrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resource_barrier.Transition.pResource	= in_resource;
			resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			resource_barrier.Transition.StateBefore = in_before_state;
			resource_barrier.Transition.StateAfter	= in_after_state;

			command_list->ResourceBarrier(1, &resource_barrier);
		}
		void wait_frame() {
			HRESULT h_result;

			const UINT64 fence = frame_number;
			h_result		   = command_queue->Signal(queue_fence.Get(), fence);
			++frame_number;
			assert(SUCCEEDED(h_result) && "Command Queue Signal");

			if (queue_fence->GetCompletedValue() < fence) {
				h_result = queue_fence->SetEventOnCompletion(fence, handle_fence_event);
				assert(SUCCEEDED(h_result) && "Set Event on Completion");
				WaitForSingleObject(handle_fence_event, INFINITE);
			}
		}
		void execute_command_list(ID3D12CommandList* const p_command_list) {
			HRESULT hr;
			command_queue->ExecuteCommandLists(1, &p_command_list);

			wait_frame();

			hr = command_allocator->Reset();
			assert(SUCCEEDED(hr) && "Command Allocator Reset[Depth]");

			hr = command_list->Reset(command_allocator.Get(), nullptr);
			assert(SUCCEEDED(hr) && "Command List Reset[Depth]");
		}
		void set_constant_root_table_by_object(int in_index) {
			command_list->SetDescriptorHeaps(1, descriptor_heap_cbv_srv.GetAddressOf());
			D3D12_GPU_DESCRIPTOR_HANDLE cbv_gpu_handle = descriptor_heap_cbv_srv->GetGPUDescriptorHandleForHeapStart();
			UINT cbv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			cbv_gpu_handle.ptr += cbv_descriptor_size * in_index * MAX_CRV_SRV_BUFFER_SIZE;
			command_list->SetGraphicsRootDescriptorTable(0, cbv_gpu_handle);
		}
	};
} // namespace albedo