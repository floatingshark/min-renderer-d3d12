#pragma once
#include "constant.hpp"
#include "shaders.hpp"
#include "shape.hpp"
#include "texture.hpp"
#include <External/glm/glm.hpp>
#include <External/glm/gtc/matrix_transform.hpp>
#include <External/glm/gtx/string_cast.hpp>
#include <cassert>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>
#include <wrl/client.h>

namespace albedo {
	class Object {
	public:
		Object(ID3D12Device* device, ID3D12DescriptorHeap* heap_cbv) : device(device), descriptor_heap_cbv(heap_cbv) {
			init_object_datum();
			init_directx_contexts();
		};

	private:
		UINT CBUFFER_SIZE = 512;
		UINT TEXTURE_SIZE = 1024;

	protected:
		// General
		std::vector<Shape::Vertex>	   vertex_data;
		std::vector<int>			   index_data;
		std::vector<byte>			   texture_data;
		std::vector<std::vector<byte>> cube_map_data;

		// For DirectX
		ID3D12Device*										device;
		ID3D12DescriptorHeap*								descriptor_heap_cbv;
		Microsoft::WRL::ComPtr<ID3D12Resource>				vertex_resource;
		Microsoft::WRL::ComPtr<ID3D12Resource>				index_resource;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> constant_resource;
		Microsoft::WRL::ComPtr<ID3D12Resource>				texture_resouce;
		Microsoft::WRL::ComPtr<ID3D12Resource>				shadow_resource;
		Microsoft::WRL::ComPtr<ID3D12Resource>				cube_map_resource;
		Microsoft::WRL::ComPtr<ID3DBlob>					vertex_shader;
		Microsoft::WRL::ComPtr<ID3DBlob>					pixel_shader;
		Microsoft::WRL::ComPtr<ID3D12RootSignature>			root_signature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>			pipeline_state;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC					pipeline_state_desc;

	public:
		std::string			  name;
		albedo::Texture::Type texture_type		 = albedo::Texture::Type::Monochrome;
		float				  texture_color[4]	 = {1.f, 1.f, 1.f, 1.f};
		std::string			  texture_file_name	 = "";
		std::string			  cube_map_file_name = "";
		albedo::Shaders::Type shader_type		 = albedo::Shaders::Type::Color;

		glm::vec3 position		 = {0.f, 0.f, 0.f};
		glm::vec3 rotation		 = {0.f, 0.f, 0.f};
		glm::vec3 scale			 = {1.f, 1.f, 1.f};
		float	  specular_power = 100.f;

	public:
		void init_object_datum() {
			albedo::Shape::create_plane(vertex_data, index_data);
			byte color[4] = {255, 255, 255, 255};
			texture_data  = albedo::Texture::create_monochromatic(TEXTURE_SIZE, color);
		}
		void init_directx_contexts() {
			init_directx_resources();
			init_directx_root_signature();
			init_directx_pipeline_state();
		};

