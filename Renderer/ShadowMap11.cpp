#pragma once
#include "ShadowMap11.h"
namespace wilson
{	
	ShadowMap::ShadowMap(ID3D11Device* const pDevice, const UINT width, const UINT height, const UINT cascadeLevel,
		const UINT dirLightCap, const UINT pntLightCap, const UINT spotLightCap)
	{
		m_dirTex.resize(dirLightCap);
		m_dirDebugTex.resize(dirLightCap);
		m_dirDebugRtvs.resize(dirLightCap);

		m_cubeTex.resize(pntLightCap);
		m_cubeDebugTex.resize(pntLightCap);
		m_cubeDebugRtvs.resize(pntLightCap);

		m_spotTex.resize(spotLightCap);
		m_spotDebugTex.resize(spotLightCap);
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
	
		m_pDirShadowSamplerState = nullptr;
		m_pCubeShadowSamplerState = nullptr;
	

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

				for (int i = 0; i < m_dirTex.size(); ++i)
				{
					hr = pDevice->CreateTexture2D(&texDesc, 0, &m_dirTex[i]);
					assert(SUCCEEDED(hr));
					m_dirTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_dirTex[i]") - 1, "ShadowMap::m_dirTex[i]");


					hr = pDevice->CreateDepthStencilView(m_dirTex[i], &dsvDesc, &m_dirDsvs[i]);
					assert(SUCCEEDED(hr));
					m_dirDsvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_dirDsvs[i]") - 1, "ShadowMap::m_dirDsvs[i]");

					hr = pDevice->CreateShaderResourceView(m_dirTex[i], &srvDesc, &m_dirSrvs[i]);
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
				for (int i = 0; i < m_spotTex.size(); ++i)
				{
					hr = pDevice->CreateTexture2D(&texDesc, 0, &m_spotTex[i]);
					assert(SUCCEEDED(hr));
					m_spotTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_spotTex[i]") - 1, "ShadowMap::m_spotTex[i]");


					hr = pDevice->CreateDepthStencilView(m_spotTex[i], &dsvDesc, &m_spotDsvs[i]);
					assert(SUCCEEDED(hr));
					m_spotDsvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_spotDsvs[i]") - 1, "ShadowMap::m_spotDsvs[i]");

					hr = pDevice->CreateShaderResourceView(m_spotTex[i], &srvDesc, &m_spotSrvs[i]);
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
				for (int i = 0; i < m_cubeTex.size(); ++i)
				{
					hr = pDevice->CreateTexture2D(&texDesc, 0, &m_cubeTex[i]);
					assert(SUCCEEDED(hr));
					m_cubeTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_cubeTex[i]") - 1, "ShadowMap::m_cubeTex[i]");

					hr = pDevice->CreateDepthStencilView(m_cubeTex[i], &dsvDesc, &m_cubeDsvs[i]);
					assert(SUCCEEDED(hr));
					m_cubeDsvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_cubeDsvs[i]") - 1, "ShadowMap::m_cubeDsvs[i]");

					hr = pDevice->CreateShaderResourceView(m_cubeTex[i], &srvDesc, &m_cubeSrvs[i]);
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
				hr = pDevice->CreateTexture2D(&texDesc, 0, &m_dirDebugTex[i]);
				assert(SUCCEEDED(hr));
				m_dirDebugTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_dirDebugTex[i]") - 1, "ShadowMap::m_dirDebugTex[i]");

				hr = pDevice->CreateRenderTargetView(m_dirDebugTex[i], &RTVDesc, &m_dirDebugRtvs[i]);
				assert(SUCCEEDED(hr));
				m_dirDebugRtvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_dirDebugRtvs[i]") - 1, "ShadowMap::m_dirDebugRtvs[i]");


				hr = pDevice->CreateShaderResourceView(m_dirDebugTex[i], &srvDesc, &m_dirDebugSrvs[i]);
				assert(SUCCEEDED(hr));
				m_dirDebugSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_dirDebugSrvs[i]") - 1, "ShadowMap::m_dirDebugSrvs[i]");

			}
			texDesc.ArraySize = 6;
			RTVDesc.Texture2DArray.ArraySize = 6;
			for (int i = 0; i < pntLightCap; ++i)
			{
				hr = pDevice->CreateTexture2D(&texDesc, 0, &m_cubeDebugTex[i]);
				assert(SUCCEEDED(hr));
				m_cubeDebugTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_cubeDebugTex[i]") - 1, "ShadowMap::m_cubeDebugTex[i]");

				hr = pDevice->CreateRenderTargetView(m_cubeDebugTex[i], &RTVDesc, &m_cubeDebugRtvs[i]);
				assert(SUCCEEDED(hr));
				m_cubeDebugRtvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_cubeDebugRtvs[i]") - 1, "ShadowMap::m_cubeDebugRtvs[i]");


				hr = pDevice->CreateShaderResourceView(m_cubeDebugTex[i], &srvDesc, &m_cubeDebugSrvs[i]);
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
				hr = pDevice->CreateTexture2D(&texDesc, 0, &m_spotDebugTex[i]);
				assert(SUCCEEDED(hr));
				m_spotDebugTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_spotDebugTex[i]") - 1, "ShadowMap::m_spotDebugTex[i]");


				hr = pDevice->CreateRenderTargetView(m_spotDebugTex[i], &RTVDesc, &m_spotDebugRtvs[i]);
				assert(SUCCEEDED(hr));
				m_spotDebugRtvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_spotDebugRtvs[i]") - 1, "ShadowMap::m_spotDebugRtvs[i]");

				hr = pDevice->CreateShaderResourceView(m_spotDebugTex[i], &srvDesc, &m_spotDebugSrvs[i]);
				assert(SUCCEEDED(hr));
				m_spotDebugSrvs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_spotDebugSrvs[i]") - 1, "ShadowMap::m_spotDebugSrvs[i]");
			}
		}

		//Create Textures for Display
		for (int i = 0; i < 6; ++i)
		{
			hr = pDevice->CreateTexture2D(&texDesc, 0, &m_debugTexs[i]);
			assert(SUCCEEDED(hr));
			m_debugTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_debugTexs[i]") - 1, "ShadowMap::m_debugTexs[i]");

			hr = pDevice->CreateShaderResourceView(m_debugTexs[i], &srvDesc, &m_debugSrvs[i]);
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
			hr = pDevice->CreateSamplerState(&samDesc, &m_pDirShadowSamplerState);
			assert(SUCCEEDED(hr));
			m_pDirShadowSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_pDirShadowSamplerState") - 1, "ShadowMap::m_pDirShadowSamplerState");


			samDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			hr = pDevice->CreateSamplerState(&samDesc, &m_pCubeShadowSamplerState);
			assert(SUCCEEDED(hr));
			m_pCubeShadowSamplerState->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_pCubeShadowSamplerState") - 1, "ShadowMap::m_pCubeShadowSamplerState");
		}

	}

	void ShadowMap::BindDirDsv(ID3D11DeviceContext* const pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_dirDebugRtvs[i] };
		pContext->ClearRenderTargetView(m_dirDebugRtvs[i], m_clear);
		pContext->ClearDepthStencilView(m_dirDsvs[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_dirDsvs[i]);
	}

	void ShadowMap::BindCubeDsv(ID3D11DeviceContext* const pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_cubeDebugRtvs[i] };
		pContext->ClearRenderTargetView(m_cubeDebugRtvs[i], m_clear);
		pContext->ClearDepthStencilView(m_cubeDsvs[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_cubeDsvs[i]);
	}

	void ShadowMap::BindSpotDsv(ID3D11DeviceContext* const pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_spotDebugRtvs[i] };
		pContext->ClearRenderTargetView(m_spotDebugRtvs[i], m_clear);
		pContext->ClearDepthStencilView(m_spotDsvs[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_spotDsvs[i]);
	}

	ID3D11ShaderResourceView* ShadowMap::GetDirDebugSrv(ID3D11DeviceContext* const pContext, const UINT i, const UINT lod)
	{	
		pContext->CopySubresourceRegion(m_debugTexs[lod], 0, 0, 0, 0,
			m_dirDebugTex[i], lod, NULL);
		return m_debugSrvs[lod];
	}

	ID3D11ShaderResourceView* ShadowMap::GetCubeDebugSrv(ID3D11DeviceContext* const pContext, const UINT i, const UINT face)
	{
		pContext->CopySubresourceRegion(m_debugTexs[face], 0, 0, 0, 0,
			m_cubeDebugTex[i], face, NULL);
		return m_debugSrvs[face];
	}

	ID3D11ShaderResourceView* ShadowMap::GetSpotDebugSrv(ID3D11DeviceContext* const pContext, const UINT i)
	{	
		pContext->CopySubresourceRegion(m_debugTexs[0], 0, 0, 0, 0,
			m_spotDebugTex[i], 0, NULL);
		return m_debugSrvs[0];
	}

	ShadowMap::~ShadowMap()
	{	
		for (int i = 0; i < m_dirTex.size(); ++i)
		{
			if (m_dirTex[i] != nullptr)
			{
				m_dirTex[i]->Release();
				m_dirTex[i] = nullptr;
			}

			if (m_dirDebugTex[i] != nullptr)
			{
				m_dirDebugTex[i]->Release();
				m_dirDebugTex[i] = nullptr;
			}

			if (m_dirDsvs[i] != nullptr)
			{
				m_dirDsvs[i]->Release();
				m_dirDsvs[i] = nullptr;
			}

			if (m_dirDebugRtvs[i] != nullptr)
			{
				m_dirDebugRtvs[i]->Release();
				m_dirDebugRtvs[i] = nullptr;
			}


			if (m_dirSrvs[i] != nullptr)
			{
				m_dirSrvs[i]->Release();
				m_dirSrvs[i] = nullptr;
			}

		
			if (m_dirDebugSrvs[i] != nullptr)
			{
				m_dirDebugSrvs[i]->Release();
				m_dirDebugSrvs[i] = nullptr;
			}
			
		}

		for (int i = 0; i < m_cubeTex.size(); ++i)
		{
			if (m_cubeTex[i] != nullptr)
			{
				m_cubeTex[i]->Release();
				m_cubeTex[i] = nullptr;
			}
			if (m_cubeDebugTex[i] != nullptr)
			{
				m_cubeDebugTex[i]->Release();
				m_cubeDebugTex[i] = nullptr;
			}

			if (m_cubeDsvs[i] != nullptr)
			{
				m_cubeDsvs[i]->Release();
				m_cubeDsvs[i] = nullptr;
			}

			if (m_cubeDebugRtvs[i] != nullptr)
			{
				m_cubeDebugRtvs[i]->Release();
				m_cubeDebugRtvs[i] = nullptr;
			}

			if (m_cubeSrvs[i] != nullptr)
			{
				m_cubeSrvs[i]->Release();
				m_cubeSrvs[i] = nullptr;
			}

			
			if (m_cubeDebugSrvs[i] != nullptr)
			{
				m_cubeDebugSrvs[i]->Release();
				m_cubeDebugSrvs[i] = nullptr;
			}
			
		}

		for (int i = 0; i < m_spotTex.size(); ++i)
		{
			if (m_spotTex[i] != nullptr)
			{
				m_spotTex[i]->Release();
				m_spotTex[i] = nullptr;
			}

			if (m_spotDebugTex[i] != nullptr)
			{
				m_spotDebugTex[i]->Release();
				m_spotDebugTex[i] = nullptr;
			}

			if (m_spotDsvs[i] != nullptr)
			{
				m_spotDsvs[i]->Release();
				m_spotDsvs[i] = nullptr;
			}

			if (m_spotDebugRtvs[i] != nullptr)
			{
				m_spotDebugRtvs[i]->Release();
				m_spotDebugRtvs[i] = nullptr;
			}

			if (m_spotSrvs[i] != nullptr)
			{
				m_spotSrvs[i]->Release();
				m_spotSrvs[i] = nullptr;
			}

			if (m_spotDebugSrvs[i] != nullptr)
			{
				m_spotDebugSrvs[i]->Release();
				m_spotDebugSrvs[i] = nullptr;
			}
		}

		for (int i = 0; i < 6; ++i)
		{
			if (m_debugTexs[i]!=nullptr)
			{
				m_debugTexs[i]->Release();
				m_debugTexs[i] = nullptr;
			}

			if (m_debugSrvs[i] != nullptr)
			{
				m_debugSrvs[i]->Release();
				m_debugSrvs[i] = nullptr;
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


		m_dirTex.clear();
		m_dirDsvs.clear();
		m_dirSrvs.clear();
		m_cubeTex.clear();
		m_cubeDsvs.clear();
		m_cubeSrvs.clear();
		m_spotTex.clear();
		m_spotDsvs.clear();
		m_spotSrvs.clear();
	}

}