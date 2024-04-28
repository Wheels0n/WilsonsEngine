#pragma once

#include"Light12.h"
#include"typedef.h"
namespace wilson
{
	class HeapManager;
	class DirectionalLight12 : public Light12
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
		DirectX::XMMATRIX UpdateLightSpaceMat(const float nearZ, const float farZ);
		void UpdateLightSpaceMatrices();
		void UpdateProperty();
		void UploadShadowMatrices(ID3D12GraphicsCommandList* const);

		DirectionalLight12(ID3D12Device* const, ID3D12GraphicsCommandList* const, HeapManager* const, const UINT, Camera12* const pCam);
		DirectionalLight12(const DirectionalLight12&) = default;
		~DirectionalLight12();
	private:
		std::vector<DirectX::XMVECTOR> GetFrustumCornersWorldSpace(const DirectX::XMMATRIX viewMat, const DirectX::XMMATRIX projMat);
	private:
		Camera12* m_pCam;
		DirLightProperty m_dirLightProperty;
		std::vector<DirectX::XMMATRIX> m_lightSpaceMat;
		D3D12_GPU_DESCRIPTOR_HANDLE m_matriceCbv;
		UINT8* m_pMatricesCbBegin;
	};
}