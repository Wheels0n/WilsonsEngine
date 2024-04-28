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
			return m_pAabbRootSignature;
		}
		inline ID3D12RootSignature* GetBlurShaderRootSingnature()
		{
			return m_pBlurRootSignature;
		}
		inline ID3D12RootSignature* GetBrdfRootSingnature()
		{
			return m_pBrdfRootSignature;
		}
		inline ID3D12RootSignature* GetCascadeDirShadowRootSingnature()
		{
			return m_pCasacadePassRootSignature;
		}
		inline ID3D12RootSignature* GetCubeShaderRootSingnature()
		{
			return m_pCubeRootsignature;
		}
		inline ID3D12RootSignature* GetCubeShadowRootSingnature()
		{
			return m_pCubeShadowRootSignature;
		}
		inline ID3D12RootSignature* GetDeferredLightingShaderRootSingnature()
		{
			return m_pLightRootSignature;
		}
		inline ID3D12RootSignature* GetDeferredGeoShaderRootSingnature()
		{
			return m_pGeoRootSignature;
		}
		inline ID3D12RootSignature* GetDiffuseIrradianceRootSingnature()
		{
			return m_pDiffuseIrradianceRootSignature;
		}
		inline ID3D12RootSignature* GetDownSampleRootSignature()
		{
			return m_pDownSampleRootSignature;
		}
		inline ID3D12RootSignature* GetEquirect2CubeRootSingnature()
		{
			return m_pEquirect2cubeRootSignature;
		}
		inline ID3D12RootSignature* GetGenHiZpassRootSignature()
		{
			return m_pGenHiZpassRootSignature;
		}
		inline ID3D12RootSignature* GetGenMipShaderRootSingnature()
		{
			return m_pGenMipMapRootsignature;
		}
		inline ID3D12RootSignature* GetHiZCullPassRootSignature()
		{
			return m_pHiZCullPassRootSignature;
		}
		inline ID3D12RootSignature* GetOutlinerTestShaderRootSingnature()
		{
			return m_pOutlinerTestRootSignature;
		}
		inline ID3D12RootSignature* GetPbrDeferredGeoShaderRootSingnature()
		{
			return m_pPbrGeoRootSignature;
		}
		inline ID3D12RootSignature* GetPbrDeferredLightingShaderRootSingnature()
		{
			return m_pPbrLightRootSignature;
		}
		inline ID3D12RootSignature* GetPrefilterRootSingnature()
		{
			return m_pPrefilterRootSignature;
		}
		inline ID3D12RootSignature* GetPostProcessShaderRootSingnature()
		{
			return m_pPostProcessRootSignature;
		}
		inline ID3D12RootSignature* GetSkyBoxRootSingnature()
		{
			return m_pSkyboxRootSignature;
		}
		inline ID3D12RootSignature* GetSpotShadowRootSingnature()
		{
			return m_pSpotShadowRootSignature;
		}
		inline ID3D12RootSignature* GetSsaoShaderRootSingnature()
		{
			return m_pSsaoRootSignature;
		}
		inline ID3D12RootSignature* GetSsaoBlurShaderRootSingnature()
		{
			return m_pSsaoBlurRootSignature;
		}
		inline ID3D12RootSignature* GetZpassRootSignature()
		{
			return m_pZpassRootSignature;
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
			D3D12_SHADER_BYTECODE csBytecode = { m_pGenMipCS->GetBufferPointer(), m_pGenMipCS->GetBufferSize() };

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
		ID3DBlob* m_pAabbVs;
		ID3DBlob* m_pCascadeDirVs;
		ID3DBlob* m_pCubeShadowVs;
		ID3DBlob* m_pForwardVs;
		ID3DBlob* m_pGeometryVs;
		ID3DBlob* m_pMatrixTransformVs;
		ID3DBlob* m_pMatrixTransformInstancedVs;
		ID3DBlob* m_pPBRGeometryVs;
		ID3DBlob* m_pPBRGeometryNormalVs;
		ID3DBlob* m_pPosOnlyVs;
		ID3DBlob* m_pShadowVs;
		ID3DBlob* m_pSkyBoxVs;
		ID3DBlob* m_pSsaoVs;
		ID3DBlob* m_pTexVs;

		ID3DBlob* m_pCascadeDirGs;
		ID3DBlob* m_pCubeShadowGs;
		ID3DBlob* m_pEquirect2CubeGs;

		ID3DBlob* m_pAabbPs;
		ID3DBlob* m_pBlurPs;
		ID3DBlob* m_pBrdfPs;
		ID3DBlob* m_pCascadeDirPs;
		ID3DBlob* m_pConstantPs;
		ID3DBlob* m_pCubeShadowPs;
		ID3DBlob* m_pDeferredPs;
		ID3DBlob* m_pDiffuseIrradiancePs;
		ID3DBlob* m_pDownSamplePs;
		ID3DBlob* m_pEquirect2CubePs;
		ID3DBlob* m_pForwardPs;
		ID3DBlob* m_pGenHiZPs;
		ID3DBlob* m_pGeometryPs;
		ID3DBlob* m_pOutlinerTestPs;
		ID3DBlob* m_pPbrDeferredPs;
		ID3DBlob* m_pPbrGeometryPs;
		ID3DBlob* m_pPbrGeometryEmissivePs;
		ID3DBlob* m_pPbrGeometryNormalPs;
		ID3DBlob* m_pPbrGeometryNormalHeightPs;
		ID3DBlob* m_pPbrGeometryNormalHeightEmissivePs;
		ID3DBlob* m_pPostProcessPs;
		ID3DBlob* m_pPrefilterPs;
		ID3DBlob* m_pShadowPs;
		ID3DBlob* m_pSkyBoxPs;
		ID3DBlob* m_pSsaoPs;
		ID3DBlob* m_pSsaoBlurPs;

		ID3DBlob* m_pGenMipCS;
		ID3DBlob* m_pHiZCullCs;
		ID3DBlob* m_pSsaoCs;
		ID3DBlob* m_pSsaoBlurCs;
		ID3DBlob* m_pPostProcessCs;

		ID3D12RootSignature* m_pAabbRootSignature;
		ID3D12RootSignature* m_pBlurRootSignature;
		ID3D12RootSignature* m_pBrdfRootSignature;
		ID3D12RootSignature* m_pCasacadePassRootSignature;
		ID3D12RootSignature* m_pCubeRootsignature;
		ID3D12RootSignature* m_pCubeShadowRootSignature;
		ID3D12RootSignature* m_pDiffuseIrradianceRootSignature;
		ID3D12RootSignature* m_pDownSampleRootSignature;
		ID3D12RootSignature* m_pEquirect2cubeRootSignature;
		ID3D12RootSignature* m_pGenHiZpassRootSignature;
		ID3D12RootSignature* m_pGenMipMapRootsignature;
		ID3D12RootSignature* m_pGeoRootSignature;
		ID3D12RootSignature* m_pHiZCullPassRootSignature;
		ID3D12RootSignature* m_pLightRootSignature;
		ID3D12RootSignature* m_pOutlinerTestRootSignature;
		ID3D12RootSignature* m_pPbrGeoRootSignature;
		ID3D12RootSignature* m_pPbrLightRootSignature;
		ID3D12RootSignature* m_pPostProcessRootSignature;
		ID3D12RootSignature* m_pPrefilterRootSignature;
		ID3D12RootSignature* m_pSkyboxRootSignature;
		ID3D12RootSignature* m_pSpotShadowRootSignature;
		ID3D12RootSignature* m_pSsaoRootSignature;
		ID3D12RootSignature* m_pSsaoBlurRootSignature;
		ID3D12RootSignature* m_pZpassRootSignature;

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