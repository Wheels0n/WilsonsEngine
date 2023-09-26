#pragma once

#include<vector>

#include"Light.h"

namespace wilson
{	
	struct DirLightProperty
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;
		DirectX::XMFLOAT3 direction;
		float pad;
	};

	class DirectionalLight : public Light
	{
	public:
		inline std::vector<DirectX::XMMATRIX>& GetLightSpaceMat()
		{
			return m_lightSpaceMat;
		}
	
		inline DirLightProperty* GetProperty()
		{
			return &m_dirLightProperty;
		}
		
		bool Init(ID3D11Device*);
		void UpdateProperty();
		void SetShadowMatrices(ID3D11DeviceContext*);
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::DIR; };
		DirectX::XMMATRIX UpdateLightSpaceMat(const float nearZ, const float farZ);
		void UpdateLightSpaceMatrices();

		DirectionalLight(Camera* pCam);
		DirectionalLight(const DirectionalLight&) = default;
		~DirectionalLight();
	private:
		std::vector<DirectX::XMVECTOR> GetFrustumCornersWorldSpace(DirectX::XMMATRIX viewMat, DirectX::XMMATRIX projMat);
	private:
		Camera* m_pCam;
		DirLightProperty m_dirLightProperty;
		std::vector<DirectX::XMMATRIX> m_lightSpaceMat;

		ID3D11Buffer* m_pMatriceBuffer;
	};
}