#pragma once
#include <vector>
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
		void CreateShadowMatrices();
		void SetShadowMatrices(ID3D11DeviceContext*);
		void SetLightPos(ID3D11DeviceContext*);
		ELIGHT_TYPE GetType() { return ELIGHT_TYPE::PNT; };

		PointLight() = default;
		PointLight(const PointLight&) = default;
		~PointLight();
		
	private:
		PointLightProperty m_pointLightProperty;
		ID3D11Buffer* m_pMatricesBuffer, *m_pPosBuffer;

		float m_range;
		DirectX::XMFLOAT3 m_attenuation;
		
		static DirectX::XMMATRIX g_perspectiveMat;
		static std::vector<DirectX::XMVECTOR> g_upVectors;
		static std::vector<DirectX::XMVECTOR> g_dirVectors;
		std::vector<DirectX::XMMATRIX> m_cubeMats;
	};
}