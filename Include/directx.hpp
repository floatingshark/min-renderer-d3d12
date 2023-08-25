#pragma once
#define _XM_NO_XMVECTOR_OVERLOADS_
#define RTV_BUFFER_NUM (2)

#include <iostream>
#include <cassert>
#include <vector>
#include <wrl/client.h>
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3d12sdklayers.h>
#include <imgui/imgui.h>
#include "constant.hpp"
#include "global.hpp"
#include "object.hpp"
#include "shape.hpp"

namespace arabesques
{
	class DirectXA
	{
	public:
		DirectXA(HWND h) : hwnd(h) {}
		~DirectXA() {}

	protected:
		HWND hwnd;
		UINT width = 800;
		UINT height = 600;
		UINT64 frame_index = 1;
		std::vector<arabesques::Object> objects;
		const UINT NUM_FRAMES_IN_FLIGHT = 2;
		const bool USE_WARP_DEVICE = false;
		UINT RTVIdx = 0;
		D3D12_VIEWPORT viewport;
		D3D12_RECT rect_scissor;
		Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
		Microsoft::WRL::ComPtr<ID3D12Device> device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;
		Microsoft::WRL::ComPtr<ID3D12Fence> queue_fence;
		HANDLE handle_fence_event;
		IDXGISwapChain3 *swap_chain;
		D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle[RTV_BUFFER_NUM];
		D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srv_heap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsb_heap;
		Microsoft::WRL::ComPtr<ID3D12Resource> depth_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> render_targets[RTV_BUFFER_NUM];
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
		Microsoft::WRL::ComPtr<ID3DBlob> root_signature_blob;
		Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state;
		Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader;
		Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader;

