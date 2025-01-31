#pragma once

#include"Light12.h"
#include"typedef.h"
namespace wilson
{
	using namespace std;
	using namespace DirectX;
	using namespace Microsoft::WRL;

	class HeapManager;
	class SpotLight12 :public Light12
	{
	public:

		XMMATRIX*						GetSpotLightSpaceMatrix();
		eLIGHT_TYPE						GetType() { return eLIGHT_TYPE::SPT; };
		FLOAT&							GetCutOff() { return m_cutoff; };
		FLOAT&							GetOuterCutOff() { return m_outerCutoff; };

		void							UpdateViewMatrix();
		void							UpdateProjMatrix();
		void							UpdateSpotLightMatrix();

		void							UploadSpotLightMatrix(ComPtr<ID3D12GraphicsCommandList> pCmdList);

										SpotLight12(const UINT);
										SpotLight12(const SpotLight12&) = default;
										~SpotLight12();
	private:
		FLOAT							m_cutoff;
		FLOAT							m_outerCutoff;

		UINT							m_spotLightMatrixKey;
		XMFLOAT3						m_attenuation;
		shared_ptr<WVPMatrices>			m_pWVPMatrices;
		shared_ptr<CompositeMatrices>	m_pCompositeMatrices;
	};
}
