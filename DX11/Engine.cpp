#include"Engine.h"

CEngine::CEngine()
{
	m_pInputHandler = nullptr;
	m_pRenderer = nullptr;
	m_pEditor = nullptr;

	m_screenHeight = 0;
	m_screenWidth = 0;
	m_lastMouseX = 0;
	m_lastMouseY = 0;
	m_curMouseX = 0;
	m_curMouseY = 0;
}

CEngine::~CEngine()
{

}

bool CEngine::Init()
{
	bool bResult;

	InitWindows(m_screenHeight, m_screenWidth);

	m_pInputHandler = new CInputHandler;
	if (m_pInputHandler == nullptr)
	{
		return false;
	}
	m_pInputHandler->Init();

	m_pRenderer = new CRenderer;
	if (m_pRenderer == nullptr)
	{
		return false;
	}

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	bResult = m_pRenderer->Init(rc.right - rc.left, rc.bottom - rc.top, m_hWnd);
	if (bResult == false)
	{
		return false;
	}
	
	m_pEditor = new CEditor;
	if (m_pEditor == nullptr)
	{
		return false;
	}
	m_pEditor->Init(m_pRenderer->GetD3D11());

	return true;
}

void CEngine::Shutdown()
{
	if (m_pInputHandler != nullptr)
	{
		delete m_pInputHandler;
		m_pInputHandler = nullptr;
	}

	if (m_pRenderer != nullptr)
	{ 
		m_pRenderer->Shutdown();
		delete m_pRenderer;
		m_pRenderer = nullptr;
	}

	if (m_pEditor == nullptr)
	{
		delete m_pEditor;
		m_pEditor = nullptr;
	}

	ShutdownWindows();

	return;
}

void CEngine::Run()
{
	MSG msg;
	bool bExit, bResult;

	ZeroMemory(&msg, sizeof(MSG));
	m_CTimer.Reset();

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

LRESULT CEngine::MsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   
	if (uMsg == WM_LBUTTONDOWN && m_pEditor != nullptr)
	{   
		//현재 창 기준 상대적 좌표
		int x = (short)(lParam)& 0xffff;
		int y = (lParam >> 16) & 0xffff;
		
		m_lastMouseX = x;
		m_lastMouseY = y;
		m_curMouseX = x;
		m_curMouseY = y;

		m_pEditor->Pick();

	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{   
		return 0;
	}
	switch (uMsg)
	{ 
	  case WM_KEYDOWN:
	  {   
		  m_pInputHandler->KeyDown(static_cast<unsigned int>(wParam));
		  return 0;
	  }

	  case WM_KEYUP:
	  { 
		  m_pInputHandler->KeyUp(static_cast<unsigned int>(wParam));
		  return 0;
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

			  int dyaw = m_lastMouseX - m_curMouseX > 0 ? 1:-1;
			  int dpitch = m_lastMouseY - m_curMouseY > 0 ? 1 : -1;

			  if (dyaw != 0 && dpitch != 0 &&
				  m_pEditor->CheckRange(winPosX + m_lastMouseX, winPosY + m_lastMouseY))
			  {
				  m_pRenderer->Rotate(dpitch, dyaw);
			  }
			  m_lastMouseX = x;
			  m_lastMouseY = y;
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
				  m_pRenderer->Translate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			  }
			  break;
		  }
		  case 's':
		  {
			  if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
			  {
				  m_pRenderer->Translate(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			  }
			  break;
		  }
		  case 'a':
		  {
			  if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
			  {
				  m_pRenderer->Translate(XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
			  }
			  break;
		  }
		  case 'd':
		  {
			  if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
			  {
				  m_pRenderer->Translate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
			  }
			  break;
		  }

		  case 'q':
		  {
			  if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
			  {
				  m_pRenderer->Translate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			  }
			  break;
		  }

		  case 'e':
		  {
			  if (GetAsyncKeyState(VK_LBUTTON) * 0x8000)
			  {
				  m_pRenderer->Translate(XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));
			  }
			  break;
		  }
		   default :
			   break;
		  }
	  }

	  default:
	  {
		  return DefWindowProc(hWnd, uMsg, wParam, lParam);
	  }
	}
}

bool CEngine::Frame()
{
	bool bResult;

	if (m_pInputHandler->IsKeyDown(VK_ESCAPE))
	{
		return false;
	} 
	
	m_CTimer.Tick();
	
	m_pRenderer->BeginFrame();
	m_CImGuiManager.Update();
	m_pEditor->Draw();
	m_pRenderer->EndFrame();

	return true;
}

void CEngine::InitWindows(int& m_screenHeight, int& m_screenWidth)
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

	if (g_bFull_SCREEN)
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
		m_screenWidth = 1280;
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

void CEngine::ShutdownWindows()
{
	if (g_bFull_SCREEN)
	{
		ChangeDisplaySettings(nullptr, 0);
	}

	ImGui_ImplWin32_Shutdown();
	DestroyWindow(m_hWnd);
	m_hWnd = nullptr;

	UnregisterClassW(m_appName, m_hInstance);
	m_hInstance = nullptr;

	g_pEngineHandle = nullptr;

	return;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{   

	switch (uMsg)
	{
	 case WM_DESTROY:
	 {
		 PostQuitMessage(0);
		 return 0;
	 }
	 
	 case WM_CLOSE:
	 {
		 PostQuitMessage(0);
		 return 0;
	 }

	 default:
	 {
		 return g_pEngineHandle->MsgHandler(hWnd, uMsg, wParam, lParam);
	 }
	}
}