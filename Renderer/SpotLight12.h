#pragma once

#include"Light12.h"
#include"typedef.h"
namespace wilson
{
	class HeapManager;
	class SpotLight12 :public Light12
	{
	public:
		inline DirectX::XMMATRIX* GetLightSpaceMat()
		{
			return &m_lightSpaceMat;
		}
		inline DirectX::XMFLOAT3* GetAttenuation()
		{
			return &m_attenuation;
		}
		inline DirectX::XMFLOAT3* GetDirection()
		{
			return &m_direction;
		}
		inline float* GetRange()
		{
			return &m_range;
		}
		inline float* GetCutoff()
		{
			return &m_cutoff;
		}
		inline float* GetOuterCutoff()
		{
			return &m_outerCutoff;
		}
		inline SpotLightProperty* GetProperty()
		{
			return &m_spotLightProperty;
		}

		void UpdateViewMat();
		void UpdateProjMat();
		void UpdateLitMat();
		void UpdateProperty();
		eLIGHT_TYPE GetType() { return eLIGHT_TYPE::SPT; };

		SpotLight12() = default;
		SpotLight12(ID3D12Device*, ID3D12GraphicsCommandList*, HeapManager*, UINT);
		SpotLight12(const SpotLight12&) = default;
		~SpotLight12() { Light12::~Light12(); };
	private:
		SpotLightProperty m_spotLightProperty;

		float m_range;
		float m_cutoff;
		float m_outerCutoff;

		DirectX::XMFLOAT3 m_attenuation;
		DirectX::XMMATRIX m_perspectiveMat;
		DirectX::XMMATRIX m_viewMat;
		DirectX::XMMATRIX m_lightSpaceMat;
	};
}
