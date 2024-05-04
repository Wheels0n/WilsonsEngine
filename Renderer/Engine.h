#pragma once 

#define WIN32_LEAN_AND_MEAN
#include"typedef.h"

#include "../ImGui/imgui_impl_win32.h"
#include "ImGuiManager.h"
#include"resource.h"
#include"Timer.h"
namespace wilson{
 class Editor;
 class Renderer;

 class Engine
 {
 public:
	void Run();

	LRESULT CALLBACK MsgHandler(HWND, UINT, WPARAM, LPARAM);

	Engine();
	Engine(Engine&) = delete;
	~Engine();
 private:
	bool Frame();
	void InitWindows(UINT&, UINT&);

 private:
	UINT m_screenHeight;
	UINT m_screenWidth;

	int m_curMouseX;
	int m_curMouseY;
	int m_lastMouseX;
	int m_lastMouseY;

	LPCWSTR m_appName;
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	Timer m_timer;
	ImGuiManager m_ImGuiManager;
	std::unique_ptr<Renderer> m_pRenderer;
	std::unique_ptr<Editor> m_pEditor;
 };
 static Engine* g_pEngineHandle = nullptr;
 static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
