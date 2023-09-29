#pragma once

#include <d3d11.h>

#include "../ImGui/imgui.h"
#include "Scene.h"
#include "D3D11.h"
#include "Import.h"

namespace wilson
{
	static const char* g_types[] =
	{
		"obj", "fbx", "dir", "pnt", "spt"
	};

	class Viewport
	{
	public:
		void Init(D3D11*, Scene*);
		void Draw();
		void Resize();
		bool CheckRange(int, int);
		
		inline float GetNDCX(int);
		inline float GetNDCY(int);
		inline int GetWidth() const
		{
			return m_width;
		};
		inline int GetHeight() const
		{
			return m_height;
		};
		inline int GetX() const
		{
			return m_left;
		};
		inline int GetY() const
		{
			return m_top;
		};

		Viewport() = default;
		Viewport(const Viewport&) = delete;
		~Viewport() = default;
	private:
		XMVECTOR CalEntityPos();
	private:
		bool m_IsFocused;

		UINT m_width;
		UINT m_height;
		UINT m_GbufferCount;
		int m_left;
		int m_top;

		D3D11* m_pD3D11;
		Camera* m_pCam;
		Scene* m_pScene;
		Importer m_importer;
		IDXGISwapChain* m_pSwapChain;

		ID3D11ShaderResourceView* m_pFinalSRV;
		ID3D11ShaderResourceView* m_pSSAOBlurredSRV;
		ID3D11ShaderResourceView** m_pGbufferSRV;

		ID3D11Device* m_pDevice;
	};
}

