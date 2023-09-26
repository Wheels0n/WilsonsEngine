#pragma once

#include"Light.h"

namespace wilson
{	
	constexpr float _S_NEAR = 0.1f;
	constexpr float _S_FAR = 100.0f;

	struct SpotLightProperty
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;

		DirectX::XMFLOAT3 position;
		float range;

		DirectX::XMFLOAT3 direction;
		float cutoff;

		DirectX::XMFLOAT3 attenuation;
		float outerCutoff;
	};

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

		bool Init(ID3D11Device*);
		void UpdateViewMat();
		void UpdateProjMat();
		void UpdateLitMat();
		void UpdateProperty();
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::SPT; };

		SpotLight() = default;
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
