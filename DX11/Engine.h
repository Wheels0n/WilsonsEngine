#pragma once 

#define WIN32_LEAN_AND_MEAN
#include<Windows.h>

#include "../ImGui/imgui_impl_win32.h"
#include"resource.h"
#include"Renderer.h"
#include"Timer.h"
#include"Editor.h"

namespace wilson{
	static constexpr float _DRAG_THRESHOLD = 6.0f;
	static bool g_bShutdown = false;
 class Engine
 {
 public:

	bool Init();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MsgHandler(HWND, UINT, WPARAM, LPARAM);

	Engine();
	Engine(Engine&) = delete;
	~Engine()=default;
 private:
	bool Frame();
	void InitWindows(UINT&, UINT&);
	void ShutdownWindows();

 private:
	UINT m_screenHeight;
	UINT m_screenWidth;

	int m_lastMouseX;
	int m_lastMouseY;
	int m_curMouseX;
	int m_curMouseY;

	LPCWSTR m_appName;
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	Timer m_timer;
	ImGuiManager m_ImGuiManager;
	Renderer* m_pRenderer;
	Editor* m_pEditor;
 };
 static Engine* g_pEngineHandle = nullptr;
 static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
