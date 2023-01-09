#include "Editor.h"

void CEditor::Init(CD3D11* pCD3D11)
{
	m_CFps.Init();
	m_CViewport.Init(pCD3D11);
	m_CContentBrowser.Init(pCD3D11->GetDevice());
}

void CEditor::Draw()
{  
	m_CContentBrowser.List();
	m_CFps.Frame();
	m_CViewport.Draw();
}