		// Update Constant and Shader Resource
		void update_directx_constant_resources(Constant::Scene scene, Constant::Local local) {
			update_directx_constant_resource_1(scene);
			update_directx_constant_resource_2(local);
		}
		// Update Resource Views and Draw Object
		void update_directx_resource_views_and_draw(ID3D12GraphicsCommandList*	command_list,
													D3D12_CPU_DESCRIPTOR_HANDLE in_handle) {
			UINT					 size_vertices = sizeof(Shape::Vertex) * vertex_data.size();
			const size_t			 size_indices  = sizeof(int) * index_data.size();
			D3D12_VERTEX_BUFFER_VIEW vertex_view{};
			D3D12_INDEX_BUFFER_VIEW	 index_view{};
			UINT					 cbv_srv_descriptor_size =
				device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			// Vertex View
			vertex_view.BufferLocation = vertex_resource->GetGPUVirtualAddress();
			vertex_view.StrideInBytes  = sizeof(Shape::Vertex); // Size of 1 Vertex
			vertex_view.SizeInBytes	   = size_vertices;			// Size of All Vertices

			// Index View
			index_view.BufferLocation = index_resource->GetGPUVirtualAddress();
			index_view.Format		  = DXGI_FORMAT_R32_UINT;
			index_view.SizeInBytes	  = size_indices;

			// Consntant Buffer View Scene (Index 1)
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbuff_desc = {};
			cbuff_desc.BufferLocation				   = constant_resource[0]->GetGPUVirtualAddress();
			cbuff_desc.SizeInBytes					   = CBUFFER_SIZE;
			device->CreateConstantBufferView(&cbuff_desc, in_handle);

			// Consntant Buffer View Local (Index 2)
			cbuff_desc.BufferLocation = constant_resource[1]->GetGPUVirtualAddress();
			in_handle.ptr += cbv_srv_descriptor_size;
			device->CreateConstantBufferView(&cbuff_desc, in_handle);

			// Shader Resource View Texture (Index 3)
			D3D12_SHADER_RESOURCE_VIEW_DESC tex_desc{};
			tex_desc.Format						   = DXGI_FORMAT_R8G8B8A8_UNORM;
			tex_desc.ViewDimension				   = D3D12_SRV_DIMENSION_TEXTURE2D;
			tex_desc.Texture2D.MipLevels		   = 1;
			tex_desc.Texture2D.MostDetailedMip	   = 0;
			tex_desc.Texture2D.PlaneSlice		   = 0;
			tex_desc.Texture2D.ResourceMinLODClamp = 0.0F;
			tex_desc.Shader4ComponentMapping	   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			in_handle.ptr += cbv_srv_descriptor_size;
			device->CreateShaderResourceView(texture_resouce.Get(), &tex_desc, in_handle);

			// Shader Resource View Depth Texture (Index 4)
			tex_desc.Format = DXGI_FORMAT_R32_FLOAT;
			in_handle.ptr += cbv_srv_descriptor_size;
			device->CreateShaderResourceView(shadow_resource.Get(), &tex_desc, in_handle);

			// Shader Resource View Cube Map Texture (Index 5)
			tex_desc.Format						 = DXGI_FORMAT_R8G8B8A8_UNORM;
			tex_desc.ViewDimension				 = D3D12_SRV_DIMENSION_TEXTURECUBE;
			tex_desc.TextureCube.MipLevels		 = 1;
			tex_desc.TextureCube.MostDetailedMip = 0;
			in_handle.ptr += cbv_srv_descriptor_size;
			device->CreateShaderResourceView(cube_map_resource.Get(), &tex_desc, in_handle);

			command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			command_list->IASetVertexBuffers(0, 1, &vertex_view);
			command_list->IASetIndexBuffer(&index_view);

			command_list->DrawIndexedInstanced(index_data.size(), 1, 0, 0, 0);
		}

		inline ID3D12RootSignature* get_directx_root_signature() { return root_signature.Get(); }
		inline ID3D12PipelineState* get_directx_pipeline_state() { return pipeline_state.Get(); }

		void set_shadow_buffer(ID3D12Resource* in_resource) { shadow_resource = in_resource; }
		void set_vertex_data(albedo::Shape::Type in_type) {
			vertex_data.clear();
			index_data.clear();

			switch (in_type) {
			case Shape::Type::Plane:
				albedo::Shape::create_plane(vertex_data, index_data);
				break;
			case Shape::Type::Cube:
				albedo::Shape::create_cube(vertex_data, index_data);
				break;
			case Shape::Type::Torus:
				albedo::Shape::create_torus(vertex_data, index_data);
				break;
			default:
				break;
			}

			init_directx_contexts();
		}
		void set_texture_data(albedo::Texture::Type in_type) {
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
			case Texture::Type::CheckerBoard:
				texture_data = Texture::create_checker_board(TEXTURE_SIZE, color, checker_num);
				break;
			case Texture::Type::Image:
				Texture::read_bmp_file(texture_file_name.c_str(), texture_data);
				break;
			default:
				break;
			}

			const D3D12_BOX box = {0, 0, 0, TEXTURE_SIZE, TEXTURE_SIZE, 1};
			hr					= texture_resouce->WriteToSubresource(0, &box, texture_data.data(), 4 * TEXTURE_SIZE,
																	  4 * TEXTURE_SIZE * TEXTURE_SIZE);
			assert(SUCCEEDED(hr) && "Write to Subrecource");
		}
		void set_cubemap_data(const char* in_file_name) {
			HRESULT hr;

			const UINT width  = TEXTURE_SIZE;
			const UINT height = TEXTURE_SIZE;

			cube_map_data.clear();
			Texture::read_bmp_cube_file(in_file_name, cube_map_data, width);

			const D3D12_BOX box = {0, 0, 0, width, height, 1};
			for (int i = 0; i < 6; i++) {
				hr = cube_map_resource->WriteToSubresource(i, &box, cube_map_data[i].data(), 4 * width,
														   4 * width * height);
				assert(SUCCEEDED(hr) && "Write to Subrecource[CubeMap]");
			}
		}

		void reset_directx_shader(albedo::Shaders::Type in_type) {
			shader_type = in_type;
			init_directx_pipeline_state();
		}
		void reset_directx_render_pipeline_state() { init_directx_pipeline_state(); }

