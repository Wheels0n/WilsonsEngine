#include "ImGuiManager.h"

#include"../ImGui/imgui_impl_win32.h"
#include"../ImGui/imgui.h"

namespace wilson {
	ImGuiManager::ImGuiManager()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;  //투명도
		}


	}

	ImGuiManager::~ImGuiManager()
	{
		ImGui::DestroyContext();
	}

	void ImGuiManager::UpdateMatrices()
	{
		SetMenuBar();
		SetDockspace();
	}

	void ImGuiManager::SetMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

	}

	void ImGuiManager::SetDockspace()
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	}

	BOOL ImGuiManager::DrawFloat4(XMFLOAT4& val, const char* pText, const float speed)
	{
		ImGui::PushID(pText);
		ImGui::Text(pText);
		bool bValueChanged = false;
		if (ImGui::Button("X"))
		{
			val.x = 0.0f;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &val.x, speed, -10000.0f, 10000.0f))
		{
			bValueChanged = true;
		}

		if (ImGui::Button("Y"))
		{
			val.y = 0.0f;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &val.y, speed, -10000.0f, 10000.0f))
		{
			bValueChanged = true;
		}

		if (ImGui::Button("Z"))
		{
			val.z = 0.0f;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &val.z, speed, -10000.0f, 10000.0f))
		{
			bValueChanged = true;
		}

		if (ImGui::Button("W"))
		{
			val.w = 0.0f;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##W", &val.w, speed, -10000.0f, 10000.0f))
		{
			bValueChanged = true;
		}
		ImGui::PopID();
		return bValueChanged;
	}
	BOOL ImGuiManager::DrawFloat3(XMFLOAT3& val, const char* pText, const float speed)
	{
		ImGui::PushID(pText);
		ImGui::Text(pText);
		bool bValueChanged = false;
		if (ImGui::Button("X"))
		{
			val.x = 0.0f;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &val.x, speed, -10000.0f, 10000.0f))
		{
			bValueChanged = true;
		}

		if (ImGui::Button("Y"))
		{
			val.y = 0.0f;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &val.y, speed, -10000.0f, 10000.0f))
		{
			bValueChanged = true;
		}

		if (ImGui::Button("Z"))
		{
			val.z = 0.0f;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &val.z, speed, -10000.0f, 10000.0f))
		{
			bValueChanged = true;
		}
		ImGui::PopID();
		return bValueChanged;
	}

	BOOL ImGuiManager::DrawVec3Control(const std::string& label, float* vals)
	{
		bool bValChanged = false;
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 70.f);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		float itemWidth = ImGui::CalcItemWidth() / 3.0f;

		ImGui::PushItemWidth(itemWidth);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiCol_Button);
		if (ImGui::Button("X"))
		{
			vals[0] = 0.0f;
			bValChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &vals[0], 0.1f))
		{
			bValChanged = true;
		}//##으로 중복되는 label처리, 출력은 안됨
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();

		ImGui::PushItemWidth(itemWidth);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiCol_Button);
		if (ImGui::Button("Y"))
		{
			vals[1] = 0.0f;
			bValChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &vals[1], 0.1f))
		{
			bValChanged = true;
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();

		ImGui::PushItemWidth(itemWidth);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGuiCol_Button);
		if (ImGui::Button("Z"))
		{
			vals[2] = 0.0f;
			bValChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &vals[2], 0.1f))
		{
			bValChanged = true;
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);

		ImGui::PopStyleVar();
		ImGui::Columns(1);

		ImGui::PopID();

		return bValChanged;
	}
}
