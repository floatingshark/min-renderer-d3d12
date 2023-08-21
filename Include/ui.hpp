#include <iostream>
#include <cassert>
#include <vector>
#include <dxgi.h>
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3d12sdklayers.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_dx12.h>
#include "constant.hpp"
#include "global.hpp"

namespace arabesques
{
	class UI
	{
	protected:
		float color[4];

	public:
		void init_imgui()
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO &io = ImGui::GetIO();
			(void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::StyleColorsLight();
		}
		void init_imgui_glfw(GLFWwindow *window)
		{
			ImGui_ImplGlfw_InitForOther(window, true);
			std::cout << "Initialized ImGui GLFW Backend" << std::endl;
		}
		void init_imgui_directX(ID3D12Device *device, UINT num_frames, ID3D12DescriptorHeap *srv_heap)
		{
			ImGui_ImplDX12_Init(device, num_frames,
								DXGI_FORMAT_R8G8B8A8_UNORM, srv_heap,
								srv_heap->GetCPUDescriptorHandleForHeapStart(),
								srv_heap->GetGPUDescriptorHandleForHeapStart());
			std::cout << "Initialized ImGui DirectX12 Backend" << std::endl;
		}
		void update()
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			if (bool use_demo = true)
			{
				ImGui::ShowDemoWindow(&use_demo);
			}
			window1();
		}
		void render()
		{
			ImGui::Render();
		}
		void shutdown()
		{
			ImGui_ImplDX12_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}
		inline ImGuiIO &get_io()
		{
			return ImGui::GetIO();
		}

	protected:
		void window1()
		{
			ImGui::Begin("Control Panel");

			ImGui::Text("fps: %.1f", ImGui::GetIO().Framerate);
			enum Element_GraphicsAPI
			{
				Element_DirectX,
				Element_Vulkan,
				Element_COUNT
			};
			static int elem = Element_DirectX;
			const char *elems_names[Element_COUNT] = {"DirectX", "Vulkan"};
			const char *elem_name = (elem >= 0 && elem < Element_COUNT) ? elems_names[elem] : "Unknown";
			ImGui::SliderInt("Graphics", &elem, 0, Element_COUNT - 1, elem_name);
			ImGui::ColorEdit3("BG Color", Global::color);

			ImGui::SeparatorText("View");
			ImGui::DragFloat3("View Pos", Global::view_position, 0.1f, -10.0f, 10.0f);
			ImGui::DragFloat3("LookAt", Global::lookat, 0.1f, -10.0f, 10.0f);
			ImGui::DragFloat3("Up", Global::up, 0.01f, -1.0f, 1.0f);

			ImGui::SeparatorText("Projection");
			ImGui::DragFloat("FOV", &Global::FOV, 1.f, 0.0f, 360.0f);

			ImGuiIO &io = ImGui::GetIO();
			ImGui::Text("io.WantCaptureMouse: %d", io.WantCaptureMouse);
			ImGui::Text("io.WantCaptureMouseUnlessPopupClose: %d", io.WantCaptureMouseUnlessPopupClose);
			ImGui::Text("io.WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
			ImGui::Text("io.WantTextInput: %d", io.WantTextInput);
			ImGui::Text("io.WantSetMousePos: %d", io.WantSetMousePos);
			ImGui::Text("io.NavActive: %d, io.NavVisible: %d", io.NavActive, io.NavVisible);

			ImGui::End();
		}
	};
}