	protected:
		void init_directx_resources() {
			// Create Buffers
			HRESULT				  hr;
			D3D12_HEAP_PROPERTIES heap_properties{};
			D3D12_RESOURCE_DESC	  resource_desc{};

			// Create Vertex Buffer Resource
			heap_properties.Type				 = D3D12_HEAP_TYPE_UPLOAD;
			heap_properties.CPUPageProperty		 = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_properties.CreationNodeMask	 = 1;
			heap_properties.VisibleNodeMask		 = 1;
			resource_desc.Dimension				 = D3D12_RESOURCE_DIMENSION_BUFFER;
			resource_desc.Width					 = sizeof(Shape::Vertex) * vertex_data.size();
			resource_desc.Height				 = 1;
			resource_desc.DepthOrArraySize		 = 1;
			resource_desc.MipLevels				 = 1;
			resource_desc.Format				 = DXGI_FORMAT_UNKNOWN;
			resource_desc.Layout				 = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resource_desc.SampleDesc.Count		 = 1;
			resource_desc.SampleDesc.Quality	 = 0;
			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												 IID_PPV_ARGS(&vertex_resource));
			assert(SUCCEEDED(hr) && "Create Committed Resource Vertex Buffer");

			// Create Index Buffer Resource
			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												 IID_PPV_ARGS(&index_resource));
			assert(SUCCEEDED(hr) && "Create Committed Resource Index Buffer");

			// Create Constant Buffer Resources
			resource_desc.Width		 = CBUFFER_SIZE;
			constexpr int cbuff_size = 2;
			constant_resource.resize(cbuff_size);
			for (int i = 0; i < 2; i++) {
				hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
													 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
													 IID_PPV_ARGS(&constant_resource[i]));
				assert(SUCCEEDED(hr) && "Create Committed Resource Constant Buffer");
			}

			// Create Texture Buffer Resource
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
												 IID_PPV_ARGS(&texture_resouce));
			assert(SUCCEEDED(hr) && "Create Committed Resource Texture Buffer");

			// Create Cube Map Resource
			resource_desc.DepthOrArraySize = 6;
			hr = device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
												 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
												 IID_PPV_ARGS(&cube_map_resource));
			assert(SUCCEEDED(hr) && "Create Committed Resource Texture Buffer");

			// Init Buffers
			void*		 Mapped;
			const UINT	 size_vertices = sizeof(Shape::Vertex) * vertex_data.size();
			const size_t size_indices  = sizeof(int) * index_data.size();

			hr = vertex_resource->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Fialed Vertex Buffer Mapping");
			CopyMemory(Mapped, vertex_data.data(), size_vertices);
			vertex_resource->Unmap(0, nullptr);
			Mapped = nullptr;

			hr = index_resource->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Failed Index Buffer Mapping");
			CopyMemory(Mapped, index_data.data(), size_indices);
			index_resource->Unmap(0, nullptr);
			Mapped = nullptr;

			const D3D12_BOX box = {0, 0, 0, TEXTURE_SIZE, TEXTURE_SIZE, 1};
			hr					= texture_resouce->WriteToSubresource(0, &box, texture_data.data(), 4 * TEXTURE_SIZE,
																	  4 * TEXTURE_SIZE * TEXTURE_SIZE);
			assert(SUCCEEDED(hr) && "Write to Subrecource");
		}
		void init_directx_root_signature() {
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
		void init_directx_pipeline_state() {
			HRESULT hr;

			UINT							 compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader;
			Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader;

			const wchar_t* shader_name = albedo::Shaders::get_shader_name(shader_type);
			hr = D3DCompileFromFile(shader_name, nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0, &vertex_shader,
									nullptr);
			hr = D3DCompileFromFile(shader_name, nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0, &pixel_shader,
									nullptr);

			// Vertex Layout
			D3D12_INPUT_ELEMENT_DESC desc_input_elements[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			};

			ZeroMemory(&pipeline_state_desc, sizeof(pipeline_state_desc));

			// Shader Settings
			pipeline_state_desc.VS.pShaderBytecode = vertex_shader->GetBufferPointer();
			pipeline_state_desc.VS.BytecodeLength  = vertex_shader->GetBufferSize();
			pipeline_state_desc.HS.pShaderBytecode = nullptr;
			pipeline_state_desc.HS.BytecodeLength  = 0;
			pipeline_state_desc.DS.pShaderBytecode = nullptr;
			pipeline_state_desc.DS.BytecodeLength  = 0;
			pipeline_state_desc.GS.pShaderBytecode = nullptr;
			pipeline_state_desc.GS.BytecodeLength  = 0;
			pipeline_state_desc.PS.pShaderBytecode = pixel_shader->GetBufferPointer();
			pipeline_state_desc.PS.BytecodeLength  = pixel_shader->GetBufferSize();

			// Input Layout Settings
			pipeline_state_desc.InputLayout.pInputElementDescs = desc_input_elements;
			pipeline_state_desc.InputLayout.NumElements		   = _countof(desc_input_elements);

			// Sample Settings
			pipeline_state_desc.SampleDesc.Count   = Global::is_enabled_msaa ? 4 : 1;
			pipeline_state_desc.SampleDesc.Quality = 0;
			pipeline_state_desc.SampleMask		   = UINT_MAX;

			// Render Target Settings
			pipeline_state_desc.NumRenderTargets = 1;
			pipeline_state_desc.RTVFormats[0]	 = DXGI_FORMAT_R8G8B8A8_UNORM;

			// Primitive Topology Type Settings
			pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			// Root Signature
			pipeline_state_desc.pRootSignature = root_signature.Get();

			// Rasterizer State Settings
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

			// Blend State Settings
			for (int i = 0; i < static_cast<int>(_countof(pipeline_state_desc.BlendState.RenderTarget)); ++i) {
				pipeline_state_desc.BlendState.RenderTarget[i].BlendEnable			 = FALSE;
				pipeline_state_desc.BlendState.RenderTarget[i].SrcBlend				 = D3D12_BLEND_ONE;
				pipeline_state_desc.BlendState.RenderTarget[i].DestBlend			 = D3D12_BLEND_ZERO;
				pipeline_state_desc.BlendState.RenderTarget[i].BlendOp				 = D3D12_BLEND_OP_ADD;
				pipeline_state_desc.BlendState.RenderTarget[i].SrcBlendAlpha		 = D3D12_BLEND_ONE;
				pipeline_state_desc.BlendState.RenderTarget[i].DestBlendAlpha		 = D3D12_BLEND_ZERO;
				pipeline_state_desc.BlendState.RenderTarget[i].BlendOpAlpha			 = D3D12_BLEND_OP_ADD;
				pipeline_state_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				pipeline_state_desc.BlendState.RenderTarget[i].LogicOpEnable		 = FALSE;
				pipeline_state_desc.BlendState.RenderTarget[i].LogicOp				 = D3D12_LOGIC_OP_CLEAR;
			}
			pipeline_state_desc.BlendState.AlphaToCoverageEnable  = FALSE;
			pipeline_state_desc.BlendState.IndependentBlendEnable = FALSE;

			// Depth Stencil State Settings
			pipeline_state_desc.DepthStencilState.DepthEnable	   = TRUE; // Enable Depth Test
			pipeline_state_desc.DepthStencilState.DepthFunc		   = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			pipeline_state_desc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
			pipeline_state_desc.DepthStencilState.StencilEnable	   = FALSE; // Disable Stencil Test
			pipeline_state_desc.DepthStencilState.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
			pipeline_state_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

			pipeline_state_desc.DepthStencilState.FrontFace.StencilFailOp	   = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.FrontFace.StencilPassOp	   = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.FrontFace.StencilFunc		   = D3D12_COMPARISON_FUNC_ALWAYS;

			pipeline_state_desc.DepthStencilState.BackFace.StencilFailOp	  = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.BackFace.StencilPassOp	  = D3D12_STENCIL_OP_KEEP;
			pipeline_state_desc.DepthStencilState.BackFace.StencilFunc		  = D3D12_COMPARISON_FUNC_ALWAYS;

			pipeline_state_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			hr = device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline_state));
			assert(SUCCEEDED(hr) && "Create Graphics Pipeline State");
		}
		// Constant Buffer 1 Supports Scene Mutual Variables
		void update_directx_constant_resource_1(Constant::Scene scene) {
			HRESULT hr;
			void*	Mapped;

			// calculate_scene(scene);

			hr = constant_resource[0]->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Constant Buffer Mappded[Scene]");
			CopyMemory(Mapped, &scene, sizeof(scene));
			constant_resource[0]->Unmap(0, nullptr);
			Mapped = nullptr;
		}
		// Constant Buffer 2 Supports Local Variables
		void update_directx_constant_resource_2(Constant::Local local) {
			HRESULT hr;
			void*	Mapped;

			calculate_local_variables(local);

			hr = constant_resource[1]->Map(0, nullptr, &Mapped);
			assert(SUCCEEDED(hr) && "Constant Buffer Mappded[Object]");
			CopyMemory(Mapped, &local, sizeof(local));
			constant_resource[1]->Unmap(0, nullptr);
			Mapped = nullptr;
		}
		// void calculate_scene(Constant::Scene& scene) {}
		void calculate_local_variables(Constant::Local& local) {
			local.world			 = glm::translate(local.world, position);
			local.world			 = glm::rotate(local.world, rotation[0], {1.f, 0.f, 0.f});
			local.world			 = glm::rotate(local.world, rotation[1], {0.f, 1.f, 0.f});
			local.world			 = glm::rotate(local.world, rotation[2], {0.f, 0.f, 1.f});
			local.world			 = glm::scale(local.world, scale);
			local.specular_power = specular_power;
		}
	};
} // namespace albedo