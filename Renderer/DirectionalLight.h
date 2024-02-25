#pragma once

#include<vector>

#include"Light.h"
#include"typedef.h"
namespace wilson
{	

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
		
		void UpdateProperty();
		void UploadShadowMatrices(ID3D11DeviceContext*);
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::DIR; };
		DirectX::XMMATRIX UpdateLightSpaceMat(const float nearZ, const float farZ);
		void UpdateLightSpaceMatrices();

		DirectionalLight(ID3D11Device*, UINT, Camera* pCam);
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