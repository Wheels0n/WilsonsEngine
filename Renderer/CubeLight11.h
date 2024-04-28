#pragma once
#include "Light11.h"
#include "typedef.h"
namespace wilson
{	

	class CubeLight11 :public Light11
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
		void UploadLightPos(ID3D11DeviceContext* const);
		void UploadShadowMatrices(ID3D11DeviceContext* const);

		CubeLight11() = default;
		CubeLight11(ID3D11Device* const, const UINT idx);
		CubeLight11(const CubeLight11&) = default;
		~CubeLight11();
		
	private:
		CubeLightProperty m_cubeLightProperty;
		ID3D11Buffer* m_pMatricesCb;
		ID3D11Buffer* m_pPosCb;

		DirectX::XMFLOAT3 m_attenuation;
		float m_range;
		
		static std::vector<DirectX::XMVECTOR> g_dirVectors;
		static std::vector<DirectX::XMVECTOR> g_upVectors;

		std::vector<DirectX::XMMATRIX> m_cubeMats;
		static DirectX::XMMATRIX g_perspectiveMat;
	};
}