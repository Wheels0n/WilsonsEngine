
#include"Engine.h"
#include"Renderer11.h"
#include"Renderer12.h"
#include"Editor11.h"
#include"Editor12.h"
namespace wilson {
	Engine::Engine()
	{
		m_pRenderer = nullptr;
		m_pEditor = nullptr;
	
		m_screenHeight = 0;
		m_screenWidth = 0;
		m_lastMouseX = 0;
		m_lastMouseY = 0;
		m_curMouseX = 0;
		m_curMouseY = 0;

		bool bResult;

		InitWindows(m_screenHeight, m_screenWidth);
		
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		if (Renderer12::CheckDx12Support())
		{
			m_pRenderer = new Renderer12(rc.right - rc.left, rc.bottom - rc.top, m_hWnd);
			m_pEditor = new Editor12(((Renderer12*)m_pRenderer)->GetD3D12());
		}
		else
		{
			m_pRenderer = new Renderer11(rc.right - rc.left, rc.bottom - rc.top, m_hWnd);
			m_pEditor = new Editor11(((Renderer11*)m_pRenderer)->GetD3D11());
		
		}
		
		
	}

	Engine::~Engine()
	{
	

		if (m_pEditor != nullptr)
		{
			delete m_pEditor;
			m_pEditor = nullptr;
		}

		if (m_pRenderer != nullptr)
		{
			delete m_pRenderer;
			m_pRenderer = nullptr;
		}


		if (g_bFULL_SCREEN)
		{
			ChangeDisplaySettings(nullptr, 0);
		}

		ImGui_ImplWin32_Shutdown();
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;

		UnregisterClassW(m_appName, m_hInstance);
		m_hInstance = nullptr;

		g_pEngineHandle = nullptr;


	}

	void Engine::Run()
	{
		MSG msg;
		bool bExit, bResult;

		ZeroMemory(&msg, sizeof(MSG));
		m_timer.Reset();

		bExit = false;
		while (bExit != true)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT)
			{
				bExit = true;
			}
			else
			{
				bResult = Frame();
				if (bResult == false)
				{
					bExit = true;
				}
			}
		}
		return;
	}

	LRESULT Engine::MsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		{
			return true;
		}

		if (uMsg == WM_LBUTTONDOWN && m_pEditor != nullptr)
		{
			//현재 창 기준 상대적 좌표
			int x = (short)(lParam) & 0xffff;
			int y = (lParam >> 16) & 0xffff;

			m_lastMouseX = x;
			m_lastMouseY = y;
			m_curMouseX = x;
			m_curMouseY = y;

			m_pEditor->Pick();

		}
		switch (uMsg)
		{
			case WM_SIZE:
			{	
				if (m_pRenderer)
				{
					m_screenWidth = LOWORD(lParam);
					m_screenHeight = HIWORD(lParam);
					m_pRenderer->UpdateResolution(m_screenWidth, m_screenHeight);
	
				}
				break;
			}
			case WM_MOUSEMOVE:
			{
				if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
				{

					RECT rc;
					GetWindowRect(m_hWnd, &rc);
					int winPosX = rc.left;
					int winPosY = rc.top;

					int x = (short)(lParam) & 0xffff;
					int y = (lParam >> 16) & 0xffff;

					m_curMouseX = x;
					m_curMouseY = y;

					int dyaw = m_lastMouseX - m_curMouseX;
					int dpitch = m_lastMouseY - m_curMouseY;

					if (abs(dyaw) > _DRAG_THRESHOLD)
					{
						dyaw = dyaw > 0 ? 1 : -1;
					}
					else
					{
						dyaw = 0;
					}

					if (abs(dpitch) > _DRAG_THRESHOLD)
					{
						dpitch = dpitch > 0 ? 1 : -1;
					}
					else
					{
						dpitch = 0;
					}

					if (m_pEditor->CheckRange(winPosX + m_lastMouseX, winPosY + m_lastMouseY))
					{
						m_pRenderer->Rotate(dpitch, dyaw);
						m_lastMouseX = x;
						m_lastMouseY = y;
					}
				}
				break;
			}

			case WM_CHAR:
			{
				switch (wParam)
				{
				case 'w':
				{
					if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
					{
						m_pRenderer->Translate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
					}
					break;
				}
				case 's':
				{
					if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
					{
						m_pRenderer->Translate(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
					}
					break;
				}
				case 'a':
				{
					if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
					{
						m_pRenderer->Translate(DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
					}
					break;
				}
				case 'd':
				{
					if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
					{
						m_pRenderer->Translate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
					}
					break;
				}

				case 'q':
				{
					if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
					{
						m_pRenderer->Translate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
					}
					break;
				}

				case 'e':
				{
					if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
					{
						m_pRenderer->Translate(DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));
					}
					break;
				}
				default:
					break;
				}
			}

			default:
			{
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
		}
	}

	bool Engine::Frame()
	{
		bool bResult;

		m_timer.Tick();
		//Draw Scene
		m_pRenderer->BeginFrame();
		//Set DockerSpace and MenuBar
		m_ImGuiManager.Update();
		//Set ImGui 
		m_pEditor->Draw();
		//Present
		m_pRenderer->EndFrame();

		return true;
	}

	void Engine::InitWindows(UINT& m_screenHeight, UINT& m_screenWidth)
	{
		WNDCLASSEX wc;
		DEVMODE dmScreenSettings;
		int iPosX, iPosY;

		g_pEngineHandle = this;
		m_hInstance = GetModuleHandle(nullptr);
		m_appName = L"Wilson's Engine";

		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = m_hInstance;
		wc.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON1));
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = m_appName;
		wc.cbSize = sizeof(WNDCLASSEX);

		RegisterClassEx(&wc);

		m_screenHeight = GetSystemMetrics(SM_CYSCREEN);
		m_screenWidth = GetSystemMetrics(SM_CXSCREEN);

		if (g_bFULL_SCREEN)
		{
			ZeroMemory(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsHeight = static_cast<unsigned long>(m_screenHeight);
			dmScreenSettings.dmPelsWidth = static_cast<unsigned long>(m_screenWidth);
			dmScreenSettings.dmBitsPerPel = 64;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

			iPosX = iPosY = 0;

		}
		else
		{
			m_screenWidth = 1600;
			m_screenHeight = 800;

			iPosX = (GetSystemMetrics(SM_CXSCREEN) - m_screenWidth) / 2;
			iPosY = (GetSystemMetrics(SM_CYSCREEN) - m_screenHeight) / 2;
		}

		m_hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			m_appName,
			m_appName,
			WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,
			iPosX, iPosY, m_screenWidth, m_screenHeight,
			nullptr,
			nullptr,
			m_hInstance,
			nullptr);

		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);
		ImGui_ImplWin32_Init(m_hWnd);
		SetForegroundWindow(m_hWnd);
		SetFocus(m_hWnd);

		return;
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		
		case WM_CLOSE:
		
			PostQuitMessage(0);
			return 0;

		default:
		
			return g_pEngineHandle->MsgHandler(hWnd, uMsg, wParam, lParam);
		
		}
	}
}