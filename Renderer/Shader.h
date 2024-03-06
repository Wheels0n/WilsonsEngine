#pragma once

#include<d3d11.h>
#include <DirectXTex.h>
namespace wilson
{
	class Shader
	{

	private:
	public:
		inline void SetInputLayout(ID3D11DeviceContext* pContext)
		{
			pContext->IASetInputLayout(m_pInputLayout);
		}
		inline void SetPosOnlyInputLayout(ID3D11DeviceContext* pContext)
		{
			pContext->IASetInputLayout(m_pPosOnlyInputLayout);
		}
		inline void SetTexInputlayout(ID3D11DeviceContext* pContext)
		{
			pContext->IASetInputLayout(m_pTexInputLayout);
		}
		inline void SetDeferredGeoLayout(ID3D11DeviceContext* pContext)
		{
			pContext->IASetInputLayout(m_pDeferredGeoLayout);
		}



		inline void SetForwardShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPS, nullptr, 0);
		}
		inline void SetEquirect2CubeShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVS, nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGS, nullptr, 0);
			pContext->PSSetShader(m_pEquirect2CubePS, nullptr, 0);
		}
		inline void SetDiffuseIrradianceShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVS, nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGS, nullptr, 0);
			pContext->PSSetShader(m_pDiffuseIrradiancePS, nullptr, 0);
		}
		inline void SetPrefilterShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVS, nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGS, nullptr, 0);
			pContext->PSSetShader(m_pPrefilterPS, nullptr, 0);
		}
		inline void SetBRDFShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pTexVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pBRDFPS, nullptr, 0);
		}

		inline void SetSkyBoxShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pSkyBoxVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSkyBoxPS, nullptr, 0);
		}
		inline void SetCascadeDirShadowShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pCascadeDirVS, nullptr, 0);
			pContext->GSSetShader(m_pCascadeDirGS, nullptr, 0);
			pContext->PSSetShader(m_pCascadeDirPS, nullptr, 0);
		}
		inline void SetSpotShadowShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pShadowVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pShadowPS, nullptr, 0);
		}
		inline void SetOmniDirShadowShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pOmniDirShadowVS, nullptr, 0);
		    pContext->GSSetShader(m_pOmniDirShadowGS, nullptr, 0);
			pContext->PSSetShader(m_pOmniDirShadowPS, nullptr, 0);
		}
		inline void SetDeferredGeoShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pGeometryVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pGeometryPS, nullptr, 0);
		}
		inline void SetPBRDeferredGeoShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pPBRGeometryVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPBRGeometryPS, nullptr, 0);
		}
		inline void SetSSAOShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pTexVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSSAOPS, nullptr, 0);
		}
		inline void SetSSAOBlurShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pTexVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSSAOBlurPS, nullptr, 0);
		}
		inline void SetDeferredLightingShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pTexVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pDeferredPS, nullptr, 0);
		}
		inline void SetPBRDeferredLightingShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pTexVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPBRDeferredPS, nullptr, 0);
		}
		inline void SetCubeShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pMatrixTransformVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pConstantPS, nullptr, 0);
		}
		inline void SetAABBShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pAABBVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pAABBPS, nullptr, 0);
		}

		inline void SetBlurShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pTexVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pBlurPS, nullptr, 0);
		}
		inline void SetOutlinerTestShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pMatrixTransformVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pOutlinerPS, nullptr, 0);
		}
		inline void SetFinalShader(ID3D11DeviceContext* pContext)
		{
			pContext->VSSetShader(m_pTexVS, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pFinPS, nullptr, 0);
		}

		Shader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
		~Shader();

	private:

		ID3D11VertexShader* m_pVS;
		ID3D11VertexShader* m_pSkyBoxVS;
		ID3D11VertexShader* m_pShadowVS;
		ID3D11VertexShader* m_pOmniDirShadowVS;
		ID3D11VertexShader* m_pTexVS;
		ID3D11VertexShader* m_pGeometryVS;
		ID3D11VertexShader* m_pPBRGeometryVS;
		ID3D11VertexShader* m_pMatrixTransformVS;
		ID3D11VertexShader* m_pPosOnlyVS;
		ID3D11VertexShader* m_pAABBVS;
		ID3D11VertexShader* m_pCascadeDirVS;

		ID3D11GeometryShader* m_pCascadeDirGS;
		ID3D11GeometryShader* m_pOmniDirShadowGS;
		ID3D11GeometryShader* m_pEquirect2CubeGS;
		
		ID3D11PixelShader* m_pPS;
		ID3D11PixelShader* m_pCascadeDirPS;
		ID3D11PixelShader* m_pSkyBoxPS;
		ID3D11PixelShader* m_pShadowPS;
		ID3D11PixelShader* m_pOmniDirShadowPS;
		ID3D11PixelShader* m_pGeometryPS;
		ID3D11PixelShader* m_pPBRGeometryPS;
		ID3D11PixelShader* m_pEquirect2CubePS;
		ID3D11PixelShader* m_pDiffuseIrradiancePS;
		ID3D11PixelShader* m_pPrefilterPS;
		ID3D11PixelShader* m_pBRDFPS;
		ID3D11PixelShader* m_pSSAOPS;
		ID3D11PixelShader* m_pSSAOBlurPS;
		ID3D11PixelShader* m_pDeferredPS;
		ID3D11PixelShader* m_pPBRDeferredPS;
		ID3D11PixelShader* m_pBlurPS;
		ID3D11PixelShader* m_pConstantPS;
		ID3D11PixelShader* m_pOutlinerPS;
		ID3D11PixelShader* m_pFinPS;
		ID3D11PixelShader* m_pAABBPS;

		ID3D11InputLayout* m_pInputLayout;
		ID3D11InputLayout* m_pPosOnlyInputLayout;
		ID3D11InputLayout* m_pTexInputLayout;
		ID3D11InputLayout* m_pDeferredGeoLayout;
		
		
	};
}