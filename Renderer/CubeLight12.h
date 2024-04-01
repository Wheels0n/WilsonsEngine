#pragma once

#include <vector>

#include "Light12.h"
#include "typedef.h"
namespace wilson
{
	class HeapManager;
	class CubeLight12 :public Light12
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
		inline CubeLightProperty* GetProperty()
		{
			return &m_CubeLightProperty;
		}

		void UpdateProperty();
		void CreateShadowMatrices();
		void UploadShadowMatrices(ID3D12GraphicsCommandList*);
		void UploadLightPos(ID3D12GraphicsCommandList*);
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::CUBE; };

		CubeLight12() = default;
		CubeLight12(ID3D12Device*, ID3D12GraphicsCommandList*, HeapManager*, UINT idx);
		CubeLight12(const CubeLight12&) = default;
		~CubeLight12();

	private:
		CubeLightProperty m_CubeLightProperty;

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