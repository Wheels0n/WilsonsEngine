#ifndef _ENGINE_H
#define _ENGINE_H

#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include "../ImGui/imgui_impl_win32.h"
#include"resource.h"
#include"Renderer.h"
#include"Timer.h"
#include"Editor.h"

namespace wilson{
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
	void InitWindows(int&, int&);
	void ShutdownWindows();

 private:
	int m_screenHeight;
	int m_screenWidth;

	int m_lastMouseX;
	int m_lastMouseY;
	int m_curMouseX;
	int m_curMouseY;

	static constexpr float m_mouseDragThreshold = 6.0f ;

	LPCWSTR m_appName;
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	Timer m_timer;
	ImGuiManager m_ImGuiManager;
	CRenderer* m_pRenderer;
	Editor* m_pEditor;
 };

 static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
 static Engine* g_pEngineHandle = nullptr;
 extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}
#endif