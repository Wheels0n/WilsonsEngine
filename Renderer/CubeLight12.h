#pragma once
#include "Light12.h"
#include "typedef.h"
namespace wilson
{
	class HeapManager;
	class CubeLight12 :public Light12
	{
	public:
		void CreateShadowMatrices();
		inline DirectX::XMFLOAT3* GetAttenuation()
		{
			return &m_attenuation;
		}
		inline CubeLightProperty* GetProperty()
		{
			return &m_cubeLightProperty;
		}
		inline float* GetRange()
		{
			return &m_range;
		}
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::CUBE; };
		void UpdateProperty();
		void UploadLightPos(ID3D12GraphicsCommandList* const);
		void UploadShadowMatrices(ID3D12GraphicsCommandList* const);

		CubeLight12() = default;
		CubeLight12(ID3D12Device* const, ID3D12GraphicsCommandList* const, HeapManager* const, const UINT idx);
		CubeLight12(const CubeLight12&)=default;
		~CubeLight12();

	private:
		CubeLightProperty m_cubeLightProperty;

		D3D12_GPU_DESCRIPTOR_HANDLE m_matriceCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_posCbv;

		DirectX::XMFLOAT3 m_attenuation;
		float m_range;

		std::vector<DirectX::XMMATRIX> m_cubeMats;
		static DirectX::XMMATRIX g_perspectiveMat;

		static std::vector<DirectX::XMVECTOR> g_dirVectors;
		static std::vector<DirectX::XMVECTOR> g_upVectors;

		UINT8* m_pMatricesCbBegin;
		UINT8* m_pPosCbBegin;
	};
}