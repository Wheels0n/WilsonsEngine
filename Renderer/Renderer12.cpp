#include"Renderer12.h"
#include"D3D12.h"
#include"../ImGui/imgui_impl_win32.h"
#include"../ImGui/imgui_impl_dx12.h"

namespace wilson
{

	Renderer12::Renderer12(int m_screenWidth, int m_screenHeight, HWND hWnd)
	{
		m_pD3D12 = nullptr;
		m_pCam = nullptr;
		
		m_eAPI = DX12;
		m_pD3D12 = new D3D12(m_screenWidth, m_screenHeight, g_bVSYNC_ENABLE, hWnd, g_bFULL_SCREEN, g_fSCREEN_FAR, g_fSCREEN_NEAR);
		m_pCam = m_pD3D12->GetCam();
		
	}

	Renderer12::~Renderer12()
	{

		if (m_pD3D12 != nullptr)
		{
			delete m_pD3D12;
			m_pD3D12 = nullptr;
		}
	}

	bool Renderer12::CheckDx12Support()
	{
		IDXGIFactory1* pFactory;
		if (CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)) != S_OK)
		{
			OutputDebugStringW(L"Renderer11::pFactory::Failed To Creaete DXGIFactory");
			throw std::exception();
		}
		else
		{
			IDXGIAdapter1* pAdapter;
			ID3D12Device6* pD3D12Deivce;
			HRESULT hr;

			hr = pFactory->EnumAdapters1(0, &pAdapter);
			assert(SUCCEEDED(hr));

			hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);

			pFactory->Release();
			assert(SUCCEEDED(hr));
			
				return true;
			
		}

	}

	void Renderer12::BeginFrame()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		m_pD3D12->UpdateScene();
	}

	void Renderer12::EndFrame()
	{
		m_pD3D12->DrawScene();
	}

	void Renderer12::UpdateResolution(int newWidth, int newHeight)
	{
		m_pD3D12->ResizeBackBuffer(newWidth, newHeight);
	}

	void Renderer12::Translate(XMVECTOR tr)
	{
		m_pCam->Translate(tr);
	}

	void Renderer12::Rotate(int dx, int dy)
	{
		m_pCam->Rotate(dx, dy);
	}


}