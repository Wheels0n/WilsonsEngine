#pragma once

#include"Light11.h"
#include"typedef.h"
namespace wilson
{	

	class SpotLight11:public Light11
	{
	public:
		inline DirectX::XMFLOAT3* GetAttenuation()
		{
			return &m_attenuation;
		}
		inline float* GetCutoff()
		{
			return &m_cutoff;
		}
		inline DirectX::XMFLOAT3* GetDirection()
		{
			return &m_direction;
		}
		inline DirectX::XMMATRIX* GetLightSpaceMat()
		{
			return &m_lightSpaceMat;
		}
		inline float* GetOuterCutoff()
		{
			return &m_outerCutoff;
		}
		inline SpotLightProperty* GetProperty()
		{
			return &m_spotLightProperty;
		}
		inline float* GetRange()
		{
			return &m_range;
		}
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::SPT; };
		void UpdateLitMat();
		void UpdateProjMat();
		void UpdateProperty();
		void UpdateViewMat();

		SpotLight11() = default;
		SpotLight11(ID3D11Device* const, const UINT);
		SpotLight11(const SpotLight11&) = default;
		~SpotLight11() { Light11::~Light11(); };
	private:
		SpotLightProperty m_spotLightProperty;
		
		float m_cutoff;
		float m_outerCutoff;
		float m_range;

		DirectX::XMFLOAT3 m_attenuation;
		DirectX::XMMATRIX m_lightSpaceMat;
		DirectX::XMMATRIX m_perspectiveMat;
		DirectX::XMMATRIX m_viewMat;
	};
}
