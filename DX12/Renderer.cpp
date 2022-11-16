#include "Renderer.h"

CRenderer::CRenderer()
{
	m_pD3D = nullptr;
}

CRenderer::CRenderer(const CRenderer&)
{
}

CRenderer::~CRenderer()
{
}

bool CRenderer::Initialize(int iScreenHeight, int iScreenWidth, HWND hWnd)
{
	bool bResult;

	m_pD3D = new CD3D;
	if (m_pD3D == nullptr)
	{
		return false;
	}

	bResult = m_pD3D->Initialize(iScreenHeight, iScreenWidth,
		hWnd, g_bVSYNC_ENABLED, g_bFULL_SCREEN);//, g_fSCREEN_DEPTH, g_fSCREEN_NEAR);

	if (bResult == false)
	{
		MessageBox(hWnd, L"Could not intialize Direct3D", L"Error", MB_OK);
		return false;
	}

	return true;
}

void CRenderer::Shutdown()
{
	if (m_pD3D == nullptr)
	{
		m_pD3D->Shutdown();
		delete m_pD3D;
		m_pD3D = nullptr;
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
	bool bResult;

	bResult = m_pD3D->Render();
	if (bResult == false)
	{
		return false;
	}

	return true;
}
