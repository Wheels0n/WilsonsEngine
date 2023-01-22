#include "ImGuiManager.h"
#include "../ImGui/imgui.h"

CImGuiManager::CImGuiManager()
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

CImGuiManager::~CImGuiManager()
{
	ImGui::DestroyContext();
}

void CImGuiManager::Update()
{   
	SetMenuBar();
	SetDockspace();
}

void CImGuiManager::SetMenuBar()
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

void CImGuiManager::SetDockspace()
{
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}


