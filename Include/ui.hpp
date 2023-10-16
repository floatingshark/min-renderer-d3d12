#include "constant.hpp"
#include "global.hpp"
#include "object.hpp"
#include "texture.hpp"
#include <External/GLFW/glfw3.h>
#include <External/imgui/imgui.h>
#include <External/imgui/imgui_impl_dx12.h>
#include <External/imgui/imgui_impl_glfw.h>
#include <External/imgui/imgui_internal.h>
#include <cassert>
#include <d3d12.h>
#include <iostream>
#include <string>
#include <vector>

namespace albedo {
	class UI {
	protected:
		std::vector<std::shared_ptr<albedo::Object>> render_objects;
		std::shared_ptr<albedo::Object>				 skydome_object;

	public:
		void init(GLFWwindow* window, ID3D12Device* device, UINT num_frames, ID3D12DescriptorHeap* heap_srv) {
			init_imgui();
			init_imgui_glfw(window);
			init_imgui_directX(device, num_frames, heap_srv);
		}
		void update() {
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			create_dock();
			create_panel_1();
			create_panel_2();
		}
		void render() { ImGui::Render(); }
		void shutdown() {
			ImGui_ImplDX12_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

		void set_render_objects(std::vector<std::shared_ptr<albedo::Object>> in_objects) {
			render_objects = in_objects;
		}
		void set_skydome_object(std::shared_ptr<albedo::Object> in_object) { skydome_object = in_object; }

	protected:
		void init_imgui() {
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			ImGui::StyleColorsLight();
		}
		void init_imgui_glfw(GLFWwindow* window) { ImGui_ImplGlfw_InitForOther(window, true); }
		void init_imgui_directX(ID3D12Device* device, UINT num_frames, ID3D12DescriptorHeap* heap_srv) {
			ImGui_ImplDX12_Init(device, num_frames, DXGI_FORMAT_R8G8B8A8_UNORM, heap_srv,
								heap_srv->GetCPUDescriptorHandleForHeapStart(),
								heap_srv->GetGPUDescriptorHandleForHeapStart());
		}

		void create_dock() {
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoBackground;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_NoDocking;
			ImGui::Begin("Dock Base Window", nullptr, window_flags);

			ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
			dock_flags |= ImGuiDockNodeFlags_NoWindowMenuButton;
			ImGui::DockSpace(ImGui::GetID("Dock Base"), ImVec2(0, 0), dock_flags);

			ImGui::End();
			ImGui::PopStyleVar();
		}
		void create_panel_1() {
			ImGuiWindowFlags panel_1_flags = ImGuiWindowFlags_NoNav;
			ImGui::Begin("World", nullptr, panel_1_flags);

			ImGui::Text("fps: %.1f", ImGui::GetIO().Framerate);
			ImGui::ColorEdit3("BG", Global::bg_color);

			if (ImGui::TreeNode("View"))
			{
				ImGui::DragFloat3("VPos", Global::view_position, 0.1f, -30.0f, 30.0f, "%.2f");
				ImGui::DragFloat3("Look", Global::view_lookat, 0.1f, -10.0f, 10.0f, "%.2f");
				ImGui::DragFloat3("VUp", Global::view_up, 0.01f, -1.0f, 1.0f, "%.2f");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Projection")) {
				ImGui::DragFloat("FOV", &Global::projection_FOV, 1.f, 0.0f, 360.0f, "%.2f");
				ImGui::DragFloat("Near", &Global::projection_near, 0.1f, 0.0f, 100.0f, "%.2f");
				ImGui::DragFloat("Far", &Global::projection_far, 1.f, 0.0f, 2000.0f, "%.2f");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Light")) {
				ImGui::DragFloat3("LPos", Global::light_position, 0.1f, -30.0f, 30.0f, "%.2f");
				ImGui::ColorEdit4("LAmb", Global::light_ambient);
				ImGui::SliderFloat("LInt", &Global::light_intensity, 0.f, 5.f, "%.2f");
				ImGui::Checkbox("Shadow Mapping", &Global::is_enabled_shadow_mapping);
				ImGui::SliderFloat("Shadow Bias", &Global::shadow_mapping_bias, 0.f, 0.0002f, "%.6f");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Anti Aliasing")) {
				if (ImGui::Checkbox("MSAA(Forward)", &Global::is_enabled_msaa)) {
					for (std::shared_ptr<albedo::Object> object : render_objects) {
						object->reset_directx_render_pipeline_state();
					}
					skydome_object->reset_directx_render_pipeline_state();
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Postprocess")) {
				ImGui::Checkbox("Postprocessing", &albedo::Global::is_enabled_postprocess);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Skydome")) {
				ImGui::Checkbox("Display Skydome", &albedo::Global::is_enabled_skydome);
				ImGui::TreePop();
			}

			ImGui::End();
		}
		void create_panel_2() {
			ImGui::Begin("Object");

			static int		select_id		   = 0;
			ImGuiTableFlags object_table_flags = ImGuiTableFlags_Resizable;
			object_table_flags |= ImGuiTableFlags_Reorderable;
			object_table_flags |= ImGuiTableFlags_Hideable;
			object_table_flags |= ImGuiTableFlags_Sortable;
			object_table_flags |= ImGuiTableFlags_SortMulti;
			object_table_flags |= ImGuiTableFlags_RowBg;
			object_table_flags |= ImGuiTableFlags_Borders;
			object_table_flags |= ImGuiTableFlags_NoBordersInBody;
			object_table_flags |= ImGuiTableFlags_ScrollX;
			object_table_flags |= ImGuiTableFlags_ScrollY;
			object_table_flags |= ImGuiTableFlags_SizingFixedFit;
			if (ImGui::BeginTable("Object Table", 2, object_table_flags, ImVec2(0.0f, 10.f * 7.f), 0.0f)) {
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();
				for (int i = 0; i < static_cast<int>(render_objects.size()); i++) {
					char			id_label[32];
					albedo::Object* object = render_objects[i].get();
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					sprintf(id_label, "%d", i);
					bool is_selected = select_id == i;
					if (ImGui::Selectable(id_label, is_selected, ImGuiSelectableFlags_SpanAllColumns,
										  ImVec2(0.0f, 0.f))) {
						select_id = i;
					}
					ImGui::TableNextColumn();
					ImGui::Text("%s", object->name.c_str());
				}
				ImGui::EndTable();
			}

			albedo::Object* object = render_objects[select_id].get();
			ImGui::Text("> %s", object->name.c_str());

			ImGui::SeparatorText("Transform");
			ImGui::DragFloat3("Pos", (float*)&object->position, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Rot", (float*)&object->rotation, 0.1f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat3("Scl", (float*)&object->scale, 0.1f, -10.0f, 10.0f, "%.2f");

			ImGui::SeparatorText("Shader");
			const char* shader_items[]		= {"Color", "Phong", "Skydome"};
			int			shader_item_current = (int)object->shader_type;
			if (ImGui::Combo("Shaders", &shader_item_current, shader_items, IM_ARRAYSIZE(shader_items))) {
				const albedo::Shaders::Type shader_type = (albedo::Shaders::Type)shader_item_current;
				object->reset_directx_shader(shader_type);
			}

			ImGui::SeparatorText("Material");
			const char* tex_items[]		 = {"Mono", "Checker Board", "Image"};
			int			tex_item_current = (int)object->texture_type;
			if (ImGui::Combo("Tex", &tex_item_current, tex_items, IM_ARRAYSIZE(tex_items))) {
				const Texture::Type texture_type = (Texture::Type)tex_item_current;
				object->set_texture_data(texture_type);
			}
			if (ImGui::ColorEdit3("TCol", object->texture_color)) {
				const Texture::Type texture_type = (Texture::Type)tex_item_current;
				object->set_texture_data(texture_type);
			}

			ImGui::SliderFloat("Spec", &object->specular_power, 0.1f, 1000.f, "%.2f");

			ImGui::End();
		}
	};
} // namespace albedo