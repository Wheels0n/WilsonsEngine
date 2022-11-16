#include "Engine.h"

CEngine::CEngine()
{  
	//If these members are not null, shutdown fun will attempt to clean up them objects
	m_pInputHandler = nullptr;
	m_pRenderer = nullptr;
}

CEngine::CEngine(const CEngine&)
{
}

CEngine::~CEngine()
{
}

bool CEngine::Initialize()
{   
	int iScreenHeight, iScreenWidth;
	bool bResult;

	iScreenHeight = 0;
	iScreenWidth  = 0;
    
	InitializeWindows(iScreenHeight, iScreenWidth);

	m_pInputHandler = new CInputHandler;
	if (m_pInputHandler == nullptr)
	{
		return false;
	}

	m_pInputHandler->Initialize();

	m_pRenderer = new CRenderer;
	if (m_pRenderer == nullptr)
	{
		return false;
	}
    
	bResult = m_pRenderer->Initialize(iScreenHeight, iScreenWidth, m_hWnd);
	if(bResult == false)
	{
		return false;
	}

	return true;
}

void CEngine::Shutdwon()
{
	if (m_pRenderer!=nullptr)
	{
		m_pRenderer->Shutdown();
		delete m_pRenderer;
		m_pRenderer = nullptr;
	}

	if (m_pInputHandler != nullptr)
	{
		delete m_pInputHandler;
		m_pInputHandler = nullptr;
	}

	ShutdownWindows();

	return;
}

void CEngine::Run()
{
	MSG msg;
	bool bExit, bResult;
	
	ZeroMemory(&msg, sizeof(MSG));

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
	switch (uMsg)
	{
	 case WM_KEYDOWN:
	 {
		 m_pInputHandler->KeyDown((unsigned int)wParam);
		 return 0;
	 }
	 case WM_KEYUP:
	 {
		 m_pInputHandler->KeyUp((unsigned int)wParam);
		 return 0;
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

	bResult = m_pRenderer->Frame();
	if (bResult == false)
	{
		return false;
	}
	
	return true;
}

void CEngine::InitializeWindows(int& iScreenHeight, int& iScreenWidth)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int iPosX, iPosY;

	g_pEngineHandle = this;

	m_hInstance = GetModuleHandle(nullptr);

	m_appName = L"Wilson'sEngine";

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = m_appName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	iScreenWidth  = GetSystemMetrics(SM_CXSCREEN);

	if (g_bFULL_SCREEN)
	{
		ZeroMemory(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsHeight = (unsigned long)iScreenHeight;
		dmScreenSettings.dmPelsWidth  = (unsigned long)iScreenWidth;
		dmScreenSettings.dmBitsPerPel = 64;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		iPosX = iPosY = 0;
	}
	else
	{
		iScreenWidth = 1280;
		iScreenHeight = 720;

		iPosX = (GetSystemMetrics(SM_CXSCREEN) - iScreenWidth) / 2;
		iPosY = (GetSystemMetrics(SM_CYSCREEN) - iScreenHeight) / 2;
	}

	m_hWnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		m_appName,
		m_appName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		iPosX, iPosY, iScreenWidth, iScreenHeight,
		nullptr,
		nullptr,
		m_hInstance,
		nullptr);

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	return;
}
void CEngine::ShutdownWindows()
{
	if (g_bFULL_SCREEN)
	{
		ChangeDisplaySettings(nullptr, 0);
	}

	DestroyWindow(m_hWnd);
	m_hWnd = nullptr;

	UnregisterClassW(m_appName, m_hInstance);
	m_hInstance = nullptr;

	g_pEngineHandle = nullptr;

	return;
}

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
