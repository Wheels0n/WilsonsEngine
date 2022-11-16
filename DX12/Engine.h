#ifndef _Engine_H_
#define _Engine_H_

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "resource.h"
#include "InputHandler.h"
#include "Renderer.h"

class CEngine
{
public:
	CEngine();
	CEngine(const CEngine&);
	~CEngine();

	bool Initialize();
	void Shutdwon();
	void Run();

	LRESULT CALLBACK MsgHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR m_appName;
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	CInputHandler* m_pInputHandler;
	CRenderer* m_pRenderer;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static CEngine* g_pEngineHandle = 0;

#endif 