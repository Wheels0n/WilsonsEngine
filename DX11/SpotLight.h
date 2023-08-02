#pragma once
#include"Light.h"

namespace wilson
{	
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
		void UpdateProperty();
		ELIGHT_TYPE GetType() { return ELIGHT_TYPE::SPT; };

		SpotLight() = default;
		SpotLight(const SpotLight&) = default;
		~SpotLight() { Light::~Light(); };
	private:
		SpotLightProperty m_spotLightProperty;
		float m_range;
		float m_cutoff, m_outerCutoff;
		DirectX::XMFLOAT3 m_direction;
		DirectX::XMFLOAT3 m_attenuation;
	};
}
