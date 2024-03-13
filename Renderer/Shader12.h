#pragma once

#include <d3d12.h>
#include "typedef.h"

namespace wilson
{
	class Shader12
	{

	private:
	public:

		inline void SetInputLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			pPSODesc->InputLayout = m_inputLayout;
		}
		inline void SetPosOnlyInputLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			pPSODesc->InputLayout = m_posOnlyInputLayout;
		}
		inline void SetTexInputlayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			pPSODesc->InputLayout = m_texInputLayout;
		}
		inline void SetDeferredGeoLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			pPSODesc->InputLayout = m_deferredGeoLayout;
		}

		//D3D12 Shader Setter
		void SetForwardShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pForwardVS->GetBufferPointer(),m_pForwardVS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pForwardPS->GetBufferPointer(),m_pForwardPS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		
		void SetZpassShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pMatrixTransformVS->GetBufferPointer(),m_pMatrixTransformVS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { nullptr, 0 };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}

		inline ID3D12RootSignature* GetZpassRootSignature()
		{
			return m_pZpassRootSignature;
		}

		void SetEquirect2CubeShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPosOnly12VS->GetBufferPointer(),m_pPosOnly12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pEquirect2Cube12GS->GetBufferPointer(),m_pEquirect2Cube12GS->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pEquirect2Cube12PS->GetBufferPointer(),m_pEquirect2Cube12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		inline ID3D12RootSignature* GetEquirect2CubeRootSingnature()
		{
			return m_pEquirect2cubeRootSignature;
		}


		void SetDiffuseIrradianceShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPosOnly12VS->GetBufferPointer(),m_pPosOnly12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pEquirect2Cube12GS->GetBufferPointer(),m_pEquirect2Cube12GS->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pDiffuseIrradiance12PS->GetBufferPointer(),m_pDiffuseIrradiance12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetDiffuseIrradianceRootSingnature()
		{
			return m_pDiffuseIrradianceRootSignature;
		}

		void SetPrefilterShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPosOnly12VS->GetBufferPointer(),m_pPosOnly12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pEquirect2Cube12GS->GetBufferPointer(),m_pEquirect2Cube12GS->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPrefilter12PS->GetBufferPointer(),m_pPrefilter12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetPrefilterRootSingnature()
		{
			return m_pPrefilterRootSignature;
		}

		void SetBRDFShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTex12VS->GetBufferPointer(),m_pTex12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pBRDF12PS->GetBufferPointer(),m_pBRDF12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetBRDFRootSingnature()
		{
			return m_pBrdfRootSignature;
		}

		void SetSkyBoxShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pSkyBox12VS->GetBufferPointer(),m_pSkyBox12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pSkyBox12PS->GetBufferPointer(),m_pSkyBox12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetSkyBoxRootSingnature()
		{
			return m_pSkyboxRootSignature;
		}

		void SetCascadeDirShadowShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pCascadeDir12VS->GetBufferPointer(),m_pCascadeDir12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pCascadeDir12GS->GetBufferPointer(),m_pCascadeDir12GS->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pCascadeDir12PS->GetBufferPointer(),m_pCascadeDir12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetCascadeDirShadowRootSingnature()
		{
			return m_pCasacadePassRootSignature;
		}

		void SetSpotShadowShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pShadow12VS->GetBufferPointer(),m_pShadow12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pShadow12PS->GetBufferPointer(),m_pShadow12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetSpotShadowRootSingnature()
		{
			return m_pSpotShadowRootSignature;
		}

		void SetCubeShadowShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pCubeShadow12VS->GetBufferPointer(),m_pCubeShadow12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { m_pCubeShadow12GS->GetBufferPointer(),m_pCubeShadow12GS->GetBufferSize() };
			D3D12_SHADER_BYTECODE psBytecode = { m_pCubeShadow12PS->GetBufferPointer(),m_pCubeShadow12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetCubeShadowRootSingnature()
		{
			return m_pCubeShadowRootSignature;
		}

		void SetDeferredGeoShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pGeometry12VS->GetBufferPointer(),m_pGeometry12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pGeometry12PS->GetBufferPointer(),m_pGeometry12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetDeferredGeoShaderRootSingnature()
		{
			return m_pGeoRootSignature;
		}
		void SetPBRDeferredGeoShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometry12VS->GetBufferPointer(),m_pPBRGeometry12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPBRGeometry12PS->GetBufferPointer(),m_pPBRGeometry12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPBRDeferredGeoEmissiveShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometry12VS->GetBufferPointer(),m_pPBRGeometry12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPBRGeometryEmissive12PS->GetBufferPointer(),m_pPBRGeometryEmissive12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPBRDeferredGeoNormalShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometryNormal12VS->GetBufferPointer(),m_pPBRGeometryNormal12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPBRGeometryNormal12PS->GetBufferPointer(),m_pPBRGeometryNormal12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPBRDeferredGeoNormalHeightShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometryNormal12VS->GetBufferPointer(),m_pPBRGeometryNormal12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPBRGeometryNormalHeight12PS->GetBufferPointer(),m_pPBRGeometryNormalHeight12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		void SetPBRDeferredGeoNormalHeightEmissiveShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pPBRGeometryNormal12VS->GetBufferPointer(),m_pPBRGeometryNormal12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPBRGeometryNormalHeightEmissive12PS->GetBufferPointer(),m_pPBRGeometryNormalHeightEmissive12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		inline ID3D12RootSignature* GetPBRDeferredGeoShaderRootSingnature()
		{
			return m_pPBRGeoRootSignature;
		}

		void SetSSAOShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = { m_pSSAOCS->GetBufferPointer(), m_pSSAOCS->GetBufferSize() };

			pPSODesc->CS = csBytecode;
			
		}
		inline ID3D12RootSignature* GetSSAOShaderRootSingnature()
		{
			return m_pSSAORootSignature;
		}

		void SetSSAOBlurShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = { m_pSSAOBlurCS->GetBufferPointer(), m_pSSAOBlurCS->GetBufferSize() };

			pPSODesc->CS = csBytecode;

		}
		inline ID3D12RootSignature* GetSSAOBlurShaderRootSingnature()
		{
			return m_pSSAOBlurRootSignature;
		}

		void SetDeferredLightingShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTex12VS->GetBufferPointer(), m_pTex12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pDeferred12PS->GetBufferPointer(),m_pDeferred12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetDeferredLightingShaderRootSingnature()
		{
			return m_pLightRootSignature;
		}
		void SetPBRDeferredLightingShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTex12VS->GetBufferPointer(), m_pTex12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pPBRDeferred12PS->GetBufferPointer(),m_pPBRDeferred12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		inline ID3D12RootSignature* GetPBRDeferredLightingShaderRootSingnature()
		{
			return m_pPBRLightRootSignature;
		}


		void SetCubeShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pMatrixTransformVS->GetBufferPointer(), m_pMatrixTransformVS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pConstantPS->GetBufferPointer(), m_pConstantPS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		inline ID3D12RootSignature* GetCubeShaderRootSingnature()
		{
			return m_pCubeRootsignature;
		}

		void SetAABBShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pAABB12VS->GetBufferPointer(), m_pAABB12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pAABB12PS->GetBufferPointer(), m_pAABB12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetAABBShaderRootSingnature()
		{
			return m_pAABBRootSignature;
		}

		void SetBlurShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE vsBytecode = { m_pTex12VS->GetBufferPointer(), m_pTex12VS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pBlur12PS->GetBufferPointer(), m_pBlur12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;

		}
		inline ID3D12RootSignature* GetBlurShaderRootSingnature()
		{
			return m_pBlurRootSignature;
		}

		void SetOutlinerTestShader(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc)
		{

			D3D12_SHADER_BYTECODE vsBytecode = { m_pMatrixTransformVS->GetBufferPointer(), m_pMatrixTransformVS->GetBufferSize() };
			D3D12_SHADER_BYTECODE gsBytecode = { nullptr, 0 };
			D3D12_SHADER_BYTECODE psBytecode = { m_pOutlinerTest12PS->GetBufferPointer(), m_pOutlinerTest12PS->GetBufferSize() };

			pPSODesc->VS = vsBytecode;
			pPSODesc->GS = gsBytecode;
			pPSODesc->PS = psBytecode;
		}
		inline ID3D12RootSignature* GetOutlinerTestShaderRootSingnature()
		{
			return m_pOutlinerTestRootSignature;
		}

		void SetPostProcessShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = { m_pPostProcessCS->GetBufferPointer(), m_pPostProcessCS->GetBufferSize() };

			pPSODesc->CS = csBytecode;
		}
		inline ID3D12RootSignature* GetPostProcessShaderRootSingnature()
		{
			return m_pPostProcessRootSignature;
		}

		void SetGenMipShader(D3D12_COMPUTE_PIPELINE_STATE_DESC* pPSODesc)
		{
			D3D12_SHADER_BYTECODE csBytecode = {m_pGenMipCS->GetBufferPointer(), m_pGenMipCS->GetBufferSize() };

			pPSODesc->CS = csBytecode;
		}
		inline ID3D12RootSignature* GetGenMipShaderRootSingnature()
		{
			return m_pGenMipMapRootsignature;
		}

		Shader12(ID3D12Device* pDevice);
		~Shader12();

	private:
		//D3D12

		ID3DBlob* m_pForwardVS;
		ID3DBlob* m_pSSAO12VS;
		ID3DBlob* m_pSkyBox12VS;
		ID3DBlob* m_pShadow12VS;
		ID3DBlob* m_pCubeShadow12VS;
		ID3DBlob* m_pTex12VS;
		ID3DBlob* m_pGeometry12VS;
		ID3DBlob* m_pPBRGeometry12VS;
		ID3DBlob* m_pPBRGeometryNormal12VS;
		ID3DBlob* m_pMatrixTransformVS;
		ID3DBlob* m_pPosOnly12VS;
		ID3DBlob* m_pAABB12VS;
		ID3DBlob* m_pCascadeDir12VS;

		ID3DBlob* m_pCascadeDir12GS;
		ID3DBlob* m_pCubeShadow12GS;
		ID3DBlob* m_pEquirect2Cube12GS;

		ID3DBlob* m_pForwardPS;
		ID3DBlob* m_pCascadeDir12PS;
		ID3DBlob* m_pSkyBox12PS;
		ID3DBlob* m_pShadow12PS;
		ID3DBlob* m_pCubeShadow12PS;
		ID3DBlob* m_pGeometry12PS;
		ID3DBlob* m_pPBRGeometry12PS;
		ID3DBlob* m_pPBRGeometryEmissive12PS;
		ID3DBlob* m_pPBRGeometryNormal12PS;
		ID3DBlob* m_pPBRGeometryNormalHeight12PS;
		ID3DBlob* m_pPBRGeometryNormalHeightEmissive12PS;
		ID3DBlob* m_pEquirect2Cube12PS;
		ID3DBlob* m_pDiffuseIrradiance12PS;
		ID3DBlob* m_pPrefilter12PS;
		ID3DBlob* m_pBRDF12PS;
		ID3DBlob* m_pSSAO12PS;
		ID3DBlob* m_pSSAOBlur12PS;
		ID3DBlob* m_pDeferred12PS;
		ID3DBlob* m_pPBRDeferred12PS;
		ID3DBlob* m_pBlur12PS;
		ID3DBlob* m_pConstantPS;
		ID3DBlob* m_pOutlinerTest12PS;
		ID3DBlob* m_pPostProcess12PS;
		ID3DBlob* m_pAABB12PS;

		ID3DBlob* m_pGenMipCS;
		ID3DBlob* m_pSSAOCS;
		ID3DBlob* m_pSSAOBlurCS;
		ID3DBlob* m_pPostProcessCS;

		ID3D12RootSignature* m_pZpassRootSignature;
		ID3D12RootSignature* m_pCasacadePassRootSignature;
		ID3D12RootSignature* m_pSpotShadowRootSignature;
		ID3D12RootSignature* m_pCubeShadowRootSignature;
		ID3D12RootSignature* m_pSkyboxRootSignature;
		ID3D12RootSignature* m_pPBRGeoRootSignature;
		ID3D12RootSignature* m_pSSAORootSignature;
		ID3D12RootSignature* m_pSSAOBlurRootSignature;
		ID3D12RootSignature* m_pPBRLightRootSignature;
		ID3D12RootSignature* m_pOutlinerTestRootSignature;
		ID3D12RootSignature* m_pPostProcessRootSignature;
		//
		ID3D12RootSignature* m_pPrefilterRootSignature;
		ID3D12RootSignature* m_pBrdfRootSignature;
		ID3D12RootSignature* m_pDiffuseIrradianceRootSignature;
		ID3D12RootSignature* m_pEquirect2cubeRootSignature;
		ID3D12RootSignature* m_pBlurRootSignature;
		ID3D12RootSignature* m_pAABBRootSignature;
		ID3D12RootSignature* m_pCubeRootsignature;
		ID3D12RootSignature* m_pGeoRootSignature;
		ID3D12RootSignature* m_pLightRootSignature;

		ID3D12RootSignature* m_pGenMipMapRootsignature;

		D3D12_INPUT_LAYOUT_DESC m_inputLayout;
		D3D12_INPUT_LAYOUT_DESC m_posOnlyInputLayout;
		D3D12_INPUT_LAYOUT_DESC m_texInputLayout;
		D3D12_INPUT_LAYOUT_DESC m_deferredGeoLayout;
		D3D12_INPUT_ELEMENT_DESC m_defaultElementDescs[8];
		D3D12_INPUT_ELEMENT_DESC m_posOnlyElementDesc;
		D3D12_INPUT_ELEMENT_DESC m_texElementDescs[2];
		D3D12_INPUT_ELEMENT_DESC m_deferredGeoElementDescs[4];

	};
}