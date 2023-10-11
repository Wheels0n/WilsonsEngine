#include "Editor.h"

namespace wilson {
	Editor::Editor(D3D11* pD3D11)
	{
		m_pD3D11 = pD3D11;
		m_pContentBrowser = nullptr;
		m_pViewport = nullptr;
		m_pScene = nullptr;
		m_pSettings = nullptr;

		m_pContentBrowser = new ContentBrowser(m_pD3D11->GetDevice());
		m_pScene = new Scene(pD3D11);
		m_pViewport = new Viewport(pD3D11, m_pScene->GetScene());
		m_pSettings = new Settings(m_pD3D11);
		std::string str = "Scene";
		m_pScene->SetCam(m_pD3D11->GetCam());
		m_pScene->SetSceneName(str);
	}
	Editor::~Editor()
	{
		if (m_pContentBrowser != nullptr)
		{
			delete m_pContentBrowser;
			m_pContentBrowser = nullptr;
		}

		if (m_pViewport != nullptr)
		{
			delete m_pViewport;
			m_pViewport = nullptr;
		}

		if (m_pScene != nullptr)
		{
			delete m_pScene;
			m_pScene = nullptr;
		}
		if (m_pSettings != nullptr)
		{	
			delete m_pSettings;
			m_pSettings = nullptr;
		}
	}
	void Editor::Draw()
	{
		m_pContentBrowser->List();
		m_pViewport->Draw();
		m_pScene->Draw();
		m_pSettings->Draw();

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

		UINT width = m_pD3D11->GetClientWidth();
		UINT height = m_pD3D11->GetClientHeight();

		float ndcX = m_pViewport->GetNDCX(x);
		float ndcY = m_pViewport->GetNDCY(y);

		float mappedX = ndcX * width;
		float mappedY = ndcY * height;

		m_pScene->Pick(mappedX, mappedY, width, height);
	}
	bool Editor::CheckRange(int x, int y)
	{
		return m_pViewport->CheckRange(x, y);
	}

}