#include"Renderer.h"
#include"../ImGui/imgui.h"
#include"../ImGui/imgui_impl_win32.h"
#include"../ImGui/imgui_impl_dx11.h"

namespace wilson
{
	Renderer::Renderer()
	{
		m_pD3D11 = nullptr;
		m_pCam = nullptr;
	}

	Renderer::~Renderer()
	{

	}

	bool Renderer::Init(int m_screenWidth, int m_screenHeight, HWND hWnd)
	{
		bool bResult;

		m_pD3D11 = new D3D11;
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

	void Renderer::Shutdown()
	{


		if (m_pD3D11 != nullptr)
		{
			m_pD3D11->Shutdown();
			delete m_pD3D11;
			m_pD3D11 = nullptr;
		}

		return;
	}

	void Renderer::BeginFrame()
	{
		m_pD3D11->UpdateScene();
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

	}

	void Renderer::EndFrame()
	{
		m_pD3D11->DrawScene();
	}

	void Renderer::Translate(XMVECTOR tr)
	{
		m_pCam->Translate(tr);
	}

	void Renderer::Rotate(int dx, int dy)
	{
		m_pCam->Rotate(dx, dy);
	}

	void Renderer::Zoom(int dz)
	{
		m_pCam->Zoom(dz);
	}

}