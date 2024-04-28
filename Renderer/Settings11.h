#pragma once 

#include"D3D11.h"
#include"FPS.h"
#include"Camera11.h"
#include"Light11.h"

namespace wilson
{
	class Settings11
	{
	public:
		void Draw();

		Settings11() = default;
		Settings11(D3D11*const pD3D11);
		Settings11(const Settings11&) = default;
		virtual ~Settings11();
	private:
		ID3D11ShaderResourceView* m_icons[3];//dir, point, spot;
		FPS m_FPS;
		Camera11* m_pCam;
		D3D11* m_pD3D11;
		Frustum11* m_pFrustum;
		Light11* m_pLight;
		
		BOOL* m_pbHeightOn;
		float* m_pExposure;
		float* m_pHeightScale;

		XMFLOAT3 m_prevPos;
		XMFLOAT3 m_prevAngleFloat;
		float m_prevFarZ;
		float m_prevNearZ;
	};
}

