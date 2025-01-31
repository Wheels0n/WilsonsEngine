#include"Renderer12.h"
#include"D3D12.h"
#include"../ImGui/imgui_impl_win32.h"
#include"../ImGui/imgui_impl_dx12.h"

namespace wilson
{
#ifdef _DEBUG

#include <dxgidebug.h>
	DEFINE_GUID(DXGI_DEBUG_D3D12, 0x4b99317b, 0xac39, 0x4aa6, 0xbb, 0xb, 0xba, 0xa0, 0x47, 0x84, 0x79, 0x8f);

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxguid.lib")
#endif
	Renderer12::Renderer12(const UINT m_screenWidth, const UINT m_screenHeight, HWND hWnd)
	{
		m_pD3D12 = std::make_shared<D3D12>
			(m_screenWidth, m_screenHeight, g_bVSYNC_ENABLE, hWnd, g_bFULL_SCREEN, g_fSCREEN_FAR, g_fSCREEN_NEAR);
		m_pD3D12->BeginWorkers();
		m_pCam = m_pD3D12->GetCam();
		
	}

	Renderer12::~Renderer12()
	{

		if (m_pD3D12 != nullptr)
		{
			m_pD3D12->EndWorkers();
			m_pD3D12 = nullptr;
#ifdef _DEBUG
			{
				HMODULE dxgidebugDLL = GetModuleHandleW(L"dxgidebug.dll");
				decltype(&DXGIGetDebugInterface) GetDebugInterface =
					reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugDLL, "DXGIGetDebugInterface"));

				IDXGIDebug* pDebug;
				GetDebugInterface(IID_PPV_ARGS(&pDebug));

				OutputDebugStringW(L"!!!WARNGING!!! \r\n");
				pDebug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
				OutputDebugStringW(L"!!!RESOURCE NOT RELEASED!!!\r\n");

				pDebug->Release();
			}
#endif
		}
	}

	BOOL Renderer12::CheckDx12Support()
	{
		IDXGIFactory1* pFactory;
		if (CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)) != S_OK)
		{
			OutputDebugStringW(L"Failed To Creaete DXGIFactory");
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

	shared_ptr<D3D12> Renderer12::GetD3D12()
	{
		return m_pD3D12;
	}

	void Renderer12::UpdateResolution(const UINT newWidth, const UINT newHeight)
	{
		m_pD3D12->ResizeBackBuffer(newWidth, newHeight);
	}

	void Renderer12::Translate(const XMVECTOR tr)
	{
		m_pCam->Translate(tr);
		m_pCam->UpdateMatrices();
	}

	void Renderer12::Rotate(int dx, int dy)
	{
		m_pCam->Rotate(dx, dy);
		m_pCam->UpdateMatrices();
	}

}