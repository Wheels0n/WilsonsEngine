#pragma once

#include <vector>

#include "Light.h"
#include "typedef.h"
namespace wilson
{	

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

		void UpdateProperty();
		void CreateShadowMatrices();
		void UploadShadowMatrices(ID3D11DeviceContext*);
		void UploadLightPos(ID3D11DeviceContext*);
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::PNT; };

		PointLight() = default;
		PointLight(ID3D11Device*, UINT idx);
		PointLight(const PointLight&) = default;
		~PointLight();
		
	private:
		PointLightProperty m_pointLightProperty;
		ID3D11Buffer* m_pMatricesBuffer;
		ID3D11Buffer* m_pPosBuffer;

		float m_range;
		DirectX::XMFLOAT3 m_attenuation;
		
		static DirectX::XMMATRIX g_perspectiveMat;
		static std::vector<DirectX::XMVECTOR> g_upVectors;
		static std::vector<DirectX::XMVECTOR> g_dirVectors;
		std::vector<DirectX::XMMATRIX> m_cubeMats;
	};
}