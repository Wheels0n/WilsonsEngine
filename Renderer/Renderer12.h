#pragma once

#include<Windows.h>
#include "Renderer.h"
#include "typedef.h"

namespace wilson
{
	class D3D12;
	class Camera12;
	class Renderer12 : public Renderer
	{
	public:

		static bool CheckDx12Support();
		void BeginFrame();
		void EndFrame();

		inline D3D12* GetD3D12()
		{
			return m_pD3D12;
		};
		void UpdateResolution(int, int);
		void Translate(DirectX::XMVECTOR);
		void Rotate(int dx, int dy);

		Renderer12() = default;
		Renderer12(int, int, HWND);
		Renderer12(Renderer12&) = delete;
		~Renderer12();
	private:
		D3D12* m_pD3D12;
		Camera12* m_pCam;
	};
}
#pragma once
