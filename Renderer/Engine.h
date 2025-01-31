#pragma once 

#define WIN32_LEAN_AND_MEAN
#include"typedef.h"

#include "../ImGui/imgui_impl_win32.h"
#include "ImGuiManager.h"
#include "resource.h"
#include "Timer.h"

namespace wilson{

	constexpr UINT _DEFAULT_SCREEN_WITDH = 1600;
	constexpr UINT _DEFAULT_SCREEN_HEIGHT = 800;

	using namespace std;

	 class Editor12;
	 class Renderer12;
	 class InputHandler;
	 class Engine
	 {
	 public:
		 UINT GetScreenWidth() { return m_screenWidth; };
		 UINT GetScreenHeight() { return m_screenHeight; };

		void InitWindows(UINT& screenWidth, UINT& screenHeight);
		void Run();

		LRESULT MsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		Engine();
		Engine(Engine&) = delete;
		~Engine();
	 private:
		bool Frame();
			
	 private:
		 int m_curMouseX;
		 int m_curMouseY;
		 int m_lastMouseX;
		 int m_lastMouseY;
		UINT m_screenWidth;
		UINT m_screenHeight;

		LPCWSTR m_appName;
		HINSTANCE m_hInstance;
		HWND m_hWnd;

		Timer m_timer;
		ImGuiManager m_ImGuiManager;
		shared_ptr<Renderer12> m_pRenderer;
		shared_ptr<Editor12> m_pEditor;
	 };
	 extern Engine* g_pEngineHandle;
	 static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
