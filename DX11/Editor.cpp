#include "Editor.h"

void CEditor::Init(CD3D11* pCD3D11)
{
	m_CFps.Init();
	m_CViewport.Init(pCD3D11, m_Scene.GetScene());
	m_CContentBrowser.Init(pCD3D11->GetDevice());

	std::string str = "Scene";
	m_Scene.SetSceneName(str);
}

void CEditor::Draw()
{  
	m_CContentBrowser.List();
	m_CViewport.Draw();
	m_Scene.Draw();
	m_CFps.Frame();
	
}
