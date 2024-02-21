#pragma once

#include<vector>

#include"Light12.h"
#include"typedef.h"
namespace wilson
{
	class DescriptorHeapManager;
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

		void UpdateProperty();
		void SetShadowMatrices(ID3D12GraphicsCommandList*);
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::DIR; };
		DirectX::XMMATRIX UpdateLightSpaceMat(const float nearZ, const float farZ);
		void UpdateLightSpaceMatrices();

		DirectionalLight12(ID3D12Device*, ID3D12GraphicsCommandList*, DescriptorHeapManager*, UINT, Camera12* pCam);
		DirectionalLight12(const DirectionalLight12&) = default;
		~DirectionalLight12();
	private:
		std::vector<DirectX::XMVECTOR> GetFrustumCornersWorldSpace(DirectX::XMMATRIX viewMat, DirectX::XMMATRIX projMat);
	private:
		Camera12* m_pCam;
		DirLightProperty m_dirLightProperty;
		std::vector<DirectX::XMMATRIX> m_lightSpaceMat;

		ID3D12Resource* m_pMatrice12Buffer;
		D3D12_GPU_DESCRIPTOR_HANDLE m_matriceCBV;
	};
}