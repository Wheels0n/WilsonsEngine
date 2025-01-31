#pragma once
#include "Editor12.h"
#include "D3D12.h"
#include "Scene12.h"
#include "ContentBrowser12.h"
#include "Viewport12.h"
#include "Settings12.h"
#include "Engine.h"

namespace wilson {
	using namespace std;

	Editor12::Editor12(shared_ptr<D3D12> pD3D12)
	{
		m_pContentBrowser = make_shared<ContentBrowser12>(pD3D12);
		m_pScene = make_shared<Scene12>(pD3D12);
		m_pViewport = make_shared<Viewport12>(pD3D12, m_pScene);
		m_pSettings = make_shared<Settings12>(pD3D12);
	
		m_pScene->SetCam(pD3D12->GetCam());
		pD3D12->SetScene(m_pScene);
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

		UINT width = io.DisplaySize.x;
		UINT height = io.DisplaySize.y;

		float ndcX = m_pViewport->GetNDCX(x);
		float ndcY = m_pViewport->GetNDCY(y);

		float mappedX = ndcX * width;
		float mappedY = ndcY * height;

		m_pScene->Pick(mappedX, mappedY, width, height);
	}
	BOOL Editor12::CheckRange(int x, int y)
	{
		return m_pViewport->CheckRange(x, y);
	}

}