#pragma once

#include <d3d11.h>
#include <DirectXTex.h>
#include "typedef.h"
namespace wilson
{
	class Shader11
	{

	private:
	public:
		inline void SetAabbShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pAabbVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pAabbPs.Get(), nullptr, 0);
		}
		inline void SetBlurShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pBlurPs.Get(), nullptr, 0);
		}
		inline void SetBrdfShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pBrdfPs.Get(), nullptr, 0);
		}
		inline void SetCascadeDirShadowShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pCascadeDirVs.Get(), nullptr, 0);
			pContext->GSSetShader(m_pCascadeDirGs.Get(), nullptr, 0);
			pContext->PSSetShader(m_pCascadeDirPs.Get(), nullptr, 0);
		}
		inline void SetCubeShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pMatrixTransformVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pConstantPs.Get(), nullptr, 0);
		}
		inline void SetCubeShadowShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pCubeShadowVs.Get(), nullptr, 0);
			pContext->GSSetShader(m_pCubeShadowGs.Get(), nullptr, 0);
			pContext->PSSetShader(m_pCubeShadowPs.Get(), nullptr, 0);
		}
		inline void SetDeferredGeoLayout(ID3D11DeviceContext* const pContext)
		{
			pContext->IASetInputLayout(m_pDeferredGeoLayout.Get());
		}
		inline void SetDeferredLightingShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pDeferredPs.Get(), nullptr, 0);
		}
		inline void SetDeferredGeoShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pGeometryVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pGeometryPs.Get(), nullptr, 0);
		}
		inline void SetDiffuseIrradianceShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVs.Get(), nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGs.Get(), nullptr, 0);
			pContext->PSSetShader(m_pDiffuseIrradiancePs.Get(), nullptr, 0);
		}
		inline void SetEquirect2CubeShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVs.Get(), nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGs.Get(), nullptr, 0);
			pContext->PSSetShader(m_pEquirect2CubePs.Get(), nullptr, 0);
		}
		inline void SetForwardShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPs.Get(), nullptr, 0);
		}
		inline void SetInputLayout(ID3D11DeviceContext*const pContext)
		{
			pContext->IASetInputLayout(m_pInputLayout.Get());
		}
		inline void SetOutlinerTestShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pMatrixTransformVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pOutlinerPs.Get(), nullptr, 0);
		}
		inline void SetPbrDeferredGeoShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pPbrGeometryVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPbrGeometryPs.Get(), nullptr, 0);
		}
		inline void SetPbrDeferredLightingShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPbrDeferredPs.Get(), nullptr, 0);
		}
		inline void SetPosOnlyInputLayout(ID3D11DeviceContext* const pContext)
		{
			pContext->IASetInputLayout(m_pPosOnlyInputLayout.Get());
		}
		inline void SetPostProcessShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pPostProcessPs.Get(), nullptr, 0);
		}
		inline void SetPrefilterShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pPosOnlyVs.Get(), nullptr, 0);
			pContext->GSSetShader(m_pEquirect2CubeGs.Get(), nullptr, 0);
			pContext->PSSetShader(m_pPrefilterPs.Get(), nullptr, 0);
		}
		inline void SetSkyBoxShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pSkyBoxVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSkyBoxPs.Get(), nullptr, 0);
		}
		inline void SetSsaoShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSsaoPs.Get(), nullptr, 0);
		}
		inline void SetSsaoBlurShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pTexVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pSsaoBlurPs.Get(), nullptr, 0);
		}
		inline void SetSpotShadowShader(ID3D11DeviceContext* const pContext)
		{
			pContext->VSSetShader(m_pShadowVs.Get(), nullptr, 0);
			pContext->GSSetShader(nullptr, nullptr, 0);
			pContext->PSSetShader(m_pShadowPs.Get(), nullptr, 0);
		}
		inline void SetTexInputlayout(ID3D11DeviceContext* const pContext)
		{
			pContext->IASetInputLayout(m_pTexInputLayout.Get());
		}

		Shader11(ID3D11Device* const pDevice, ID3D11DeviceContext* const pContext);
		~Shader11();

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pAabbVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pCascadeDirVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pCubeShadowVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pGeometryVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pMatrixTransformVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pPbrGeometryVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pPosOnlyVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pShadowVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pSkyBoxVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pTexVs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVs;

		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_pCascadeDirGs;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_pCubeShadowGs;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_pEquirect2CubeGs;
		
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pAabbPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pBlurPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pBrdfPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pCascadeDirPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pConstantPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pCubeShadowPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pDeferredPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pDiffuseIrradiancePs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pEquirect2CubePs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pGeometryPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pOutlinerPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPbrDeferredPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPbrGeometryPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPostProcessPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPrefilterPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pSsaoPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pSsaoBlurPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pShadowPs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pSkyBoxPs;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pDeferredGeoLayout;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pPosOnlyInputLayout;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pTexInputLayout;
		
	};
}