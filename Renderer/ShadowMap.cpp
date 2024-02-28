#pragma once
#include <cassert>
#include "ShadowMap.h"
namespace wilson
{	
	ShadowMap::ShadowMap(ID3D11Device* pDevice, const UINT width, const UINT height, const UINT cascadeLevel,
		const UINT dirLightCap, const UINT pntLightCap, const UINT spotLightCap)
	{
		m_dirTex.resize(dirLightCap);
		m_dirDebugTex.resize(dirLightCap);
		m_dirDebugRTVs.resize(dirLightCap);

		m_cubeTex.resize(pntLightCap);
		m_cubeDebugTex.resize(pntLightCap);
		m_cubeDebugRTVs.resize(pntLightCap);

		m_spotTex.resize(spotLightCap);
		m_spotDebugTex.resize(spotLightCap);
		m_spotDebugRTVs.resize(spotLightCap);

		m_dirSRVs.resize(dirLightCap);
		m_dirDebugSRVs.resize(dirLightCap);

		m_cubeSRVs.resize(pntLightCap);
		m_cubeDebugSRVs.resize(pntLightCap);

		m_spotSRVs.resize(spotLightCap);
		m_spotDebugSRVs.resize(spotLightCap);

		m_debugTexs.resize(6);
		m_debugSRVs.resize(6);

		m_dirDSVs.resize(dirLightCap);
		m_cubeDSVs.resize(pntLightCap);
		m_spotDSVs.resize(spotLightCap);
	
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


					hr = pDevice->CreateDepthStencilView(m_dirTex[i], &dsvDesc, &m_dirDSVs[i]);
					assert(SUCCEEDED(hr));
					m_dirDSVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_dirDSVs[i]") - 1, "ShadowMap::m_dirDSVs[i]");

					hr = pDevice->CreateShaderResourceView(m_dirTex[i], &srvDesc, &m_dirSRVs[i]);
					assert(SUCCEEDED(hr));
					m_dirSRVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_dirSRVs[i]") - 1, "ShadowMap::m_dirSRVs[i]");
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


					hr = pDevice->CreateDepthStencilView(m_spotTex[i], &dsvDesc, &m_spotDSVs[i]);
					assert(SUCCEEDED(hr));
					m_spotDSVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_spotDSVs[i]") - 1, "ShadowMap::m_spotDSVs[i]");

					hr = pDevice->CreateShaderResourceView(m_spotTex[i], &srvDesc, &m_spotSRVs[i]);
					assert(SUCCEEDED(hr));
					m_spotSRVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_spotSRVs[i]") - 1, "ShadowMap::m_spotSRVs[i]");
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

					hr = pDevice->CreateDepthStencilView(m_cubeTex[i], &dsvDesc, &m_cubeDSVs[i]);
					assert(SUCCEEDED(hr));
					m_cubeDSVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_cubeDSVs[i]") - 1, "ShadowMap::m_cubeDSVs[i]");

					hr = pDevice->CreateShaderResourceView(m_cubeTex[i], &srvDesc, &m_cubeSRVs[i]);
					assert(SUCCEEDED(hr));
					m_cubeSRVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap::m_cubeSRVs[i]") - 1, "ShadowMap::m_cubeSRVs[i]");
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

				hr = pDevice->CreateRenderTargetView(m_dirDebugTex[i], &RTVDesc, &m_dirDebugRTVs[i]);
				assert(SUCCEEDED(hr));
				m_dirDebugRTVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_dirDebugRTVs[i]") - 1, "ShadowMap::m_dirDebugRTVs[i]");


				hr = pDevice->CreateShaderResourceView(m_dirDebugTex[i], &srvDesc, &m_dirDebugSRVs[i]);
				assert(SUCCEEDED(hr));
				m_dirDebugSRVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_dirDebugSRVs[i]") - 1, "ShadowMap::m_dirDebugSRVs[i]");

			}
			texDesc.ArraySize = 6;
			RTVDesc.Texture2DArray.ArraySize = 6;
			for (int i = 0; i < pntLightCap; ++i)
			{
				hr = pDevice->CreateTexture2D(&texDesc, 0, &m_cubeDebugTex[i]);
				assert(SUCCEEDED(hr));
				m_cubeDebugTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_cubeDebugTex[i]") - 1, "ShadowMap::m_cubeDebugTex[i]");

				hr = pDevice->CreateRenderTargetView(m_cubeDebugTex[i], &RTVDesc, &m_cubeDebugRTVs[i]);
				assert(SUCCEEDED(hr));
				m_cubeDebugRTVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_cubeDebugRTVs[i]") - 1, "ShadowMap::m_cubeDebugRTVs[i]");


				hr = pDevice->CreateShaderResourceView(m_cubeDebugTex[i], &srvDesc, &m_cubeDebugSRVs[i]);
				assert(SUCCEEDED(hr));
				m_cubeDebugSRVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_cubeDebugSRVs[i]") - 1, "ShadowMap::m_cubeDebugSRVs[i]");

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


				hr = pDevice->CreateRenderTargetView(m_spotDebugTex[i], &RTVDesc, &m_spotDebugRTVs[i]);
				assert(SUCCEEDED(hr));
				m_spotDebugRTVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_spotDebugRTVs[i]") - 1, "ShadowMap::m_spotDebugRTVs[i]");

				hr = pDevice->CreateShaderResourceView(m_spotDebugTex[i], &srvDesc, &m_spotDebugSRVs[i]);
				assert(SUCCEEDED(hr));
				m_spotDebugSRVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("ShadowMap::m_spotDebugSRVs[i]") - 1, "ShadowMap::m_spotDebugSRVs[i]");
			}
		}

		//Create Textures for Display
		for (int i = 0; i < 6; ++i)
		{
			hr = pDevice->CreateTexture2D(&texDesc, 0, &m_debugTexs[i]);
			assert(SUCCEEDED(hr));
			m_debugTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_debugTexs[i]") - 1, "ShadowMap::m_debugTexs[i]");

			hr = pDevice->CreateShaderResourceView(m_debugTexs[i], &srvDesc, &m_debugSRVs[i]);
			assert(SUCCEEDED(hr));
			m_debugSRVs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ShadowMap::m_debugSRVs[i]") - 1, "ShadowMap::m_debugSRVs[i]");
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

	void ShadowMap::BindDirDSV(ID3D11DeviceContext* pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_dirDebugRTVs[i] };
		pContext->ClearRenderTargetView(m_dirDebugRTVs[i], m_clear);
		pContext->ClearDepthStencilView(m_dirDSVs[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_dirDSVs[i]);
	}

	void ShadowMap::BindCubeDSV(ID3D11DeviceContext* pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_cubeDebugRTVs[i] };
		pContext->ClearRenderTargetView(m_cubeDebugRTVs[i], m_clear);
		pContext->ClearDepthStencilView(m_cubeDSVs[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_cubeDSVs[i]);
	}

	void ShadowMap::BindSpotDSV(ID3D11DeviceContext* pContext, const UINT i)
	{	
		ID3D11RenderTargetView* renderTargets[1] = { m_spotDebugRTVs[i] };
		pContext->ClearRenderTargetView(m_spotDebugRTVs[i], m_clear);
		pContext->ClearDepthStencilView(m_spotDSVs[i], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
		pContext->OMSetRenderTargets(1, renderTargets, m_spotDSVs[i]);
	}

	ID3D11ShaderResourceView* ShadowMap::GetDirDebugSRV(ID3D11DeviceContext* pContext, UINT i, UINT lod)
	{	
		pContext->CopySubresourceRegion(m_debugTexs[lod], 0, 0, 0, 0,
			m_dirDebugTex[i], lod, NULL);
		return m_debugSRVs[lod];
	}

	ID3D11ShaderResourceView* ShadowMap::GetCubeDebugSRV(ID3D11DeviceContext* pContext, UINT i, UINT face)
	{
		pContext->CopySubresourceRegion(m_debugTexs[face], 0, 0, 0, 0,
			m_cubeDebugTex[i], face, NULL);
		return m_debugSRVs[face];
	}

	ID3D11ShaderResourceView* ShadowMap::GetSpotDebugSRV(ID3D11DeviceContext* pContext, UINT i)
	{	
		pContext->CopySubresourceRegion(m_debugTexs[0], 0, 0, 0, 0,
			m_spotDebugTex[i], 0, NULL);
		return m_debugSRVs[0];
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

			if (m_dirDSVs[i] != nullptr)
			{
				m_dirDSVs[i]->Release();
				m_dirDSVs[i] = nullptr;
			}

			if (m_dirDebugRTVs[i] != nullptr)
			{
				m_dirDebugRTVs[i]->Release();
				m_dirDebugRTVs[i] = nullptr;
			}


			if (m_dirSRVs[i] != nullptr)
			{
				m_dirSRVs[i]->Release();
				m_dirSRVs[i] = nullptr;
			}

		
			if (m_dirDebugSRVs[i] != nullptr)
			{
				m_dirDebugSRVs[i]->Release();
				m_dirDebugSRVs[i] = nullptr;
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

			if (m_cubeDSVs[i] != nullptr)
			{
				m_cubeDSVs[i]->Release();
				m_cubeDSVs[i] = nullptr;
			}

			if (m_cubeDebugRTVs[i] != nullptr)
			{
				m_cubeDebugRTVs[i]->Release();
				m_cubeDebugRTVs[i] = nullptr;
			}

			if (m_cubeSRVs[i] != nullptr)
			{
				m_cubeSRVs[i]->Release();
				m_cubeSRVs[i] = nullptr;
			}

			
			if (m_cubeDebugSRVs[i] != nullptr)
			{
				m_cubeDebugSRVs[i]->Release();
				m_cubeDebugSRVs[i] = nullptr;
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

			if (m_spotDSVs[i] != nullptr)
			{
				m_spotDSVs[i]->Release();
				m_spotDSVs[i] = nullptr;
			}

			if (m_spotDebugRTVs[i] != nullptr)
			{
				m_spotDebugRTVs[i]->Release();
				m_spotDebugRTVs[i] = nullptr;
			}

			if (m_spotSRVs[i] != nullptr)
			{
				m_spotSRVs[i]->Release();
				m_spotSRVs[i] = nullptr;
			}

			if (m_spotDebugSRVs[i] != nullptr)
			{
				m_spotDebugSRVs[i]->Release();
				m_spotDebugSRVs[i] = nullptr;
			}
		}

		for (int i = 0; i < 6; ++i)
		{
			if (m_debugTexs[i]!=nullptr)
			{
				m_debugTexs[i]->Release();
				m_debugTexs[i] = nullptr;
			}

			if (m_debugSRVs[i] != nullptr)
			{
				m_debugSRVs[i]->Release();
				m_debugSRVs[i] = nullptr;
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
		m_dirDSVs.clear();
		m_dirSRVs.clear();
		m_cubeTex.clear();
		m_cubeDSVs.clear();
		m_cubeSRVs.clear();
		m_spotTex.clear();
		m_spotDSVs.clear();
		m_spotSRVs.clear();
	}

}