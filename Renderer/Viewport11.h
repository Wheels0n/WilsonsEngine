#pragma once

#include <d3d11.h>

#include "../ImGui/imgui.h"
#include "Scene11.h"
#include "D3D11.h"
#include "Import11.h"
#include "typedef.h"
namespace wilson
{

	class Viewport11
	{
	public:
		bool CheckRange(const int, const int);
		void Draw();
		inline int GetHeight() const
		{
			return m_height;
		};
		inline float GetNDCX(const int);
		inline float GetNDCY(const int);
		inline int GetWidth() const
		{
			return m_width;
		};
		inline int GetX() const
		{
			return m_left;
		};
		inline int GetY() const
		{
			return m_top;
		};
		void Resize();

		Viewport11() = default;
		Viewport11(D3D11* const, Scene11* const);
		Viewport11(const Viewport11&) = delete;
		~Viewport11();
	private:
		XMVECTOR CalEntityPos();
	private:
		bool m_IsFocused;

		UINT m_width;
		UINT m_height;
		UINT m_nGbuffer;
		int m_left;
		int m_top;

		D3D11* m_pD3D11;
		Camera11* m_pCam;
		Scene11* m_pScene;
		std::unique_ptr<Importer11> m_pImporter;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pPostProcessSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>m_pSsaoBlurredSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ppGbufferSrvs[static_cast<UINT>(eGbuf::cnt)];

		Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	};
}

