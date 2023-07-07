#pragma once
#include "Light.h"
namespace wilson
{	
	struct PointLightProperty
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;

		DirectX::XMFLOAT3 position;
		float range;

		DirectX::XMFLOAT3 attenuation;
		float pad;
	};

	class PointLight :public Light
	{
	public:
		inline DirectX::XMFLOAT3* GetAttenuation()
		{
			return &m_attenuation;
		}
		inline float* GetRange()
		{
			return &m_range;
		}
		inline PointLightProperty* GetProperty()
		{
			return &m_pointLightProperty;
		}

		bool Init(ID3D11Device*);
		void UpdateProperty();
		ELIGHT_TYPE GetType() { return ELIGHT_TYPE::PNT; };

		PointLight() = default;
		PointLight(const PointLight&) = default;
		~PointLight() { Light::~Light(); };
	private:
		PointLightProperty m_pointLightProperty;
		float m_range;
		DirectX::XMFLOAT3 m_attenuation;
	};
}