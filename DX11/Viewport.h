#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../ImGui/imgui.h"
#include <d3d11.h>
#include "Scene.h"
#include "D3D11.h"
#include "Import.h"

namespace wilson
{
	static const char* texFormats[] =
	{
		"bmp", "dds", "png"
	};

	class Viewport
	{
	public:
		void Init(CD3D11*, CScene*);
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
		bool m_IsFocused;

		int m_width;
		int m_height;
		int m_left;
		int m_top;

		CD3D11* m_pD3D11;
		Camera* m_pCam;
		CScene* m_pScene;
		Importer m_importer;
		IDXGISwapChain* m_pSwapChain;
		ID3D11ShaderResourceView* m_pSRV;
		ID3D11Device* m_pDevice;
	};
}
#endif // !VIEWPORT_H

