#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <d3d12.h>
#include <GLFW/glfw3.h>
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
		}
		void init_imgui_directX(ID3D12Device *device, UINT num_frames, ID3D12DescriptorHeap *heap_srv)
		{
			ImGui_ImplDX12_Init(device, num_frames,
								DXGI_FORMAT_R8G8B8A8_UNORM, heap_srv,
								heap_srv->GetCPUDescriptorHandleForHeapStart(),
								heap_srv->GetGPUDescriptorHandleForHeapStart());
		}
		void update(std::vector<arabesques::Object> &objects)
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			window_1();
			window_2(objects);
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
		void window_1()
		{
			ImGui::Begin("Control Panel");

			ImGui::Text("fps: %.1f", ImGui::GetIO().Framerate);
			enum Element_GraphicsAPI
			{
				Element_DirectX,
				Element_Vulkan,
				Element_COUNT
			};
			static int elem_graphics_int = Element_DirectX;
			const char *elems_graphics_names[Element_COUNT] = {"DirectX", "Vulkan"};
			const char *elem_graphics_name = (elem_graphics_int >= 0 && elem_graphics_int < Element_COUNT) ? elems_graphics_names[elem_graphics_int] : "Unknown";
			ImGui::SliderInt("Graphics", &elem_graphics_int, 0, Element_COUNT - 1, elem_graphics_name);
			ImGui::ColorEdit3("BG Color", Global::color);

			ImGui::SeparatorText("View");
			ImGui::DragFloat3("CPos", Global::view_position, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Look", Global::lookat, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Up", Global::up, 0.01f, -1.0f, 1.0f, "%.2f");

			ImGui::SeparatorText("Projection");
			ImGui::DragFloat("FOV", &Global::FOV, 1.f, 0.0f, 360.0f, "%.2f");

			ImGui::SeparatorText("Light");
			ImGui::DragFloat3("LPos", Global::light_position, 0.1f, -10.0f, 10.0f, "%.2f");

			ImGui::End();
		}
		void window_2(std::vector<arabesques::Object> &objects)
		{
			ImGui::Begin("Object Panel");

			// Objects Table
			static int select_id = 0;
			static ImGuiTableFlags object_table_flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
			if (ImGui::BeginTable("Object Table", 2, object_table_flags, ImVec2(0.0f, 10.f * 7.f), 0.0f))
			{
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();

				for (int i = 0; i < objects.size(); i++)
				{
					char id_label[32];
					arabesques::Object object = objects[i];
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					sprintf(id_label, "%d", i);
					if (ImGui::Selectable(id_label, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.0f, 0.f)))
					{
						select_id = i;
					}
					ImGui::TableNextColumn();
					ImGui::Text("%s", object.get_name().c_str());
				}
				ImGui::EndTable();
			}

			arabesques::Object &object = objects[select_id];
			enum Element_Object_Enabled
			{
				Disabled,
				Enabled,
				Element_COUNT
			};
			const char *elems_object_enable_names[Element_COUNT] = {"Disabled", "Enabled"};
			const char *elem_object_enable_name = elems_object_enable_names[object.enabled];
			ImGui::SliderInt("Disp", (int*)&object.enabled, 0, Element_COUNT - 1, elem_object_enable_name);
			ImGui::SeparatorText("World");
			ImGui::DragFloat3("Pos", (float *)&object.position, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Rot", (float *)&object.rotation, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Scl", (float *)&object.scale, 0.1f, -10.0f, 10.0f, "%.2f");

			ImGui::End();
		}
	};
}