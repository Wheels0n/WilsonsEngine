#pragma once

#include <d3d12.h>
#include "typedef.h"

namespace wilson
{
	class Shader12
	{

	private:
	public:
		inline ID3D12RootSignature* GetAabbShaderRootSingnature()
		{
			return m_pAabbRootSignature.Get();
		}
		inline ID3D12RootSignature* GetBlurShaderRootSingnature()
		{
			return m_pBlurRootSignature.Get();
		}
		inline ID3D12RootSignature* GetBrdfRootSingnature()
		{
			return m_pBrdfRootSignature.Get();
		}
		inline ID3D12RootSignature* GetCascadeDirShadowRootSingnature()
		{
			return m_pCasacadePassRootSignature.Get();
		}
		inline ID3D12RootSignature* GetCubeShaderRootSingnature()
		{
			return m_pCubeRootsignature.Get();
		}
		inline ID3D12RootSignature* GetCubeShadowRootSingnature()
		{
			return m_pCubeShadowRootSignature.Get();
		}
		inline ID3D12RootSignature* GetDeferredLightingShaderRootSingnature()
		{
			return m_pLightRootSignature.Get();
		}
		inline ID3D12RootSignature* GetDeferredGeoShaderRootSingnature()
		{
			return m_pGeoRootSignature.Get();
		}
		inline ID3D12RootSignature* GetDiffuseIrradianceRootSingnature()
		{
			return m_pDiffuseIrradianceRootSignature.Get();
		}
		inline ID3D12RootSignature* GetDownSampleRootSignature()
		{
			return m_pDownSampleRootSignature.Get();
		}
		inline ID3D12RootSignature* GetEquirect2CubeRootSingnature()
		{
			return m_pEquirect2cubeRootSignature.Get();
		}
		inline ID3D12RootSignature* GetGenHiZpassRootSignature()
		{
			return m_pGenHiZpassRootSignature.Get();
		}
		inline ID3D12RootSignature* GetGenMipShaderRootSingnature()
		{
			return m_pGenMipMapRootsignature.Get();
		}
		inline ID3D12RootSignature* GetHiZCullPassRootSignature()
		{
			return m_pHiZCullPassRootSignature.Get();
		}
		inline ID3D12RootSignature* GetOutlinerTestShaderRootSingnature()
		{
			return m_pOutlinerTestRootSignature.Get();
		}
		inline ID3D12RootSignature* GetPbrDeferredGeoShaderRootSingnature()
		{
			return m_pPbrGeoRootSignature.Get();
		}
		inline ID3D12RootSignature* GetPbrDeferredLightingShaderRootSingnature()
		{
			return m_pPbrLightRootSignature.Get();
		}
		inline ID3D12RootSignature* GetPrefilterRootSingnature()
		{
			return m_pPrefilterRootSignature.Get();
		}
		inline ID3D12RootSignature* GetPostProcessShaderRootSingnature()
		{
			return m_pPostProcessRootSignature.Get();
		}
		inline ID3D12RootSignature* GetSkyBoxRootSingnature()
		{
			return m_pSkyboxRootSignature.Get();
		}
		inline ID3D12RootSignature* GetSpotShadowRootSingnature()
		{
			return m_pSpotShadowRootSignature.Get();
		}
		inline ID3D12RootSignature* GetSsaoShaderRootSingnature()
		{
			return m_pSsaoRootSignature.Get();
		}
		inline ID3D12RootSignature* GetSsaoBlurShaderRootSingnature()
		{
			return m_pSsaoBlurRootSignature.Get();
		}
		inline ID3D12RootSignature* GetZpassRootSignature()
		{
			return m_pZpassRootSignature.Get();
		}

