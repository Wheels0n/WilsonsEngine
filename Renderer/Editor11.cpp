#include "Editor11.h"
#include"D3D11.h"
#include"Scene11.h"
#include"ContentBrowser11.h"
#include"Viewport11.h"
#include"Settings11.h"
namespace wilson {
	Editor11::Editor11(D3D11*const pD3D11)
	{
		m_pD3D11 = pD3D11;
		m_pContentBrowser = nullptr;
		m_pViewport = nullptr;
		m_pScene = nullptr;
		m_pSettings = nullptr;

		m_pContentBrowser = new ContentBrowser11(m_pD3D11->GetDevice());
		m_pScene = new Scene11(pD3D11);
		m_pViewport = new Viewport11(pD3D11, m_pScene->GetScene());
		m_pSettings = new Settings11(m_pD3D11);
		std::string str = "Scene";
		m_pScene->SetCam(m_pD3D11->GetCam());
		m_pScene->SetSceneName(str);
	}
	
	Editor11::~Editor11()
	{
	}
	void Editor11::Draw()
	{
		m_pContentBrowser->List();
		m_pViewport->Draw();
		m_pScene->Draw();
		m_pSettings->Draw();

	}
	void Editor11::Pick()
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
	bool Editor11::CheckRange(int x, int y)
	{
		return m_pViewport->CheckRange(x, y);
	}

}