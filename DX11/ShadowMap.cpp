#include "ShadowMap.h"

bool ShadowMap::Init(ID3D11Device* pDevice, int width, int height)
{	
	m_SRV = nullptr;
	m_DSV = nullptr;
	m_pSamplerState = nullptr;

	HRESULT hr;
	FLOAT borderColor[4] = { 0.0f, };
	D3D11_TEXTURE2D_DESC texDesc = {};
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

	hr = pDevice->CreateTexture2D(&texDesc, 0, &m_depthMap);
	if (FAILED(hr))
	{
		return false;
	}

	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = pDevice->CreateDepthStencilView(m_depthMap, &dsvDesc, &m_DSV);
	if (FAILED(hr))
	{
		return false;
	}

	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = pDevice->CreateShaderResourceView(m_depthMap, &srvDesc, &m_SRV);
	if (FAILED(hr))
	{
		return false;
	}

	samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samDesc.BorderColor[0] = 0.0f;
	samDesc.BorderColor[1] = 0.0f;
	samDesc.BorderColor[2] = 0.0f;
	samDesc.BorderColor[3] = 0.0f;
	samDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	hr = pDevice->CreateSamplerState(&samDesc, &m_pSamplerState);
	if (FAILED(hr))
	{
		return false;
	}

	return true;

}

void ShadowMap::BindDSV(ID3D11DeviceContext* pContext)
{
	SetNullRenderTarget(pContext);
	pContext->ClearDepthStencilView(m_DSV, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
}

ShadowMap::~ShadowMap()
{	
	if (m_depthMap != nullptr)
	{
		m_depthMap->Release();
		m_depthMap = nullptr;
	}

	if (m_DSV != nullptr)
	{
		m_DSV->Release();
		m_DSV = nullptr;
	}

	if (m_SRV != nullptr)
	{
		m_SRV->Release();
		m_SRV = nullptr;
	}

	if (m_pSamplerState != nullptr)
	{
		m_pSamplerState->Release();
		m_pSamplerState = nullptr;
	}
}

void ShadowMap::SetNullRenderTarget(ID3D11DeviceContext* pContext)
{	
	ID3D11RenderTargetView* renderTargets[1] = {nullptr };
	pContext->OMSetRenderTargets(1, renderTargets, m_DSV);
}
