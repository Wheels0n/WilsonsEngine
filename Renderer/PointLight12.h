#pragma once

#include <vector>

#include "Light12.h"
#include "typedef.h"
namespace wilson
{
	class DescriptorHeapManager;
	class PointLight12 :public Light12
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
		void UploadShadowMatrices(ID3D12GraphicsCommandList*);
		void UploadLightPos(ID3D12GraphicsCommandList*);
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::PNT; };

		PointLight12() = default;
		PointLight12(ID3D12Device*, ID3D12GraphicsCommandList*, DescriptorHeapManager*, UINT idx);
		PointLight12(const PointLight12&) = default;
		~PointLight12();

	private:
		PointLightProperty m_pointLightProperty;

		ID3D12Resource* m_pMatrices12Buffer;
		ID3D12Resource* m_pPosCb;
		D3D12_GPU_DESCRIPTOR_HANDLE m_posCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_matriceCBV;

		float m_range;
		DirectX::XMFLOAT3 m_attenuation;

		static DirectX::XMMATRIX g_perspectiveMat;
		static std::vector<DirectX::XMVECTOR> g_upVectors;
		static std::vector<DirectX::XMVECTOR> g_dirVectors;
		std::vector<DirectX::XMMATRIX> m_cubeMats;

		UINT8* m_pMatricesCbBegin;
		UINT8* m_pPosCbBegin;
	};
}