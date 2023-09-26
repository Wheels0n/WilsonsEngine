#pragma once

#include <d3d11.h>
#include <vector>

namespace wilson
{
	class ShadowMap
	{

	public:
		bool Init(ID3D11Device* pDevice, const UINT width, const UINT height, const UINT cascadeLevel,
			const UINT dirLightCap, const UINT pntLightCap, const UINT spotLightCap);

		void BindDirDSV(ID3D11DeviceContext* pContext, const UINT i);
		void BindCubeDSV(ID3D11DeviceContext* pContext, const UINT i);
		void BindSpotDSV(ID3D11DeviceContext* pContext, const UINT i);
		inline ID3D11ShaderResourceView** GetDirSRV()
		{
			return &m_dirSRVs[0];
		}
		inline ID3D11ShaderResourceView** GetCubeSRV()
		{
			return &m_cubeSRVs[0];
		}
		inline ID3D11ShaderResourceView** GetSpotSRV()
		{
			return &m_spotSRVs[0];
		}
		inline ID3D11SamplerState** GetDirShadowSampler()
		{
			return &m_pDirShadowSamplerState;
		}
		inline ID3D11SamplerState** GetCubeShadowSampler()
		{
			return &m_pCubeShadowSamplerState;
		}
		inline D3D11_VIEWPORT* GetViewport()
		{
			return &m_viewport;
		}
		ShadowMap() = default;
		~ShadowMap();

	private:
		std::vector<ID3D11Texture2D*> m_dirTex;
		std::vector<ID3D11Texture2D*> m_cubeTex;
		std::vector<ID3D11Texture2D*> m_spotTex;

		std::vector<ID3D11ShaderResourceView*> m_dirSRVs;
		std::vector<ID3D11ShaderResourceView*> m_cubeSRVs;
		std::vector<ID3D11ShaderResourceView*> m_spotSRVs;

		std::vector<ID3D11DepthStencilView*> m_dirDSVs;
		std::vector<ID3D11DepthStencilView*> m_cubeDSVs;
		std::vector<ID3D11DepthStencilView*> m_spotDSVs;

		ID3D11RenderTargetView* m_pRTV;
		ID3D11Texture2D* m_pTex;

		ID3D11SamplerState* m_pDirShadowSamplerState;
		ID3D11SamplerState* m_pCubeShadowSamplerState;

		D3D11_VIEWPORT m_viewport;
	};
};