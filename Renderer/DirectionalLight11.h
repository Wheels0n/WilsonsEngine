#pragma once

#include"Light11.h"
#include"typedef.h"
namespace wilson
{	

	class DirectionalLight11 : public Light11
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
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::DIR; };
		void UpdateProperty();
		DirectX::XMMATRIX UpdateLightSpaceMat(const float nearZ, const float farZ);
		void UpdateLightSpaceMatrices();
		void UploadShadowMatrices(ID3D11DeviceContext* const);

		DirectionalLight11(ID3D11Device* const, const UINT, Camera11* const pCam);
		DirectionalLight11(const DirectionalLight11&) = default;
		~DirectionalLight11();
	private:
		std::vector<DirectX::XMVECTOR> GetFrustumCornersWorldSpace(const DirectX::XMMATRIX viewMat, const DirectX::XMMATRIX projMat);
	private:
		Camera11* m_pCam;
		DirLightProperty m_dirLightProperty;
		std::vector<DirectX::XMMATRIX> m_lightSpaceMat;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pMatriceCb;
	};
}