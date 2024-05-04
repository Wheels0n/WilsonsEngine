#pragma once

#include <d3d11.h>
#include "typedef.h"
namespace wilson
{
	class ShadowMap
	{

	public:
		void BindCubeDsv(ID3D11DeviceContext* const pContext, const UINT i);
		void BindDirDsv(ID3D11DeviceContext* const pContext, const UINT i);
		void BindSpotDsv(ID3D11DeviceContext* const pContext, const UINT i);
		inline ID3D11ShaderResourceView** GetCubeSrv()
		{
			return m_cubeSrvs[0].GetAddressOf();
		}
		ID3D11ShaderResourceView* GetCubeDebugSrv(ID3D11DeviceContext* const pContext, const UINT i, const UINT face);
		inline ID3D11ShaderResourceView** GetDirSrv()
		{
			return m_dirSrvs[0].GetAddressOf();
		}
		ID3D11ShaderResourceView* GetDirDebugSrv(ID3D11DeviceContext* const pContext, const UINT i, const UINT lod);
		inline ID3D11ShaderResourceView** GetSpotSrv()
		{
			return m_spotSrvs[0].GetAddressOf();
		}
		ID3D11ShaderResourceView* GetSpotDebugSrv(ID3D11DeviceContext* const pContext, const UINT i);
		inline ID3D11SamplerState** GetCubeShadowSampler()
		{
			return m_pCubeShadowSamplerState.GetAddressOf();
		}
		inline ID3D11SamplerState** GetDirShadowSampler()
		{
			return m_pDirShadowSamplerState.GetAddressOf();
		}
		inline D3D11_VIEWPORT* GetViewport()
		{
			return &m_viewport;
		}

		ShadowMap() = default;
		ShadowMap(ID3D11Device* const pDevice, const UINT width, const UINT height, const UINT cascadeLevel,
			const UINT dirLightCap, const UINT pntLightCap, const UINT spotLightCap);
		~ShadowMap();

	private:
		UINT m_width;
		UINT m_height;
		FLOAT m_clear[4] = { 1.0f,1.0f,1.0f,1.0f };

		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_cubeTexs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_cubeDebugTexs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_debugTexs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_dirTexs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_dirDebugTexs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_spotTexs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> m_spotDebugTexs;

		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_cubeDebugRtvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_dirDebugRtvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> m_spotDebugRtvs;

		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_cubeSrvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_cubeDebugSrvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_debugSrvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_dirSrvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_dirDebugSrvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_spotSrvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_spotDebugSrvs;

		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_cubeDsvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_dirDsvs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_spotDsvs;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pDirShadowSamplerState;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pCubeShadowSamplerState;

		D3D11_VIEWPORT m_viewport;

	};
};