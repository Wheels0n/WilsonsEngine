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
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	SetMenuBar();
	bool b = true;
	ImGui::ShowDemoWindow(&b);
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


