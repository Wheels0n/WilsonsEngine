#include "ImGuiManager.h"
#include "../ImGui/imgui.h"

CImGuiManager::CImGuiManager()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = true;
}

CImGuiManager::~CImGuiManager()
{
	ImGui::DestroyContext();
}

void CImGuiManager::Update()
{   
	SetMenuBar();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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