		void SetAabbShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pAabbVs->GetBufferPointer(), m_pAabbVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pAabbPs->GetBufferPointer(), m_pAabbPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetBlurShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTexVs->GetBufferPointer(), m_pTexVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pBlurPs->GetBufferPointer(), m_pBlurPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetBrdfShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTexVs->GetBufferPointer(),m_pTexVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pBrdfPs->GetBufferPointer(),m_pBrdfPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetCascadeDirShadowShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pCascadeDirVs->GetBufferPointer(),m_pCascadeDirVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pCascadeDirGs->GetBufferPointer(),m_pCascadeDirGs->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pCascadeDirPs->GetBufferPointer(),m_pCascadeDirPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetCubeShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pMatrixTransformVs->GetBufferPointer(), m_pMatrixTransformVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pConstantPs->GetBufferPointer(), m_pConstantPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetCubeShadowShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pCubeShadowVs->GetBufferPointer(),m_pCubeShadowVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pCubeShadowGs->GetBufferPointer(),m_pCubeShadowGs->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pCubeShadowPs->GetBufferPointer(),m_pCubeShadowPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline void SetDeferredGeoLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			pPSODesc->InputLayout = m_deferredGeoLayout;
		}
		void SetDeferredGeoShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pGeometryVs->GetBufferPointer(),m_pGeometryVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pGeometryPs->GetBufferPointer(),m_pGeometryPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetDeferredLightingShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTexVs->GetBufferPointer(), m_pTexVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pDeferredPs->GetBufferPointer(),m_pDeferredPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetDiffuseIrradianceShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPosOnlyVs->GetBufferPointer(),m_pPosOnlyVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pEquirect2CubeGs->GetBufferPointer(),m_pEquirect2CubeGs->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pDiffuseIrradiancePs->GetBufferPointer(),m_pDiffuseIrradiancePs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetDownSampleShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTexVs->GetBufferPointer(), m_pTexVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pDownSamplePs->GetBufferPointer(),m_pDownSamplePs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetEquirect2CubeShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPosOnlyVs->GetBufferPointer(),m_pPosOnlyVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pEquirect2CubeGs->GetBufferPointer(),m_pEquirect2CubeGs->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pEquirect2CubePs->GetBufferPointer(),m_pEquirect2CubePs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetForwardShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pForwardVs->GetBufferPointer(),m_pForwardVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pForwardPs->GetBufferPointer(),m_pForwardPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetGenHiZpassShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTexVs->GetBufferPointer(), m_pTexVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pGenHiZPs->GetBufferPointer(), m_pGenHiZPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetGenMipShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = { m_pGenMipCs->GetBufferPointer(), m_pGenMipCs->GetBufferSize() };

