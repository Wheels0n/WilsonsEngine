#ifndef _ENGINE_H
#define _ENGINE_H

#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include "../ImGui/imgui_impl_win32.h"
#include"resource.h"
#include"InputHandler.h"
#include"Renderer.h"
#include"Timer.h"
#include"Editor.h"


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
	int m_screenHeight;
	int m_screenWidth;

	int m_lastMouseX;
	int m_lastMouseY;
	int m_curMouseX;
	int m_curMouseY;

	LPCWSTR m_appName;
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	CTimer m_CTimer;
	CImGuiManager m_CImGuiManager;
	CRenderer* m_pRenderer;
	CInputHandler* m_pInputHandler;
	CEditor* m_pEditor;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static CEngine* g_pEngineHandle = nullptr;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif