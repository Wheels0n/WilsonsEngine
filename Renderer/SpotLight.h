#pragma once

#include"Light.h"
#include"typedef.h"
namespace wilson
{	

	class SpotLight:public Light
	{
	public:
		inline DirectX::XMMATRIX* GetLightSpaceMat()
		{
			return &m_lightSpaceMat;
		}
		inline DirectX::XMFLOAT3* GetAttenuation()
		{
			return &m_attenuation;
		}
		inline DirectX::XMFLOAT3* GetDirection()
		{
			return &m_direction;
		}
		inline float* GetRange()
		{
			return &m_range;
		}
		inline float* GetCutoff()
		{
			return &m_cutoff;
		}
		inline float* GetOuterCutoff()
		{
			return &m_outerCutoff;
		}
		inline SpotLightProperty* GetProperty()
		{
			return &m_spotLightProperty;
		}

		void UpdateViewMat();
		void UpdateProjMat();
		void UpdateLitMat();
		void UpdateProperty();
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::SPT; };

		SpotLight() = default;
		SpotLight(ID3D11Device*, UINT);
		SpotLight(const SpotLight&) = default;
		~SpotLight() { Light::~Light(); };
	private:
		SpotLightProperty m_spotLightProperty;
		
		float m_range;
		float m_cutoff;
		float m_outerCutoff;
		
		DirectX::XMFLOAT3 m_attenuation;
		DirectX::XMMATRIX m_perspectiveMat;
		DirectX::XMMATRIX m_viewMat;
		DirectX::XMMATRIX m_lightSpaceMat;
	};
}
