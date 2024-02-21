#pragma once

#include<Windows.h>

#include "Renderer.h"
#include "typedef.h"

namespace wilson
{

	class D3D11;
	class Camera;

	class Renderer11 : public Renderer
	{
	public:
		
		void BeginFrame();
		void EndFrame();

		inline D3D11* GetD3D11()
		{
			return m_pD3D11;
		};
		void UpdateResolution(int, int);
		void Translate(DirectX::XMVECTOR);
		void Rotate(int dx, int dy);

		Renderer11()=default;
		Renderer11(int, int, HWND);
		Renderer11(Renderer11&) = delete;
		~Renderer11();
	 private:
		D3D11* m_pD3D11;
		Camera* m_pCam;
	};
}
