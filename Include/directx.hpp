#include <iostream>
#include <cassert>
#include <Windows.h>
#include <wrl.h>

#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3d12sdklayers.h>

namespace arabesque
{
	class DirectX
	{
	protected:
		Microsoft::WRL::ComPtr<IDXGIFactory4> DXGI_factory;
		Microsoft::WRL::ComPtr<ID3D12Device> device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;
		Microsoft::WRL::ComPtr<ID3D12Fence> queue_fence;
		bool use_warp_device = false;

	public:
		void init_directx()
		{
			init_device();
			init_command_queue();
			init_swap_chain();
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
			h_result = CreateDXGIFactory2(flags_DXGI_factory, IID_PPV_ARGS(&DXGI_factory));
			assert(SUCCEEDED(h_result) && "Failed to create DXGI Factory");

			// Create D3D12 device
			if (use_warp_device)
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter> warp_adapter;
				this->DXGI_factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter));
				D3D12CreateDevice(
					warp_adapter.Get(),
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&device));
			}
			else
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter1> hardware_adapter;
				GetHardwareAdapter(DXGI_factory.Get(), &hardware_adapter);
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
			queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queue_desc.Priority = 0;
			queue_desc.NodeMask = 0;
			h_result = this->device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue));
			assert(SUCCEEDED(h_result) && "Create Command Queue");

			HANDLE handleFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
			assert(handleFenceEvent && "Create Fence Event Handle");
			h_result = this->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queue_fence));
			assert(SUCCEEDED(h_result) && "Create Fence");
		}
		void init_swap_chain()
		{
			
		}

		// https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
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
	};
}