#include"Renderer.h"
#include"../ImGui/imgui.h"
#include"../ImGui/imgui_impl_win32.h"
#include"../ImGui/imgui_impl_dx11.h"

CRenderer::CRenderer()
{
	m_pD3D11 = nullptr;
	m_pCam = nullptr;
}

CRenderer::~CRenderer()
{

}

bool CRenderer::Init(int m_screenWidth, int m_screenHeight, HWND hWnd)
{   
	bool bResult;

	m_pD3D11 = new CD3D11;
	if (m_pD3D11 == nullptr)
	{
		return false;
	}

	bResult = m_pD3D11->Init(m_screenWidth, m_screenHeight, g_bVSYNC_ENABLE, hWnd, g_bFull_SCREEN, g_fSCREEN_FAR, g_fSCREEN_NEAR);
	if (bResult == false)
	{
		MessageBox(hWnd, L"Could not initialize D3D", L"Error", MB_OK);
		return false;
	}

	m_pCam = m_pD3D11->GetCam();
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

void CRenderer::BeginFrame()
{   
	m_pD3D11->UpdateScene();
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

void CRenderer::EndFrame()
{
	m_pD3D11->DrawScene();
}

void CRenderer::TranslateRight()
{
	m_pCam->translateRight();
}

void CRenderer::TranslateLeft()
{
	m_pCam->translateLeft();
}

void CRenderer::TranslateDownward()
{
	m_pCam->translateDownWard();
}

void CRenderer::TranslateUpward()
{
	m_pCam->translateUpward();
}

void CRenderer::RotateLeft()
{
	m_pCam->RotateLeft();
}

void CRenderer::RotateRight()
{
	m_pCam->RotateRight();
}

void CRenderer::ZoomIn()
{
	m_pCam->ZoomIn();
}

void CRenderer::ZoomOut()
{
	m_pCam->ZoomOut();
}
