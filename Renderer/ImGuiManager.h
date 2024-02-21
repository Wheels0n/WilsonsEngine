#pragma once

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
