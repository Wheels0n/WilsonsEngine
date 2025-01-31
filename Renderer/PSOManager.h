#pragma once
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{
	using namespace Microsoft::WRL;


	class ShaderWrapper : public enable_shared_from_this<ShaderWrapper>
	{
	public:
		ComPtr<ID3D12RootSignature> GetRootSignature();
		D3D12_SHADER_BYTECODE GetVSByteCode();
		D3D12_SHADER_BYTECODE GetGSByteCode();
		D3D12_SHADER_BYTECODE GetPSByteCode();
		D3D12_SHADER_BYTECODE GetCSByteCode();
		D3D12_INPUT_LAYOUT_DESC GetInputLayout();

		ShaderWrapper(const wchar_t* vsPath, const wchar_t* gsPath, const wchar_t* psPath, const wchar_t* csPath,
			D3D12_INPUT_LAYOUT_DESC inputDesc,
			D3D12_DESCRIPTOR_RANGE_TYPE rangeType[], D3D12_SHADER_VISIBILITY visibility[], UINT nParameter);

	private:

		ComPtr<ID3D12RootSignature> rs;
		ComPtr<ID3DBlob>			vs;
		ComPtr<ID3DBlob>			gs;
		ComPtr<ID3DBlob>			ps;
		ComPtr<ID3DBlob>			cs;

		D3D12_INPUT_LAYOUT_DESC		InputDesc;
	};


	class Shader12;
	class PSOManager
	{
	private:
		void CreateGraphic(ComPtr<ID3D12PipelineState>&, shared_ptr<ShaderWrapper> pShader,
			D3D12_RASTERIZER_DESC, D3D12_BLEND_DESC, D3D12_DEPTH_STENCIL_DESC,
			UINT nRT, DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat);
		void CreateCompute(ComPtr<ID3D12PipelineState>&, shared_ptr<ShaderWrapper> pShader);
	public:
		
		void SetBrdf(ComPtr<ID3D12GraphicsCommandList>);
		void SetCascadeDirShadow(ComPtr<ID3D12GraphicsCommandList>);
		void SetCubeShadow(ComPtr<ID3D12GraphicsCommandList>);
		void SetCubeMipGen(ComPtr<ID3D12GraphicsCommandList>);
		void SetDownSample(ComPtr<ID3D12GraphicsCommandList>);
		void SetDiffuseIrrad(ComPtr<ID3D12GraphicsCommandList>);
		void SetEquirect2Cube(ComPtr<ID3D12GraphicsCommandList>);
		void SetHiZGen(ComPtr<ID3D12GraphicsCommandList>);
		void SetHiZCull(ComPtr<ID3D12GraphicsCommandList>);
		void SetHWOcclusion(ComPtr<ID3D12GraphicsCommandList>);
		void SetPbrDeferredGeo(ComPtr<ID3D12GraphicsCommandList>);
		void SetPbrDeferredGeoNormalHeightEmissive(ComPtr<ID3D12GraphicsCommandList>);
		void SetPbrDeferredGeoNormalHeight(ComPtr<ID3D12GraphicsCommandList>);
		void SetPbrDeferredGeoNormalEmissive(ComPtr<ID3D12GraphicsCommandList>);
		void SetPbrDeferredGeoNormal(ComPtr<ID3D12GraphicsCommandList>);
		void SetPbrDeferredGeoEmissive(ComPtr<ID3D12GraphicsCommandList>);
		void SetPbrDeferredLighting(ComPtr<ID3D12GraphicsCommandList>);
		void SetPostProcess(ComPtr<ID3D12GraphicsCommandList>);
		void SetPrefilter(ComPtr<ID3D12GraphicsCommandList>);
		void SetSkyBox(ComPtr<ID3D12GraphicsCommandList>);
		void SetSpotShadow(ComPtr<ID3D12GraphicsCommandList>);
		void SetSSAO(ComPtr<ID3D12GraphicsCommandList>);
		void SetSSAOBlur(ComPtr<ID3D12GraphicsCommandList>);
		void SetZpass(ComPtr<ID3D12GraphicsCommandList>);

		PSOManager();
	private:
		ComPtr<ID3D12PipelineState> m_pAabbPso;
		ComPtr<ID3D12PipelineState> m_pBrdfPso;
		ComPtr<ID3D12PipelineState> m_pCascadeDirShadowPso;
		ComPtr<ID3D12PipelineState> m_pCubeShadowPso;
		ComPtr<ID3D12PipelineState> m_pDownSamplePso;
		ComPtr<ID3D12PipelineState> m_pDiffuseIrradiancePso;
		ComPtr<ID3D12PipelineState> m_pEquirect2CubePso;
		ComPtr<ID3D12PipelineState> m_pGenHiZPassPso;
		ComPtr<ID3D12PipelineState> m_pGenMipmapPso;
		ComPtr<ID3D12PipelineState> m_pHiZCullPassPso;
		ComPtr<ID3D12PipelineState> m_pHWOcclusionQueryPso;
		ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoPso;
		ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoEmissivePso;
		ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoNormalPso;
		ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoNormalEmissivePso;
		ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoNormalHeightPso;
		ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoNormalHeightEmissivePso;
		ComPtr<ID3D12PipelineState> m_pPbrDeferredLightingPso;
		ComPtr<ID3D12PipelineState> m_pPostProcessPso;
		ComPtr<ID3D12PipelineState> m_pPrefilterPso;
		ComPtr<ID3D12PipelineState> m_pSkyBoxPso;
		ComPtr<ID3D12PipelineState> m_pSpotShadowPso;
		ComPtr<ID3D12PipelineState> m_pSsaoPso;
		ComPtr<ID3D12PipelineState> m_pSsaoBlurPso;
		ComPtr<ID3D12PipelineState> m_pZpassPso;

		shared_ptr<ShaderWrapper> m_pAabbShader;
		shared_ptr<ShaderWrapper> m_pBrdfShader;
		shared_ptr<ShaderWrapper> m_pCascadeDirShadowShader;
		shared_ptr<ShaderWrapper> m_pCubeShadowShader;
		shared_ptr<ShaderWrapper> m_pDownSampleShader;
		shared_ptr<ShaderWrapper> m_pDiffuseIrradianceShader;
		shared_ptr<ShaderWrapper> m_pEquirect2CubeShader;
		shared_ptr<ShaderWrapper> m_pGenHiZPassShader;
		shared_ptr<ShaderWrapper> m_pGenMipmapShader;
		shared_ptr<ShaderWrapper> m_pHiZCullPassShader;
		shared_ptr<ShaderWrapper> m_pHWOcclusionQueryShader;
		shared_ptr<ShaderWrapper> m_pPbrDeferredGeoShader;
		shared_ptr<ShaderWrapper> m_pPbrDeferredGeoEmissiveShader;
		shared_ptr<ShaderWrapper> m_pPbrDeferredGeoNormalShader;
		shared_ptr<ShaderWrapper> m_pPbrDeferredGeoNormalEmissiveShader;
		shared_ptr<ShaderWrapper> m_pPbrDeferredGeoNormalHeightShader;
		shared_ptr<ShaderWrapper> m_pPbrDeferredGeoNormalHeightEmissiveShader;
		shared_ptr<ShaderWrapper> m_pPbrDeferredLightingShader;
		shared_ptr<ShaderWrapper> m_pPostProcessShader;
		shared_ptr<ShaderWrapper> m_pPrefilterShader;
		shared_ptr<ShaderWrapper> m_pSkyBoxShader;
		shared_ptr<ShaderWrapper> m_pSpotShadowShader;
		shared_ptr<ShaderWrapper> m_pSsaoShader;
		shared_ptr<ShaderWrapper> m_pSsaoBlurShader;
		shared_ptr<ShaderWrapper> m_pZpassShader;
	};
}


