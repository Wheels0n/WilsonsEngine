#pragma once 

#include"D3D11.h"
#include"FPS.h"
#include"Camera.h"
#include"Light.h"

namespace wilson
{
	class Settings
	{
	public:
		void Draw();

		Settings() = default;
		Settings(D3D11* pD3D11);
		Settings(const Settings&) = default;
		virtual ~Settings();
	private:
		ID3D11ShaderResourceView* m_icons[3];//dir, point, spot;
		FPS m_FPS;
		D3D11* m_pD3D11;
		Light* m_pLight;
		Camera* m_pCam;
		Frustum* m_pFrustum;
		
		BOOL* m_pHeightOnOFF;
		float* m_pExposure;
		float* m_pHeightScale;

		XMFLOAT3 m_prevPos;
		XMFLOAT3 m_prevAngleFloat;
		float m_prevFarZ;
		float m_prevNearZ;
	};
}
