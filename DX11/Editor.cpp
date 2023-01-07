#include "Editor.h"

void CEditor::Init(ID3D11ShaderResourceView* pSRV)
{
	m_CFps.Init();
	m_CViewport.Init(pSRV);
}

void CEditor::Draw()
{  
	m_CContentBrowser.List();
	m_CFps.Frame();
	m_CViewport.Draw();
}
