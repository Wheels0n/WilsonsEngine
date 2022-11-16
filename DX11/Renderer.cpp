#include"Renderer.h"

CRenderer::CRenderer()
{
	m_pD3D11 = nullptr;
}

CRenderer::~CRenderer()
{

}

bool CRenderer::Init(int iScreenHeight, int iScreenWidth, HWND hWnd)
{   
	bool bResult;

	m_pD3D11 = new CD3D11;
	if (m_pD3D11 == nullptr)
	{
		return false;
	}

	bResult = m_pD3D11->Init(iScreenWidth, iScreenHeight, g_bVSYNC_ENABLE, hWnd, g_bFull_SCREEN, g_fSCREEN_DEPTH, g_fSCREEN_NEAR);
	if (bResult == false)
	{
		MessageBox(hWnd, L"Could not initialize D3D", L"Error", MB_OK);
		return false;
	}

	return true;
}

void CRenderer::Shutdown()
{   


	if (m_pD3D11!=nullptr)
	{
		m_pD3D11->Shutdown();
		delete m_pD3D11;
		m_pD3D11 = nullptr;
	}

	return;
}

bool CRenderer::Frame()
{   
	bool bResult;

	bResult = Render();
	if (bResult == false)
	{
		return false;
	}
	return true;
}

bool CRenderer::Render()
{   
	//버퍼 내용지우기
	m_pD3D11->UpdateScene();
	//화면표시
	m_pD3D11->DrawScene();
	return true;
}