#pragma once

namespace wilson {
	class ImGuiManager
	{
	public:
		void Update();

		ImGuiManager();
		~ImGuiManager();
	private:
		void SetDockspace();
		void SetMenuBar();
	};
}
