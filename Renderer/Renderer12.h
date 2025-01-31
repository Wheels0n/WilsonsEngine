#pragma once

#include "typedef.h"

namespace wilson
{
	using namespace std;
	using namespace DirectX;

	class D3D12;
	class Camera12;
	class Renderer12 
	{
	public:
		inline int								GetClientHeight() const
		{
			return m_screenHeight;
		};
		inline int								GetClientWidth() const
		{
			return m_screenWidth;
		};

		static BOOL CheckDx12Support();

		void BeginFrame();
		void EndFrame();

		shared_ptr<D3D12> GetD3D12();

		void Rotate(int dx, int dy);
		void Translate(const XMVECTOR);
		void UpdateResolution(const UINT, const UINT);

		Renderer12(const UINT, const UINT, HWND);
		Renderer12(Renderer12&) = delete;
		~Renderer12();
	private:
		UINT m_screenHeight;
		UINT m_screenWidth;

		shared_ptr<Camera12> m_pCam;
		shared_ptr<D3D12> m_pD3D12;
	};
}
