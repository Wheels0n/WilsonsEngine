#pragma once
#include <d3d12.h>
#include "typedef.h"

namespace wilson
{
	using namespace Microsoft::WRL;

	class TextureWrapper;
	class PSOManager;
	class Shader12;
	class PBRResourceManager
	{
	private:
		void UploadSkyBoxIA(ComPtr<ID3D12GraphicsCommandList>);

		bool CreateEquirentangularMap(const char* pPath);
		void ConvertEquirectagular2Cube(shared_ptr<PSOManager> psoManager);
		void CreateDiffuseIrradianceMap(shared_ptr<PSOManager> psoManager);
		void CreatePrefileterMap(shared_ptr<PSOManager> psoManager);
		void CreateMipMap(shared_ptr<PSOManager> psoManager);
		void CreateBrdfMap(shared_ptr<PSOManager> psoManager);

		void DestroyHdr();
	public:
		void Init(shared_ptr<PSOManager> psoManager);

		void SetSkyBoxPass(ComPtr<ID3D12GraphicsCommandList>);
		void SetLightingPass(ComPtr<ID3D12GraphicsCommandList> );

		void UpdateSkyBoxMat(shared_ptr<WVPMatrices> pWvpMatrices);

		PBRResourceManager(shared_ptr<PSOManager> psoManager);
		~PBRResourceManager();
	private:
		UINT m_equirect2CubeCbKey;
		UINT m_skyMatrixCbKey;
		UINT m_roughnessCbKey;
		UINT m_resolutionCbKey;

		D3D12_VIEWPORT			m_brdfViewport;
		D3D12_VIEWPORT			m_skyboxViewport;
		D3D12_VIEWPORT			m_diffIrradViewport;
		D3D12_VIEWPORT			m_prefilterViewport;
		D3D12_RECT				m_brdfRect;
		D3D12_RECT				m_skyboxRect;
		D3D12_RECT				m_diffIrradRect;
		D3D12_RECT				m_prefilterRect;

		D3D12_VERTEX_BUFFER_VIEW m_skyBoxVbv;
		D3D12_INDEX_BUFFER_VIEW  m_skyBoxIbv;

		shared_ptr<TextureWrapper>		m_pHdrTex;
		shared_ptr<TextureWrapper>		m_pSkyBoxTex;
		shared_ptr<TextureWrapper>		m_pDiffIrradianceTex;
		shared_ptr<TextureWrapper>		m_pPrefilterTex;
		shared_ptr<TextureWrapper>		m_pGenMipUavTex;
		shared_ptr<TextureWrapper>		m_pBrdfTex;
	};
}