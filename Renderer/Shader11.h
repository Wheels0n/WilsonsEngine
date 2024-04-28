#pragma once

#include <d3d11.h>
#include <DirectXTex.h>
namespace wilson
{
	class Shader11
	{

	private:
	public:
		inline void SetAabbShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pAabbVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pAabbPs, nullptr, 0);
		}
		inline void SetBlurShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pBlurPs, nullptr, 0);
		}
		inline void SetBrdfShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pBrdfPs, nullptr, 0);
		}
		inline void SetCascadeDirShadowShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pCascadeDirVs, nullptr, 0);
			pContext->GSSetShader(m_pCascadeDirGs, nullptr, 0);
			pContext->PSSetShader(m_pCascadeDirPs, nullptr, 0);
		}
		inline void SetCubeShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pMatrixTransformVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pConstantPs, nullptr, 0);
		}
		inline void SetCubeShadowShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pCubeShadowVs, nullptr, 0);
			pContext->GSSetShader(m_pCubeShadowGs, nullptr, 0);
			pContext->PSSetShader(m_pCubeShadowPs, nullptr, 0);
		}
		inline void SetDeferredGeoLayout(ID3D11DeviceContext* const pContext)
		{
			pContext->IASetInputLayout(m_pDeferredGeoLayout);
		}
		inline void SetDeferredLightingShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pDeferredPs, nullptr, 0);
		}
		inline void SetDeferredGeoShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pGeometryVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pGeometryPs, nullptr, 0);
		}
		inline void SetDiffuseIrradianceShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVs, nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGs, nullptr, 0);
			pContext->PSSetShader(m_pDiffuseIrradiancePs, nullptr, 0);
		}
		inline void SetEquirect2CubeShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVs, nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGs, nullptr, 0);
			pContext->PSSetShader(m_pEquirect2CubePs, nullptr, 0);
		}
		inline void SetForwardShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPs, nullptr, 0);
		}
		inline void SetInputLayout(ID3D11DeviceContext*const pContext)
		{
			pContext->IASetInputLayout(m_pInputLayout);
		}
		inline void SetOutlinerTestShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pMatrixTransformVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pOutlinerPs, nullptr, 0);
		}
		inline void SetPbrDeferredGeoShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pPbrGeometryVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPbrGeometryPs, nullptr, 0);
		}
		inline void SetPbrDeferredLightingShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPbrDeferredPs, nullptr, 0);
		}
		inline void SetPosOnlyInputLayout(ID3D11DeviceContext* const pContext)
		{
			pContext->IASetInputLayout(m_pPosOnlyInputLayout);
		}
		inline void SetPostProcessShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPostProcessPs, nullptr, 0);
		}
		inline void SetPrefilterShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVs, nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGs, nullptr, 0);
			pContext->PSSetShader(m_pPrefilterPs, nullptr, 0);
		}
		inline void SetSkyBoxShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pSkyBoxVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSkyBoxPs, nullptr, 0);
		}
		inline void SetSsaoShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSsaoPs, nullptr, 0);
		}
		inline void SetSsaoBlurShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSsaoBlurPs, nullptr, 0);
		}
		inline void SetSpotShadowShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pShadowVs, nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pShadowPs, nullptr, 0);
		}
		inline void SetTexInputlayout(ID3D11DeviceContext* const pContext)
		{
			pContext->IASetInputLayout(m_pTexInputLayout);
		}

		Shader11(ID3D11Device* const pDevice, ID3D11DeviceContext* const pContext);
		~Shader11();

	private:
		ID3D11VertexShader* m_pAabbVs;
		ID3D11VertexShader* m_pCascadeDirVs;
		ID3D11VertexShader* m_pCubeShadowVs;
		ID3D11VertexShader* m_pGeometryVs;
		ID3D11VertexShader* m_pMatrixTransformVs;
		ID3D11VertexShader* m_pPbrGeometryVs;
		ID3D11VertexShader* m_pPosOnlyVs;
		ID3D11VertexShader* m_pShadowVs;
		ID3D11VertexShader* m_pSkyBoxVs;
		ID3D11VertexShader* m_pTexVs;
		ID3D11VertexShader* m_pVs;

		ID3D11GeometryShader* m_pCascadeDirGs;
		ID3D11GeometryShader* m_pCubeShadowGs;
		ID3D11GeometryShader* m_pEquirect2CubeGs;
		
		ID3D11PixelShader* m_pAabbPs;
		ID3D11PixelShader* m_pBlurPs;
		ID3D11PixelShader* m_pBrdfPs;
		ID3D11PixelShader* m_pCascadeDirPs;
		ID3D11PixelShader* m_pConstantPs;
		ID3D11PixelShader* m_pCubeShadowPs;
		ID3D11PixelShader* m_pDeferredPs;
		ID3D11PixelShader* m_pDiffuseIrradiancePs;
		ID3D11PixelShader* m_pEquirect2CubePs;
		ID3D11PixelShader* m_pGeometryPs;
		ID3D11PixelShader* m_pOutlinerPs;
		ID3D11PixelShader* m_pPbrDeferredPs;
		ID3D11PixelShader* m_pPbrGeometryPs;
		ID3D11PixelShader* m_pPostProcessPs;
		ID3D11PixelShader* m_pPrefilterPs;
		ID3D11PixelShader* m_pPs;
		ID3D11PixelShader* m_pSsaoPs;
		ID3D11PixelShader* m_pSsaoBlurPs;
		ID3D11PixelShader* m_pShadowPs;
		ID3D11PixelShader* m_pSkyBoxPs;

		ID3D11InputLayout* m_pDeferredGeoLayout;
		ID3D11InputLayout* m_pInputLayout;
		ID3D11InputLayout* m_pPosOnlyInputLayout;
		ID3D11InputLayout* m_pTexInputLayout;
		
	};
}