	public:
		// Initialize Functions
		void init_directx()
		{
			init_viewport();
			init_device();
			init_command_queue();
			init_swap_chain();
			init_descriptor_heap();
			init_render_target();
			init_depth_buffer();
			init_command_list();
			init_root_signature();
			init_shader();
			init_pipeline_state_object();

			// render();
		}
		void init_viewport()
		{
			viewport.TopLeftX = 0.f;
			viewport.TopLeftY = 0.f;
			viewport.Width = (FLOAT)width;
			viewport.Height = (FLOAT)height;
			viewport.MinDepth = 0.f;
			viewport.MaxDepth = 1.f;

			rect_scissor.top = 0;
			rect_scissor.left = 0;
			rect_scissor.right = width;
			rect_scissor.bottom = height;
		}
		void init_device()
		{
			HRESULT h_result;
			UINT flags_DXGI_factory = 0;
#if _DEBUG || 1
			//  Enable Debug Layer
			Microsoft::WRL::ComPtr<ID3D12Debug> dx_debug = nullptr;
			h_result = D3D12GetDebugInterface(IID_PPV_ARGS(&dx_debug));
			assert(SUCCEEDED(h_result) && "Failed to get D3D12 Debug Layer");
			dx_debug->EnableDebugLayer();
			flags_DXGI_factory |= DXGI_CREATE_FACTORY_DEBUG;
#endif

			// Create DXGI Factory
			h_result = CreateDXGIFactory2(flags_DXGI_factory, IID_PPV_ARGS(&factory));
			assert(SUCCEEDED(h_result) && "Failed to create DXGI Factory");

			// Create D3D12 device
			if (USE_WARP_DEVICE)
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter> warp_adapter;
				this->factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter));
				D3D12CreateDevice(
					warp_adapter.Get(),
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&device));
			}
			else
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter1> hardware_adapter;
				GetHardwareAdapter(factory.Get(), &hardware_adapter);
				D3D12CreateDevice(
					hardware_adapter.Get(),
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&device));
			}
		}
		void init_command_queue()
		{
			HRESULT h_result;
			D3D12_COMMAND_QUEUE_DESC queue_desc = {};
			queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE | D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
			queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queue_desc.Priority = 0;
			queue_desc.NodeMask = 0;
			h_result = this->device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue));
			assert(SUCCEEDED(h_result) && "Create Command Queue");

			handle_fence_event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
			assert(handle_fence_event && "Create Fence Event Handle");
			h_result = this->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queue_fence));
			assert(SUCCEEDED(h_result) && "Create Fence");
		}
		void init_swap_chain()
		{
			HRESULT h_result;

			DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
			ZeroMemory(&swapchain_desc, sizeof(swapchain_desc));
			swapchain_desc.BufferCount = NUM_FRAMES_IN_FLIGHT;
			swapchain_desc.BufferDesc.Width = width;
			swapchain_desc.BufferDesc.Height = height;
			swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapchain_desc.OutputWindow = hwnd;
			swapchain_desc.SampleDesc.Count = 1;
			swapchain_desc.Windowed = TRUE;

			IDXGISwapChain *temp_swap_chain;
			h_result = factory->CreateSwapChain(command_queue.Get(),
												&swapchain_desc,
												&temp_swap_chain);
			assert(SUCCEEDED(h_result) && "Create Swap Chain");
			// temp_swap_chain.As(&swap_chain);
			temp_swap_chain->QueryInterface(&swap_chain);
			temp_swap_chain->Release();
			temp_swap_chain = nullptr;
		}
		void init_descriptor_heap()
		{
			HRESULT h_result;

			D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
			ZeroMemory(&rtv_heap_desc, sizeof(rtv_heap_desc));
			rtv_heap_desc.NumDescriptors = 2;
			rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtv_heap_desc.NodeMask = 0;
			h_result = device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));
			assert(SUCCEEDED(h_result) && "Create RTV Descriptor Heap");

			D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc;
			srv_heap_desc.NumDescriptors = 1;
			srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			srv_heap_desc.NodeMask = 0;
			h_result = device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&srv_heap));

			D3D12_DESCRIPTOR_HEAP_DESC dsb_heap_desc;
			ZeroMemory(&dsb_heap_desc, sizeof(dsb_heap_desc));
			dsb_heap_desc.NumDescriptors = 1;
			dsb_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsb_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			dsb_heap_desc.NodeMask = 0;
			h_result = device->CreateDescriptorHeap(&dsb_heap_desc, IID_PPV_ARGS(&dsb_heap));
			assert(SUCCEEDED(h_result) && "Create DSB Descriptor Heap");
		}
		void init_render_target()
		{
			HRESULT h_result;
			UINT rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			for (UINT i = 0; i < RTV_BUFFER_NUM; i++)
			{
				h_result = swap_chain->GetBuffer(i, IID_PPV_ARGS(&render_targets[i]));
				assert(SUCCEEDED(h_result) && "Get Buffer");

				rtv_handle[i] = rtv_heap->GetCPUDescriptorHandleForHeapStart();
				rtv_handle[i].ptr += rtv_descriptor_size * i;
				device->CreateRenderTargetView(render_targets[i].Get(), nullptr, rtv_handle[i]);
			}
		}
		void init_depth_buffer()
		{
			HRESULT h_result;

			D3D12_RESOURCE_DESC depth_desc;
			D3D12_HEAP_PROPERTIES heap_props;
			D3D12_CLEAR_VALUE clear_value;
			ZeroMemory(&depth_desc, sizeof(depth_desc));
			ZeroMemory(&heap_props, sizeof(heap_props));
			ZeroMemory(&clear_value, sizeof(clear_value));

			depth_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depth_desc.Width = width;
			depth_desc.Height = height;
			depth_desc.DepthOrArraySize = 1;
			depth_desc.MipLevels = 0;
			depth_desc.Format = DXGI_FORMAT_R32_TYPELESS;
			depth_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depth_desc.SampleDesc.Count = 1;
			depth_desc.SampleDesc.Quality = 0;
			depth_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
			heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_props.CreationNodeMask = 0;
			heap_props.VisibleNodeMask = 0;

			clear_value.Format = DXGI_FORMAT_D32_FLOAT;
			clear_value.DepthStencil.Depth = 1.0f;
			clear_value.DepthStencil.Stencil = 0;

			h_result = device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &depth_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(&depth_buffer));
			assert(SUCCEEDED(h_result) && "Create Committed Resource");

			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
			ZeroMemory(&dsv_desc, sizeof(dsv_desc));
			dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
			dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Texture2D.MipSlice = 0;
			dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
			dsv_handle = dsb_heap->GetCPUDescriptorHandleForHeapStart();

			device->CreateDepthStencilView(depth_buffer.Get(), &dsv_desc, dsv_handle);
		}
		void init_command_list()
		{
			HRESULT h_result;

			h_result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator));
			assert(SUCCEEDED(h_result) && "Create Command Allocator");

			// コマンドアロケータとバインドしてコマンドリストを作成する
			h_result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), nullptr, IID_PPV_ARGS(&command_list));
			assert(SUCCEEDED(h_result) && "Create Command List");
		}
		void init_shader()
		{
			HRESULT h_result;
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			// Stop When Shader Compile Error
			h_result = D3DCompileFromFile(L"./Source/Shader/PhongShaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertex_shader, nullptr);
			assert(SUCCEEDED(h_result) && "Compile Vertex Shader");
			h_result = D3DCompileFromFile(L"./Source/Shader/PhongShaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixel_shader, nullptr);
			assert(SUCCEEDED(h_result) && "Compile Pixel Shader");
		}
		void init_root_signature()
		{
			HRESULT h_result;
			D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
			D3D12_ROOT_PARAMETER RootParameters[2];

			ZeroMemory(&RootSignatureDesc, sizeof(RootSignatureDesc));
			ZeroMemory(&RootParameters[0], sizeof(RootParameters[0]));
			ZeroMemory(&RootParameters[1], sizeof(RootParameters[1]));

			RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			RootParameters[0].Descriptor.ShaderRegister = 0;
			RootParameters[0].Descriptor.RegisterSpace = 0;

			RootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			RootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			RootParameters[1].Descriptor.ShaderRegister = 1;
			RootParameters[1].Descriptor.RegisterSpace = 0;

			RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			RootSignatureDesc.NumParameters = _countof(RootParameters);
			RootSignatureDesc.pParameters = RootParameters;

			h_result = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &root_signature_blob, &error_blob);
			assert(SUCCEEDED(h_result) && "Serialize Root Signature");

			h_result = device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&root_signature));
			assert(SUCCEEDED(h_result) && "Create Root Signature");
		}
		void init_pipeline_state_object()
		{
			HRESULT h_result;

			// Vertex Layout
			D3D12_INPUT_ELEMENT_DESC desc_input_elements[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipline_state_desc;
			ZeroMemory(&pipline_state_desc, sizeof(pipline_state_desc));

			// Shader Settings
			pipline_state_desc.VS.pShaderBytecode = vertex_shader->GetBufferPointer();
			pipline_state_desc.VS.BytecodeLength = vertex_shader->GetBufferSize();
			pipline_state_desc.HS.pShaderBytecode = nullptr;
			pipline_state_desc.HS.BytecodeLength = 0;
			pipline_state_desc.DS.pShaderBytecode = nullptr;
			pipline_state_desc.DS.BytecodeLength = 0;
			pipline_state_desc.GS.pShaderBytecode = nullptr;
			pipline_state_desc.GS.BytecodeLength = 0;
			pipline_state_desc.PS.pShaderBytecode = pixel_shader->GetBufferPointer();
			pipline_state_desc.PS.BytecodeLength = pixel_shader->GetBufferSize();

			// Input Layout Settings
			pipline_state_desc.InputLayout.pInputElementDescs = desc_input_elements;
			pipline_state_desc.InputLayout.NumElements = _countof(desc_input_elements);

			// Sample Settings
			pipline_state_desc.SampleDesc.Count = 1;
			pipline_state_desc.SampleDesc.Quality = 0;
			pipline_state_desc.SampleMask = UINT_MAX;

			// Render Target Settings
			pipline_state_desc.NumRenderTargets = 1;
			pipline_state_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

			// Primitive Topology Type Settings
			pipline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			// Root Signature
			pipline_state_desc.pRootSignature = root_signature.Get();

			// Rasterizer State Settings
			pipline_state_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			pipline_state_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			pipline_state_desc.RasterizerState.FrontCounterClockwise = FALSE;
			pipline_state_desc.RasterizerState.DepthBias = 0;
			pipline_state_desc.RasterizerState.DepthBiasClamp = 0;
			pipline_state_desc.RasterizerState.SlopeScaledDepthBias = 0;
			pipline_state_desc.RasterizerState.DepthClipEnable = TRUE;
			pipline_state_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			pipline_state_desc.RasterizerState.AntialiasedLineEnable = FALSE;
			pipline_state_desc.RasterizerState.MultisampleEnable = FALSE;

			// Blend State Settings
			for (int i = 0; i < _countof(pipline_state_desc.BlendState.RenderTarget); ++i)
			{
				pipline_state_desc.BlendState.RenderTarget[i].BlendEnable = FALSE;
				pipline_state_desc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
				pipline_state_desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
				pipline_state_desc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
				pipline_state_desc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
				pipline_state_desc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
				pipline_state_desc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
				pipline_state_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				pipline_state_desc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
				pipline_state_desc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_CLEAR;
			}
			pipline_state_desc.BlendState.AlphaToCoverageEnable = FALSE;
			pipline_state_desc.BlendState.IndependentBlendEnable = FALSE;

			// Depth Stencil State Settings
			pipline_state_desc.DepthStencilState.DepthEnable = TRUE; // Enable Depth Test
			pipline_state_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			pipline_state_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			pipline_state_desc.DepthStencilState.StencilEnable = FALSE; // Disable Stencil Test
			pipline_state_desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
			pipline_state_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

			pipline_state_desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

			pipline_state_desc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			pipline_state_desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

			pipline_state_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			h_result = device->CreateGraphicsPipelineState(&pipline_state_desc, IID_PPV_ARGS(&pipeline_state));
			assert(SUCCEEDED(h_result) && "Create Graphics Pipeline State");
		}

		// Update Functions
		void wait_frame()
		{
			HRESULT h_result;

			const UINT64 fence = frame_index;
			h_result = command_queue->Signal(queue_fence.Get(), fence);
			++frame_index;
			assert(SUCCEEDED(h_result) && "Command Queue Signal");

			if (queue_fence->GetCompletedValue() < fence)
			{
				h_result = queue_fence->SetEventOnCompletion(fence, handle_fence_event);
				assert(SUCCEEDED(h_result) && "Set Event on Completion");
				WaitForSingleObject(handle_fence_event, INFINITE);
			}
		}
		void populate_command_list()
		{
			HRESULT h_result;

			FLOAT ClearColor[4] = {Global::color[0], Global::color[1], Global::color[2], Global::color[3]};

			SetResourceBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			command_list->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			command_list->ClearRenderTargetView(rtv_handle[RTVIdx], ClearColor, 0, nullptr);

			command_list->SetGraphicsRootSignature(root_signature.Get());
			command_list->SetPipelineState(pipeline_state.Get());

			command_list->RSSetViewports(1, &viewport);
			command_list->RSSetScissorRects(1, &rect_scissor);
			command_list->OMSetRenderTargets(1, &rtv_handle[RTVIdx], TRUE, &dsv_handle);

			for (arabesques::Object &object : objects)
			{
				object.draw_directx(command_list.Get());
			}

			command_list->SetDescriptorHeaps(1, srv_heap.GetAddressOf());
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());

			SetResourceBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			h_result = command_list->Close();
			assert(SUCCEEDED(h_result) && "Command List Closed");
		}
		void render()
		{
			HRESULT h_result;

			populate_command_list();

			ID3D12CommandList *const ppCommandList = command_list.Get();
			command_queue->ExecuteCommandLists(1, &ppCommandList);

			wait_frame();

			h_result = command_allocator->Reset();
			assert(SUCCEEDED(h_result) && "Command Allocator Reset");

			h_result = command_list->Reset(command_allocator.Get(), nullptr);
			assert(SUCCEEDED(h_result) && "Command List Reset");

			h_result = swap_chain->Present(1, 0);
			assert(SUCCEEDED(h_result) && "Swapchain Present");

			RTVIdx = swap_chain->GetCurrentBackBufferIndex();
		}

		// Setter Functions
		void set_objects(std::vector<arabesques::Object> in_objects)
		{
			objects = in_objects;
		}

		// Getter Functions
		inline UINT64 get_num_frames()
		{
			return NUM_FRAMES_IN_FLIGHT;
		}
		inline ID3D12Device *get_device()
		{
			return device.Get();
		}
		inline ID3D12DescriptorHeap *get_srv_heap()
		{
			return srv_heap.Get();
		}

	protected:
		void GetHardwareAdapter(IDXGIFactory4 *pFactory, IDXGIAdapter1 **ppAdapter)
		{
			*ppAdapter = nullptr;
			for (UINT adapterIndex = 0;; ++adapterIndex)
			{
				IDXGIAdapter1 *pAdapter = nullptr;
				if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
				{
					// No more adapters to enumerate.
					break;
				}

				// Check to see if the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				{
					*ppAdapter = pAdapter;
					return;
				}
				pAdapter->Release();
			}
		}
		void SetResourceBarrier(D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState)
		{
			D3D12_RESOURCE_BARRIER ResourceBarrier;
			ZeroMemory(&ResourceBarrier, sizeof(ResourceBarrier));
			ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			ResourceBarrier.Transition.pResource = render_targets[RTVIdx].Get();
			ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			ResourceBarrier.Transition.StateBefore = BeforeState;
			ResourceBarrier.Transition.StateAfter = AfterState;

			command_list->ResourceBarrier(1, &ResourceBarrier);
		}
	};
}