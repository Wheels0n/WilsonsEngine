#include "Editor.h"

namespace wilson {
	void Editor::Init(D3D11* pCD3D11)
	{
		m_pD3D11 = pCD3D11;

		m_viewport.Init(m_pD3D11, m_scene.GetScene());
		m_contentBrowser.Init(m_pD3D11->GetDevice());
		m_settings.Init(m_pD3D11->GetDevice(),m_pD3D11->GetCam());
		m_scene.Init(pCD3D11);

		std::string str = "Scene";
		m_scene.SetCam(m_pD3D11->GetCam());
		m_scene.SetSceneName(str);
	}
	void Editor::Draw()
	{
		m_contentBrowser.List();
		m_viewport.Draw();
		m_scene.Draw();
		m_settings.Draw();

	}
	void Editor::Pick()
	{
		ImGuiIO io = ImGui::GetIO();
		int x = io.MousePos.x;
		int y = io.MousePos.y;
		if (!CheckRange(x, y))
		{
			return;
		}

		int width = m_pD3D11->GetClientWidth();
		int height = m_pD3D11->GetClientHeight();

		float ndcX = m_viewport.GetNDCX(x);
		float ndcY = m_viewport.GetNDCY(y);

		float mappedX = ndcX * width;
		float mappedY = ndcY * height;

		m_scene.Pick(mappedX, mappedY, width, height);
	}
	bool Editor::CheckRange(int x, int y)
	{
		return m_viewport.CheckRange(x, y);
	}

}