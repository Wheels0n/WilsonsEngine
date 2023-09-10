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
		void Init(D3D11* pD3D11);
		void Draw();

		Settings() = default;
		Settings(const Settings&) = default;
		~Settings();
	private:
		ID3D11ShaderResourceView* m_icons[3];//dir, point, spot;

		FPS m_FPS;
		Light* m_pLight;
		Camera* m_pCam;
		float* m_pExposure;
	};
}

