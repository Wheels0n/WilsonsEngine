#include "ImGuiManager.h"
#include "../ImGui/imgui.h"

CImGuiManager::CImGuiManager()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsLight();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
}

CImGuiManager::~CImGuiManager()
{
	ImGui::DestroyContext();
}


