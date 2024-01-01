#pragma once

#include<Windows.h>

#include "D3D11.h"
#include "ImGuiManager.h"

constexpr bool g_bFULL_SCREEN = false;
constexpr bool g_bVSYNC_ENABLE =false;
constexpr float g_fSCREEN_FAR = 3000.0f;
constexpr float g_fSCREEN_NEAR = 0.1f;

namespace wilson
{
	class Renderer
	{
	public:
		
		void BeginFrame();
		void EndFrame();

		inline D3D11* GetD3D11()
		{
			return m_pD3D11;
		};

		void UpdateResolution(int, int);
		void Translate(XMVECTOR);
		void Rotate(int dx, int dy);

		Renderer()=default;
		Renderer(int, int, HWND);
		Renderer(Renderer&) = delete;
		~Renderer();
	 private:

		D3D11* m_pD3D11;
		Camera* m_pCam;
	};
}
