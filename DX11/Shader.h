#ifndef SHADER_H
#define SHADER_H

#include<d3d11.h>
#include<D3DX11tex.h>

namespace wilson
{
	class Shader
	{
	public:
		bool Init();
		inline void SetInputLayout()
		{
			m_pContext->IASetInputLayout(m_pInputLayout);
		}
		inline void SetPosOnlyInputLayout()
		{
			m_pContext->IASetInputLayout(m_pPosOnlyInputLayout);
		}
		inline void SetTexInputlayout()
		{
			m_pContext->IASetInputLayout(m_pTexInputLayout);
		}
		inline void SetDeferredGeoLayout()
		{
			m_pContext->IASetInputLayout(m_pDeferredGeoLayout);
		}
		inline void SetShader()
		{
			m_pContext->VSSetShader(m_pVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pPS, nullptr, 0);
		}
		inline void SetSkyBoxShader()
		{
			m_pContext->VSSetShader(m_pSkyBoxVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pSkyBoxPS, nullptr, 0);
		}
		inline void SetShadowShader()
		{
			m_pContext->VSSetShader(m_pShadowVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pShadowPS, nullptr, 0);
		}
		inline void SetOmniDirShadowShader()
		{
			m_pContext->VSSetShader(m_pOmniDirShadowVS, nullptr, 0);
			m_pContext->GSSetShader(m_pOmniDirShadowGS, nullptr, 0);
			m_pContext->PSSetShader(m_pOmniDirShadowPS, nullptr, 0);
		}
		inline void SetDeferredGeoShader()
		{
			m_pContext->VSSetShader(m_pGeometryVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pGeometryPS, nullptr, 0);
		}
		inline void SetPBRDeferredGeoShader()
		{
			m_pContext->VSSetShader(m_pPBRGeometryVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pPBRGeometryPS, nullptr, 0);
		}
		inline void SetSSAOShader()
		{
			m_pContext->VSSetShader(m_pTexVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pSSAOPS, nullptr, 0);
		}
		inline void SetSSAOBlurShader()
		{
			m_pContext->VSSetShader(m_pTexVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pSSAOBlurPS, nullptr, 0);
		}
		inline void SetDeferredLightingShader()
		{
			m_pContext->VSSetShader(m_pTexVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pDeferredPS, nullptr, 0);
		}
		inline void SetPBRDeferredLightingShader()
		{
			m_pContext->VSSetShader(m_pTexVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pPBRDeferredPS, nullptr, 0);
		}
		inline void SetCubeShader()
		{
			m_pContext->VSSetShader(m_pLightCubeVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pLightCubePS, nullptr, 0);
		}
		inline void SetBlurShader()
		{
			m_pContext->VSSetShader(m_pTexVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pBlurPS, nullptr, 0);
		}
		inline void SetOutlinerShader()
		{
			m_pContext->VSSetShader(m_pTexVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pOutlinerPS, nullptr, 0);
		}
		inline void SetFinalShader()
		{
			m_pContext->VSSetShader(m_pTexVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pFinPS, nullptr, 0);
		}

		Shader(ID3D11Device* pDevice, ID3D11DeviceContext* context);
		~Shader();

	private:

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pContext;

		ID3D11VertexShader* m_pVS, *m_pSkyBoxVS, *m_pShadowVS, *m_pOmniDirShadowVS, *m_pTexVS, *m_pGeometryVS, *m_pPBRGeometryVS, *m_pLightCubeVS;
		ID3D11GeometryShader* m_pOmniDirShadowGS;
		ID3D11PixelShader* m_pPS, *m_pSkyBoxPS, * m_pShadowPS, *m_pOmniDirShadowPS, *m_pGeometryPS, * m_pPBRGeometryPS,
			*m_pSSAOPS, *m_pSSAOBlurPS, *m_pDeferredPS, *m_pPBRDeferredPS, *m_pBlurPS, * m_pLightCubePS, *m_pOutlinerPS, *m_pFinPS;


		ID3D11InputLayout* m_pInputLayout, * m_pPosOnlyInputLayout, * m_pTexInputLayout, * m_pDeferredGeoLayout;

	};
}
#endif
