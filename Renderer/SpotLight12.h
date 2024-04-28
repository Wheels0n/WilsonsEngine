#pragma once

#include"Light12.h"
#include"typedef.h"
namespace wilson
{
	class HeapManager;
	class SpotLight12 :public Light12
	{
	public:
		inline DirectX::XMFLOAT3* GetAttenuation()
		{
			return &m_attenuation;
		}
		inline float* GetCutoff()
		{
			return &m_cutoff;
		}
		inline DirectX::XMFLOAT3* GetDirection()
		{
			return &m_direction;
		}
		inline DirectX::XMMATRIX* GetLightSpaceMat()
		{
			return &m_lightSpaceMat;
		}
		inline float* GetOuterCutoff()
		{
			return &m_outerCutoff;
		}
		inline SpotLightProperty* GetProperty()
		{
			return &m_spotLightProperty;
		}
		inline float* GetRange()
		{
			return &m_range;
		}
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::SPT; };
		void UpdateLitMat();
		void UpdateProjMat();
		void UpdateProperty();
		void UpdateViewMat();

		SpotLight12() = default;
		SpotLight12(ID3D12Device* const, ID3D12GraphicsCommandList* const, HeapManager* const, const UINT);
		SpotLight12(const SpotLight12&) = default;
		~SpotLight12() { Light12::~Light12(); };
	private:
		SpotLightProperty m_spotLightProperty;

		float m_cutoff;
		float m_outerCutoff;
		float m_range;

		DirectX::XMFLOAT3 m_attenuation;
		DirectX::XMMATRIX m_lightSpaceMat;
		DirectX::XMMATRIX m_perspectiveMat;
		DirectX::XMMATRIX m_viewMat;
	};
}
