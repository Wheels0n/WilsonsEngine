#pragma once

#include "Renderer.h"
#include "typedef.h"

namespace wilson
{
	class D3D12;
	class Camera12;
	class Renderer12 : public Renderer
	{
	public:

		void BeginFrame();
		static bool CheckDx12Support();
		void EndFrame();
		inline D3D12* GetD3D12()
		{
			return m_pD3D12;
		};
		void Rotate(int dx, int dy);
		void Translate(const DirectX::XMVECTOR);
		void UpdateResolution(const UINT, const UINT);

		Renderer12() = default;
		Renderer12(const UINT, const UINT, HWND);
		Renderer12(Renderer12&) = delete;
		~Renderer12();
	private:
		Camera12* m_pCam;
		D3D12* m_pD3D12;
	};
}
