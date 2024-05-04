#pragma once
#include "ShadowMap11.h"
namespace wilson
{	
	ShadowMap::ShadowMap(ID3D11Device* const pDevice, const UINT width, const UINT height, const UINT cascadeLevel,
		const UINT dirLightCap, const UINT pntLightCap, const UINT spotLightCap)
	{
		m_dirTexs.resize(dirLightCap);
		m_dirDebugTexs.resize(dirLightCap);
		m_dirDebugRtvs.resize(dirLightCap);

		m_cubeTexs.resize(pntLightCap);
		m_cubeDebugTexs.resize(pntLightCap);
		m_cubeDebugRtvs.resize(pntLightCap);

		m_spotTexs.resize(spotLightCap);
		m_spotDebugTexs.resize(spotLightCap);
		m_spotDebugRtvs.resize(spotLightCap);

		m_dirSrvs.resize(dirLightCap);
		m_dirDebugSrvs.resize(dirLightCap);

		m_cubeSrvs.resize(pntLightCap);
		m_cubeDebugSrvs.resize(pntLightCap);

		m_spotSrvs.resize(spotLightCap);
		m_spotDebugSrvs.resize(spotLightCap);

		m_debugTexs.resize(6);
		m_debugSrvs.resize(6);

		m_dirDsvs.resize(dirLightCap);
		m_cubeDsvs.resize(pntLightCap);
		m_spotDsvs.resize(spotLightCap);

		HRESULT hr;
		FLOAT borderColor[4] = { 0.0f, };
		D3D11_TEXTURE2D_DESC texDesc = {};
		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc= {};
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		D3D11_SAMPLER_DESC samDesc = {};

		m_width = width;
		m_height = height;

		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = static_cast<float>(width);
		m_viewport.Height = static_cast<float>(height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		//Gen shadowMapTex
		{
			//Gen DirShadowMap
			{
				texDesc.Width = width;
				texDesc.Height = height;
				texDesc.MipLevels = 1;
				texDesc.ArraySize = cascadeLevel;
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
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
				dsvDesc.Texture2DArray.MipSlice = 0;

				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
				srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2DArray.MostDetailedMip = 0;

				for (int i = 0; i < m_dirTexs.size(); ++i)
				{
					hr = pDevice->CreateTexture2D(&texDesc, 0, m_dirTexs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_dirTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_dirTexs[i]") - 1, "ShadowMap::m_dirTexs[i]");


					hr = pDevice->CreateDepthStencilView(m_dirTexs[i].Get(), &dsvDesc, m_dirDsvs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_dirDsvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_dirDsvs[i]") - 1, "ShadowMap::m_dirDsvs[i]");

					hr = pDevice->CreateShaderResourceView(m_dirTexs[i].Get(), &srvDesc, m_dirSrvs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_dirSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_dirSrvs[i]") - 1, "ShadowMap::m_dirSrvs[i]");
				}
			}
			

			//Gen SpotShadowMap
			{
				texDesc.ArraySize = 1;

				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Texture2D.MipSlice = 0;

				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				for (int i = 0; i < m_spotTexs.size(); ++i)
				{
					hr = pDevice->CreateTexture2D(&texDesc, 0, m_spotTexs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_spotTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_spotTexs[i]") - 1, "ShadowMap::m_spotTexs[i]");


					hr = pDevice->CreateDepthStencilView(m_spotTexs[i].Get(), &dsvDesc, m_spotDsvs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_spotDsvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_spotDsvs[i]") - 1, "ShadowMap::m_spotDsvs[i]");

					hr = pDevice->CreateShaderResourceView(m_spotTexs[i].Get(), &srvDesc, m_spotSrvs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_spotSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_spotSrvs[i]") - 1, "ShadowMap::m_spotSrvs[i]");
				}
			}
			


			//Gen CubeShadowMap
			{
				texDesc.ArraySize = 6;
				texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.ArraySize = 6;
				dsvDesc.Texture2DArray.FirstArraySlice = 0;
				dsvDesc.Texture2DArray.MipSlice = 0;


				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.Texture2DArray.ArraySize = 6;
				srvDesc.Texture2DArray.FirstArraySlice = 0;
				srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				for (int i = 0; i < m_cubeTexs.size(); ++i)
				{
					hr = pDevice->CreateTexture2D(&texDesc, 0, m_cubeTexs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_cubeTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_cubeTexs[i]") - 1, "ShadowMap::m_cubeTexs[i]");

					hr = pDevice->CreateDepthStencilView(m_cubeTexs[i].Get(), &dsvDesc, m_cubeDsvs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_cubeDsvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_cubeDsvs[i]") - 1, "ShadowMap::m_cubeDsvs[i]");

					hr = pDevice->CreateShaderResourceView(m_cubeTexs[i].Get(), &srvDesc, m_cubeSrvs[i].GetAddressOf());
					assert(SUCCEEDED(hr));
					m_cubeSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_cubeSrvs[i]") - 1, "ShadowMap::m_cubeSrvs[i]");
				}


			}
		}
		
		//Gen Debug RTV
		{
			texDesc.ArraySize = cascadeLevel;
			texDesc.MiscFlags = 0;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

			RTVDesc.Format = texDesc.Format;
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			RTVDesc.Texture2DArray.ArraySize = cascadeLevel;
			RTVDesc.Texture2DArray.FirstArraySlice = 0;
			RTVDesc.Texture2DArray.MipSlice = 0;

			srvDesc.Format = texDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = cascadeLevel;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			for (int i = 0; i < dirLightCap; ++i)
			{
				hr = pDevice->CreateTexture2D(&texDesc, 0, m_dirDebugTexs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_dirDebugTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_dirDebugTexs[i]") - 1, "ShadowMap::m_dirDebugTexs[i]");

				hr = pDevice->CreateRenderTargetView(m_dirDebugTexs[i].Get(), &RTVDesc, m_dirDebugRtvs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_dirDebugRtvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_dirDebugRtvs[i]") - 1, "ShadowMap::m_dirDebugRtvs[i]");


				hr = pDevice->CreateShaderResourceView(m_dirDebugTexs[i].Get(), &srvDesc, m_dirDebugSrvs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_dirDebugSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_dirDebugSrvs[i]") - 1, "ShadowMap::m_dirDebugSrvs[i]");

			}
			texDesc.ArraySize = 6;
			RTVDesc.Texture2DArray.ArraySize = 6;
			for (int i = 0; i < pntLightCap; ++i)
			{
				hr = pDevice->CreateTexture2D(&texDesc, 0, m_cubeDebugTexs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_cubeDebugTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_cubeDebugTexs[i]") - 1, "ShadowMap::m_cubeDebugTexs[i]");

				hr = pDevice->CreateRenderTargetView(m_cubeDebugTexs[i].Get(), &RTVDesc, m_cubeDebugRtvs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_cubeDebugRtvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_cubeDebugRtvs[i]") - 1, "ShadowMap::m_cubeDebugRtvs[i]");


				hr = pDevice->CreateShaderResourceView(m_cubeDebugTexs[i].Get(), &srvDesc, m_cubeDebugSrvs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_cubeDebugSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_cubeDebugSrvs[i]") - 1, "ShadowMap::m_cubeDebugSrvs[i]");

			}
			texDesc.ArraySize = 1;

			RTVDesc.Texture2DArray.ArraySize = 1;
			RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;

			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			for (int i = 0; i < spotLightCap; ++i)
			{
				hr = pDevice->CreateTexture2D(&texDesc, 0, m_spotDebugTexs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_spotDebugTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_spotDebugTexs[i]") - 1, "ShadowMap::m_spotDebugTexs[i]");


				hr = pDevice->CreateRenderTargetView(m_spotDebugTexs[i].Get(), &RTVDesc, m_spotDebugRtvs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_spotDebugRtvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_spotDebugRtvs[i]") - 1, "ShadowMap::m_spotDebugRtvs[i]");

				hr = pDevice->CreateShaderResourceView(m_spotDebugTexs[i].Get(), &srvDesc, m_spotDebugSrvs[i].GetAddressOf());
				assert(SUCCEEDED(hr));
				m_spotDebugSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_spotDebugSrvs[i]") - 1, "ShadowMap::m_spotDebugSrvs[i]");
			}
		}

		//Create Textures for Display
		for (int i = 0; i < 6; ++i)
		{
			hr = pDevice->CreateTexture2D(&texDesc, 0, m_debugTexs[i].GetAddressOf());
			assert(SUCCEEDED(hr));
			m_debugTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_debugTexs[i]") - 1, "ShadowMap::m_debugTexs[i]");

			hr = pDevice->CreateShaderResourceView(m_debugTexs[i].Get(), &srvDesc, m_debugSrvs[i].GetAddressOf());
			assert(SUCCEEDED(hr));
			m_debugSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_debugSrvs[i]") - 1, "ShadowMap::m_debugSrvs[i]");
		}

		//Gen SamplerStates;
		{
			samDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samDesc.BorderColor[0] = 1.0f;
			samDesc.BorderColor[1] = 1.0f;
			samDesc.BorderColor[2] = 1.0f;
			samDesc.BorderColor[3] = 1.0f;
			samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			samDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
			hr = pDevice->CreateSamplerState(&samDesc, m_pDirShadowSamplerState.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pDirShadowSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_pDirShadowSamplerState") - 1, "ShadowMap::m_pDirShadowSamplerState");


			samDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			hr = pDevice->CreateSamplerState(&samDesc, m_pCubeShadowSamplerState.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCubeShadowSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_pCubeShadowSamplerState") - 1, "ShadowMap::m_pCubeShadowSamplerState");
		}

	}

	void ShadowMap::BindDirDsv(ID3D11DeviceContext* const pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_dirDebugRtvs[i].Get() };
		pContext->ClearRenderTargetView(m_dirDebugRtvs[i].Get(), m_clear);
		pContext->ClearDepthStencilView(m_dirDsvs[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_dirDsvs[i].Get());
	}

	void ShadowMap::BindCubeDsv(ID3D11DeviceContext* const pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_cubeDebugRtvs[i].Get() };
		pContext->ClearRenderTargetView(m_cubeDebugRtvs[i].Get(), m_clear);
		pContext->ClearDepthStencilView(m_cubeDsvs[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_cubeDsvs[i].Get());
	}

	void ShadowMap::BindSpotDsv(ID3D11DeviceContext* const pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_spotDebugRtvs[i].Get() };
		pContext->ClearRenderTargetView(m_spotDebugRtvs[i].Get(), m_clear);
		pContext->ClearDepthStencilView(m_spotDsvs[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_spotDsvs[i].Get());
	}

	ID3D11ShaderResourceView* ShadowMap::GetDirDebugSrv(ID3D11DeviceContext* const pContext, const UINT i, const UINT lod)
	{	
		pContext->CopySubresourceRegion(m_debugTexs[lod].Get(), 0, 0, 0, 0,
			m_dirDebugTexs[i].Get(), lod, NULL);
		return m_debugSrvs[lod].Get();
	}

	ID3D11ShaderResourceView* ShadowMap::GetCubeDebugSrv(ID3D11DeviceContext* const pContext, const UINT i, const UINT face)
	{
		pContext->CopySubresourceRegion(m_debugTexs[face].Get(), 0, 0, 0, 0,
			m_cubeDebugTexs[i].Get(), face, NULL);
		return m_debugSrvs[face].Get();
	}

	ID3D11ShaderResourceView* ShadowMap::GetSpotDebugSrv(ID3D11DeviceContext* const pContext, const UINT i)
	{	
		pContext->CopySubresourceRegion(m_debugTexs[0].Get(), 0, 0, 0, 0,
			m_spotDebugTexs[i].Get(), 0, NULL);
		return m_debugSrvs[0].Get();
	}

	ShadowMap::~ShadowMap()
	{	
		m_dirTexs.clear();
		m_dirDsvs.clear();
		m_dirSrvs.clear();
		m_cubeTexs.clear();
		m_cubeDsvs.clear();
		m_cubeSrvs.clear();
		m_spotTexs.clear();
		m_spotDsvs.clear();
		m_spotSrvs.clear();
	}

}