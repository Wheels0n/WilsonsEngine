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
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;  //≈ı∏Ìµµ
		}


	}

	ImGuiManager::~ImGuiManager()
	{
		ImGui::DestroyContext();
	}

	void ImGuiManager::Update()
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
}
