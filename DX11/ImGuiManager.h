#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include"../ImGui/imgui.h"
#include"../ImGui/imgui_impl_win32.h"
#include"../ImGui/imgui_impl_dx11.h"

namespace wilson {
	class ImGuiManager
	{
	public:
		void Update();
		ImGuiManager();
		~ImGuiManager();
	private:
		void SetMenuBar();
		void SetDockspace();
	};
}
#endif
