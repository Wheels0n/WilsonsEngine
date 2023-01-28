#include "Editor.h"

void CEditor::Init(CD3D11* pCD3D11)
{   
	m_pCD3D11 = pCD3D11;

	m_CViewport.Init(m_pCD3D11, m_Scene.GetScene());
	m_CContentBrowser.Init(m_pCD3D11->GetDevice());
	m_CSettings.Init(m_pCD3D11->GetCam(), m_pCD3D11->GetLight());
	m_Scene.Init(pCD3D11);

	std::string str = "Scene";
	m_Scene.SetCam(m_pCD3D11->GetCam());
	m_Scene.SetSceneName(str);
}

void CEditor::Draw()
{  
	m_CContentBrowser.List();
	m_CViewport.Draw();
	m_Scene.Draw();
	m_CSettings.Draw();
	
}

void CEditor::Pick()
{   
	ImGuiIO io = ImGui::GetIO();
	int x = io.MousePos.x;
	int y = io.MousePos.y;
	if (!CheckRange(x, y))
	{
		return;
	}

	int width = m_pCD3D11->GetClientWidth();
	int height = m_pCD3D11->GetClientHeight();

	float ndcX = m_CViewport.GetNDCX(x);
	float ndcY = m_CViewport.GetNDCY(y);

	int mappedX = ndcX *width;
	int mappedY = ndcY *height;

	m_Scene.Pick(mappedX, mappedY, width, height);
}

bool CEditor::CheckRange(int x, int y)
{
	return m_CViewport.CheckRange(x, y);
}
