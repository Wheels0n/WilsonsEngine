#ifndef _ENGINE_H
#define _ENGINE_H

#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include "../ImGui/imgui_impl_win32.h"
#include"resource.h"
#include"InputHandler.h"
#include"Renderer.h"


class CEngine
{
public:
	CEngine();
	CEngine(CEngine&) = delete;
	~CEngine();

	bool Init();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MsgHandler(HWND, UINT, WPARAM, LPARAM);
private:
	bool Frame();
	void InitWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR m_appName;
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	CImGuiManager m_CImGuiManager;
	CRenderer* m_pRenderer;
	CInputHandler* m_pInputHandler;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static CEngine* g_pEngineHandle = nullptr;

#endif