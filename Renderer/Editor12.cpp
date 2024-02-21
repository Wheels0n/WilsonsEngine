#include "Editor12.h"
#include "D3D12.h"
#include "Scene12.h"
#include "ContentBrowser12.h"
#include "Viewport12.h"
#include "Settings12.h"
namespace wilson {
	Editor12::Editor12(D3D12* pD3D12)
	{
		m_pD3D12 = pD3D12;
		m_pContentBrowser12 = nullptr;
		m_pViewport12 = nullptr;
		m_pSettings12 = nullptr;

		m_pContentBrowser12 = new ContentBrowser12(m_pD3D12);
		m_pScene12 = new Scene12(m_pD3D12);
		m_pViewport12 = new Viewport12(m_pD3D12, m_pScene12->GetScene());
		m_pSettings12 = new Settings12(m_pD3D12);
		std::string str = "Scene";
		m_pScene12->SetCam(m_pD3D12->GetCam());
		m_pScene12->SetSceneName(str);
	}
	Editor12::~Editor12()
	{

		if (m_pContentBrowser12 != nullptr)
		{
			delete m_pContentBrowser12;
			m_pContentBrowser12 = nullptr;
		}

		if (m_pViewport12 != nullptr)
		{
			delete m_pViewport12;
			m_pViewport12 = nullptr;
		}


		if (m_pScene12 != nullptr)
		{
			delete m_pScene12;
			m_pScene12 = nullptr;
		}

		if (m_pSettings12 != nullptr)
		{
			delete m_pSettings12;
			m_pSettings12 = nullptr;
		}
	}
	void Editor12::Draw()
	{
		m_pContentBrowser12->List();
		m_pViewport12->Draw();
		m_pScene12->Draw();
		m_pSettings12->Draw();

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

		float ndcX = m_pViewport12->GetNDCX(x);
		float ndcY = m_pViewport12->GetNDCY(y);

		float mappedX = ndcX * width;
		float mappedY = ndcY * height;

		m_pScene12->Pick(mappedX, mappedY, width, height);
	}
	bool Editor12::CheckRange(int x, int y)
	{
		return m_pViewport12->CheckRange(x, y);
	}

}