#pragma once
#include "Editor12.h"
#include "D3D12.h"
#include "Scene12.h"
#include "ContentBrowser12.h"
#include "Viewport12.h"
#include "Settings12.h"
namespace wilson {
	Editor12::Editor12(D3D12*const pD3D12)
	{
		m_pD3D12 = pD3D12;
		m_pContentBrowser = nullptr;
		m_pViewport = nullptr;
		m_pSettings = nullptr;

		m_pContentBrowser = new ContentBrowser12(m_pD3D12);
		m_pScene = new Scene12(m_pD3D12);
		m_pViewport = new Viewport12(m_pD3D12, m_pScene->GetScene());
		m_pSettings = new Settings12(m_pD3D12);
		std::string str = "Scene11";
		m_pScene->SetCam(m_pD3D12->GetCam());
		m_pScene->SetSceneName(str);
	}
	Editor12::~Editor12()
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
	void Editor12::Draw()
	{
		m_pContentBrowser->List();
		m_pViewport->Draw();
		m_pScene->Draw();
		m_pSettings->Draw();

	}
	void Editor12::Pick()
	{
		ImGuiIO io = ImGui::GetIO();
		int x = io.MousePos.x;
		int y = io.MousePos.y;
		if (!CheckRange(x, y))
		{
			return;
		}

		UINT width = m_pD3D12->GetClientWidth();
		UINT height = m_pD3D12->GetClientHeight();

		float ndcX = m_pViewport->GetNDCX(x);
		float ndcY = m_pViewport->GetNDCY(y);

		float mappedX = ndcX * width;
		float mappedY = ndcY * height;

		m_pScene->Pick(mappedX, mappedY, width, height);
	}
	bool Editor12::CheckRange(int x, int y)
	{
		return m_pViewport->CheckRange(x, y);
	}

}