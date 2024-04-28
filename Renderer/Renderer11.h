#pragma once
#include "Renderer.h"
#include "typedef.h"

namespace wilson
{

	class D3D11;
	class Camera11;

	class Renderer11 : public Renderer
	{
	public:
		
		void BeginFrame();
		void EndFrame();
		inline D3D11* GetD3D11()
		{
			return m_pD3D11;
		};
		void Rotate(int dx, int dy);
		void Translate(const DirectX::XMVECTOR);
		void UpdateResolution(const UINT, const UINT);

		Renderer11()=default;
		Renderer11(const UINT, const UINT, HWND);
		Renderer11(Renderer11&) = delete;
		~Renderer11();
	 private:
		D3D11* m_pD3D11;
		Camera11* m_pCam;
	};
}
