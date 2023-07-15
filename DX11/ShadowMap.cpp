#include "ShadowMap.h"
namespace wilson
{	
	bool ShadowMap::Init(ID3D11Device* pDevice, int width, int height,
		int dirLightCap, int pntLightCap)
	{
		m_dirDepthMaps.resize(dirLightCap);
		m_cubeDepthMaps.resize(pntLightCap);
		m_dirSRVs.resize(dirLightCap);
		m_cubeSRVs.resize(pntLightCap);
		m_dirDSVs.resize(dirLightCap);
		m_cubeDSVs.resize(pntLightCap);
		m_pDirShadowSamplerState = nullptr;
		m_pCubeShadowSamplerState = nullptr;

		HRESULT hr;
		FLOAT borderColor[4] = { 0.0f, };
		D3D11_TEXTURE2D_DESC texDesc = {};
		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc= {};
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		D3D11_SAMPLER_DESC samDesc = {};

		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = static_cast<float>(width);
		m_viewport.Height = static_cast<float>(height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;


		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL |
			D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		dsvDesc.Flags = 0;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		for (int i = 0; i < m_dirDepthMaps.size(); ++i)
		{
			hr = pDevice->CreateTexture2D(&texDesc, 0, &m_dirDepthMaps[i]);
			if (FAILED(hr))
			{
				return false;
			}


			hr = pDevice->CreateDepthStencilView(m_dirDepthMaps[i], &dsvDesc, &m_dirDSVs[i]);
			if (FAILED(hr))
			{
				return false;
			}

			hr = pDevice->CreateShaderResourceView(m_dirDepthMaps[i], &srvDesc, &m_dirSRVs[i]);
			if (FAILED(hr))
			{
				return false;
			}
		}

		samDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samDesc.BorderColor[0] = 1.0f;
		samDesc.BorderColor[1] = 1.0f;
		samDesc.BorderColor[2] = 1.0f;
		samDesc.BorderColor[3] = 1.0f;
		samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		samDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
		hr = pDevice->CreateSamplerState(&samDesc, &m_pDirShadowSamplerState);
		if (FAILED(hr))
		{
			return false;
		}
		samDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = pDevice->CreateSamplerState(&samDesc, &m_pCubeShadowSamplerState);
		if (FAILED(hr))
		{
			return false;
		}
		{	
			texDesc.ArraySize = 6;
			texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = 6;
			dsvDesc.Texture2DArray.FirstArraySlice = 0;
			dsvDesc.Texture2DArray.MipSlice = 0;


			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.Texture2DArray.ArraySize = 6;
			srvDesc.Texture2DArray.FirstArraySlice=0;
			srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			for (int i = 0; i < m_cubeDepthMaps.size(); ++i)
			{
				hr = pDevice->CreateTexture2D(&texDesc, 0, &m_cubeDepthMaps[i]);
				if (FAILED(hr))
				{
					return false;
				}

				hr = pDevice->CreateDepthStencilView(m_cubeDepthMaps[i], &dsvDesc, &m_cubeDSVs[i]);
				if (FAILED(hr))
				{
					return false;
				}

				hr = pDevice->CreateShaderResourceView(m_cubeDepthMaps[i], &srvDesc, &m_cubeSRVs[i]);
				if (FAILED(hr))
				{
					return false;
				}
			}
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			pDevice->CreateTexture2D(&texDesc, 0, &m_pTex);
			RTVDesc.Format = texDesc.Format;
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			RTVDesc.Texture2DArray.ArraySize = 6;
			RTVDesc.Texture2DArray.FirstArraySlice = 0;
			RTVDesc.Texture2DArray.MipSlice = 0;
			pDevice->CreateRenderTargetView(m_pTex, &RTVDesc, &m_pRTV);
		}

		return true;

	}

	void ShadowMap::BindDirDSV(ID3D11DeviceContext* pContext, UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { nullptr };
		pContext->OMSetRenderTargets(1, renderTargets, m_dirDSVs[i]);
		pContext->ClearDepthStencilView(m_dirDSVs[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	}

	void ShadowMap::BindCubeDSV(ID3D11DeviceContext* pContext, UINT i)
	{	
		FLOAT clear[4] = { 1, };
		ID3D11RenderTargetView* renderTargets[1] = { m_pRTV };
		pContext->OMSetRenderTargets(1, renderTargets, m_cubeDSVs[i]);
		pContext->ClearRenderTargetView(m_pRTV, clear);
		pContext->ClearDepthStencilView(m_cubeDSVs[i], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	}

	ShadowMap::~ShadowMap()
	{	
		for (int i = 0; i < m_dirDepthMaps.size(); ++i)
		{
			if (m_dirDepthMaps[i] != nullptr)
			{
				m_dirDepthMaps[i]->Release();
				m_dirDepthMaps[i] = nullptr;
			}

			if (m_dirDSVs[i] != nullptr)
			{
				m_dirDSVs[i]->Release();
				m_dirDSVs[i] = nullptr;
			}

			if (m_dirSRVs[i] != nullptr)
			{
				m_dirSRVs[i]->Release();
				m_dirSRVs[i] = nullptr;
			}
		}

		for (int i = 0; i < m_cubeDepthMaps.size(); ++i)
		{
			if (m_cubeDepthMaps[i] != nullptr)
			{
				m_cubeDepthMaps[i]->Release();
				m_cubeDepthMaps[i] = nullptr;
			}

			if (m_cubeDSVs[i] != nullptr)
			{
				m_cubeDSVs[i]->Release();
				m_cubeDSVs[i] = nullptr;
			}


			if (m_cubeSRVs[i] != nullptr)
			{
				m_cubeSRVs[i]->Release();
				m_cubeSRVs[i] = nullptr;
			}
		}
		if (m_pDirShadowSamplerState != nullptr)
		{
			m_pDirShadowSamplerState->Release();
			m_pDirShadowSamplerState = nullptr;
		}
		if (m_pCubeShadowSamplerState != nullptr)
		{
			m_pCubeShadowSamplerState->Release();
			m_pCubeShadowSamplerState = nullptr;
		}
		if (m_pRTV != nullptr)
		{
			m_pRTV->Release();
		}
		if (m_pTex != nullptr)
		{
			m_pTex->Release();
		}
		m_dirDepthMaps.clear();
		m_dirDSVs.clear();
		m_dirSRVs.clear();
		m_cubeDepthMaps.clear();
		m_cubeDSVs.clear();
		m_cubeSRVs.clear();
	}

}