			pPSODesc->CS = csBytecode;
		}
		void SetHiZCullPassShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = { m_pHiZCullCs->GetBufferPointer(), m_pHiZCullCs->GetBufferSize() };

			pPSODesc->CS = csBytecode;

		}
		void SetHWOcclusionQueryShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pMatrixTransformVs->GetBufferPointer(),m_pMatrixTransformVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { nullptr, 0 };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline void SetInputLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			pPSODesc->InputLayout = m_inputLayout;
		}
		void SetOutlinerTestShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{

			D3D12_SHADER_BYTECODE vsBytecode = { m_pMatrixTransformVs->GetBufferPointer(), m_pMatrixTransformVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pOutlinerTestPs->GetBufferPointer(), m_pOutlinerTestPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetPbrDeferredGeoShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometryVs->GetBufferPointer(),m_pPBRGeometryVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPbrGeometryPs->GetBufferPointer(),m_pPbrGeometryPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPbrDeferredGeoEmissiveShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometryVs->GetBufferPointer(),m_pPBRGeometryVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPbrGeometryEmissivePs->GetBufferPointer(),m_pPbrGeometryEmissivePs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPbrDeferredGeoNormalShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometryNormalVs->GetBufferPointer(),m_pPBRGeometryNormalVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPbrGeometryNormalPs->GetBufferPointer(),m_pPbrGeometryNormalPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPbrDeferredGeoNormalHeightShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometryNormalVs->GetBufferPointer(),m_pPBRGeometryNormalVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPbrGeometryNormalHeightPs->GetBufferPointer(),m_pPbrGeometryNormalHeightPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPbrDeferredGeoNormalHeightEmissiveShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometryNormalVs->GetBufferPointer(),m_pPBRGeometryNormalVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPbrGeometryNormalHeightEmissivePs->GetBufferPointer(),m_pPbrGeometryNormalHeightEmissivePs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPbrDeferredLightingShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTexVs->GetBufferPointer(), m_pTexVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPbrDeferredPs->GetBufferPointer(),m_pPbrDeferredPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		inline void SetPosOnlyInputLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			pPSODesc->InputLayout = m_posOnlyInputLayout;
		}
		void SetPostProcessShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = { m_pPostProcessCs->GetBufferPointer(), m_pPostProcessCs->GetBufferSize() };

			pPSODesc->CS = csBytecode;
		}
		void SetPrefilterShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPosOnlyVs->GetBufferPointer(),m_pPosOnlyVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pEquirect2CubeGs->GetBufferPointer(),m_pEquirect2CubeGs->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPrefilterPs->GetBufferPointer(),m_pPrefilterPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetSkyBoxShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pSkyBoxVs->GetBufferPointer(),m_pSkyBoxVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pSkyBoxPs->GetBufferPointer(),m_pSkyBoxPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetSpotShadowShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pShadowVs->GetBufferPointer(),m_pShadowVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pShadowPs->GetBufferPointer(),m_pShadowPs->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		void SetSsaoShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = { m_pSsaoCs->GetBufferPointer(), m_pSsaoCs->GetBufferSize() };

			pPSODesc->CS = csBytecode;

		}
		void SetSsaoBlurShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = { m_pSsaoBlurCs->GetBufferPointer(), m_pSsaoBlurCs->GetBufferSize() };

			pPSODesc->CS = csBytecode;

		}
		inline void SetTexInputlayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			pPSODesc->InputLayout = m_texInputLayout;
		}
		void SetZpassShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* const pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pMatrixTransformVs->GetBufferPointer(),m_pMatrixTransformVs->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { nullptr, 0 };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		
		Shader12(ID3D12Device* const pDevice);
		~Shader12();

	private:
		//D3D12
		Microsoft::WRL::ComPtr<ID3DBlob> m_pAabbVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pCascadeDirVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pCubeShadowVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pForwardVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pGeometryVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pMatrixTransformVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pMatrixTransformInstancedVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPBRGeometryVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPBRGeometryNormalVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPosOnlyVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pShadowVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pSkyBoxVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pSsaoVs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pTexVs;

		Microsoft::WRL::ComPtr<ID3DBlob> m_pCascadeDirGs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pCubeShadowGs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pEquirect2CubeGs;

		Microsoft::WRL::ComPtr<ID3DBlob> m_pAabbPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pBlurPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pBrdfPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pCascadeDirPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pConstantPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pCubeShadowPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pDeferredPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pDiffuseIrradiancePs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pDownSamplePs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pEquirect2CubePs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pForwardPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pGenHiZPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pGeometryPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pOutlinerTestPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPbrDeferredPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPbrGeometryPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPbrGeometryEmissivePs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPbrGeometryNormalPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPbrGeometryNormalHeightPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPbrGeometryNormalHeightEmissivePs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPostProcessPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPrefilterPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pShadowPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pSkyBoxPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pSsaoPs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pSsaoBlurPs;

		Microsoft::WRL::ComPtr<ID3DBlob> m_pGenMipCs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pHiZCullCs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pSsaoCs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pSsaoBlurCs;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pPostProcessCs;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pAabbRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pBlurRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pBrdfRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pCasacadePassRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pCubeRootsignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pCubeShadowRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pDiffuseIrradianceRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pDownSampleRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pEquirect2cubeRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pGenHiZpassRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pGenMipMapRootsignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pGeoRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pHiZCullPassRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pLightRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pOutlinerTestRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pPbrGeoRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pPbrLightRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pPostProcessRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pPrefilterRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pSkyboxRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pSpotShadowRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pSsaoRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pSsaoBlurRootSignature;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pZpassRootSignature;

		D3D12_INPUT_LAYOUT_DESC m_deferredGeoLayout;
		D3D12_INPUT_LAYOUT_DESC m_inputLayout;
		D3D12_INPUT_LAYOUT_DESC m_posOnlyInputLayout;
		D3D12_INPUT_LAYOUT_DESC m_texInputLayout;

		D3D12_INPUT_ELEMENT_DESC m_defaultElementDescs[8];
		D3D12_INPUT_ELEMENT_DESC m_deferredGeoElementDescs[4];
		D3D12_INPUT_ELEMENT_DESC m_posOnlyElementDesc;
		D3D12_INPUT_ELEMENT_DESC m_texElementDescs[2];

	};
}