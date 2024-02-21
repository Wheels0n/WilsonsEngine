#include"Renderer11.h"
#include"D3D11.h"
#include"../ImGui/imgui_impl_win32.h"
#include"../ImGui/imgui_impl_dx11.h"

namespace wilson
{
	Renderer11::Renderer11(int m_screenWidth, int m_screenHeight, HWND hWnd)
	{
		m_pD3D11 = nullptr;
		m_pCam = nullptr;
		m_eAPI = DX11;

		m_pD3D11 = new D3D11(m_screenWidth, m_screenHeight, g_bVSYNC_ENABLE, hWnd, g_bFULL_SCREEN, g_fSCREEN_FAR, g_fSCREEN_NEAR);
		m_pCam = m_pD3D11->GetCam();
		
	}

	Renderer11::~Renderer11()
	{
		if (m_pD3D11 != nullptr)
		{
			delete m_pD3D11;
			m_pD3D11 = nullptr;
		}
	}

	void Renderer11::BeginFrame()
	{	
		m_pD3D11->UpdateScene();
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

	}

	void Renderer11::EndFrame()
	{
		m_pD3D11->DrawScene();
	}

	void Renderer11::UpdateResolution(int newWidth, int newHeight)
	{
		m_pD3D11->ResizeBackBuffer(newWidth, newHeight);
	}

	void Renderer11::Translate(XMVECTOR tr)
	{
		m_pCam->Translate(tr);
	}

	void Renderer11::Rotate(int dx, int dy)
	{
		m_pCam->Rotate(dx, dy);
	}


}