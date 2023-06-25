#ifndef _RENDERER_H_
#define _RENDERER_H_

#include<Windows.h>
#include "D3D11.h"
#include "ImGuiManager.h"

constexpr bool g_bFull_SCREEN = false;
constexpr bool g_bVSYNC_ENABLE =false;
constexpr float g_fSCREEN_FAR = 3000.0f;
constexpr float g_fSCREEN_NEAR = 0.1f;

namespace wilson
{
	class Renderer
	{
	public:
		bool Init(int, int, HWND);
		void Shutdown();
		void BeginFrame();
		void EndFrame();

		inline D3D11* GetD3D11()
		{
			return m_pD3D11;
		};

		void Translate(XMVECTOR);
		void Rotate(int dx, int dy);

		Renderer();
		Renderer(Renderer&) = delete;
		~Renderer();
	 private:
		bool Render();

		D3D11* m_pD3D11;
		Camera* m_pCam;
	};
}
#endif