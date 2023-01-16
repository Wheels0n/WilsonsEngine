#include "Editor.h"

void CEditor::Init(CD3D11* pCD3D11)
{   
	m_pCD3D11 = pCD3D11;

	m_CFps.Init();
	m_CViewport.Init(m_pCD3D11, m_Scene.GetScene());
	m_CContentBrowser.Init(m_pCD3D11->GetDevice());

	std::string str = "Scene";
	m_Scene.SetCam(m_pCD3D11->GetCam());
	m_Scene.SetSceneName(str);
}

void CEditor::Draw()
{  
	m_CContentBrowser.List();
	m_CViewport.Draw();
	m_Scene.Draw();
	m_CFps.Frame();
	
}

void CEditor::Pick(int sx, int sy)
{   
	int width = m_pCD3D11->GetClientWidth();
	int height = m_pCD3D11->GetClientHeight();
	m_Scene.Pick(sx, sy, width, height);
}
