#include "LightBuffer11.h"

namespace wilson
{
	void LightBuffer11::UpdateDirLightMatrices(ID3D11DeviceContext* const pContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		DirLightMatrices* pMatrices;
		HRESULT hr = pContext->Map(m_pDirLitMatricesCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		pMatrices = reinterpret_cast<DirLightMatrices*>(mappedResource.pData);
		pMatrices->dirCnt = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{	
			std::vector<DirectX::XMMATRIX> cascadeMat = m_pDirLights[i]->GetLightSpaceMat();
			for (int j = 0; j < cascadeMat.size(); ++j)
			{
				pMatrices->matrices[i][j] = cascadeMat[j];
			}
			
		}
		pContext->Unmap(m_pDirLitMatricesCb.Get(), 0);
		pContext->PSSetConstantBuffers(3, 1, m_pDirLitMatricesCb.GetAddressOf());
	}

	void LightBuffer11::UpdateSpotLightMatrices(ID3D11DeviceContext* const pContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		SpotLightMatrices* pMatrices;
		HRESULT hr = pContext->Map(m_pSpotLitMatricesCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		pMatrices = reinterpret_cast<SpotLightMatrices*>(mappedResource.pData);
		pMatrices->spotCnt = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			pMatrices->matrices[i] = *(m_pSpotLights[i]->GetLightSpaceMat());
		}
		pContext->Unmap(m_pSpotLitMatricesCb.Get(), 0);
		pContext->PSSetConstantBuffers(4, 1, m_pSpotLitMatricesCb.GetAddressOf());
	}
	
	void LightBuffer11::UpdateLightBuffer(ID3D11DeviceContext* const pContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		LightBufferProperty* plbProperty;
		HRESULT hr = pContext->Map(m_pLightPropertyCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		plbProperty = reinterpret_cast<LightBufferProperty*>(mappedResource.pData);
		plbProperty->dirCnt = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			plbProperty->dirLights[i] = *m_pDirLights[i]->GetProperty(); 
		}
		
		plbProperty->pntCnt = m_pCubeLights.size();
		for (int i = 0; i < m_pCubeLights.size(); ++i)
		{
			plbProperty->pntLights[i] = * m_pCubeLights[i]->GetProperty();
		}
		plbProperty->sptCnt = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			plbProperty->sptLights[i] = *m_pSpotLights[i]->GetProperty(); 
		}
		pContext->Unmap(m_pLightPropertyCb.Get(),0);
		pContext->PSSetConstantBuffers(2, 1, m_pLightPropertyCb.GetAddressOf());
		
		
	}
	
	LightBuffer11::LightBuffer11(ID3D11Device* const pDevice)
	{	
		
		D3D11_BUFFER_DESC cbufferDesc;
		cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbufferDesc.ByteWidth = sizeof(LightBufferProperty);
		cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbufferDesc.MiscFlags = 0;
		cbufferDesc.StructureByteStride = 0;
		pDevice->CreateBuffer(&cbufferDesc, 0, m_pLightPropertyCb.GetAddressOf());
		m_pLightPropertyCb->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("LightBuffer11::m_pLightPropertyCb") - 1, "LightBuffer11::m_pLightPropertyCb");


		cbufferDesc.ByteWidth = sizeof(DirLightMatrices);
		pDevice->CreateBuffer(&cbufferDesc, 0, m_pDirLitMatricesCb.GetAddressOf());
		m_pDirLitMatricesCb->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("LightBuffer11::m_pDirLitMatricesCb") - 1, "LightBuffer11::m_pDirLitMatricesCb");

		cbufferDesc.ByteWidth = sizeof(SpotLightMatrices);
		pDevice->CreateBuffer(&cbufferDesc, 0, m_pSpotLitMatricesCb.GetAddressOf());
		m_pSpotLitMatricesCb->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("LightBuffer11::m_pSpotLitMatricesCb") - 1, "LightBuffer11::m_pSpotLitMatricesCb");

		m_pDirLights.reserve(_MAX_DIR_LIGHTS);
		m_pCubeLights.reserve(_MAX_PNT_LIGHTS);
		m_pSpotLights.reserve(_MAX_SPT_LIGHTS);

		m_pNullSrvs.resize(_MAX_DIR_LIGHTS + _MAX_PNT_LIGHTS + _MAX_SPT_LIGHTS+4, nullptr);
	

	}

	LightBuffer11::~LightBuffer11()
	{	

		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			if (m_pDirLights[i] != nullptr)
			{
				delete m_pDirLights[i];
			}
		}
		for (int i = 0; i < m_pCubeLights.size(); ++i)
		{
			if (m_pCubeLights[i] != nullptr)
			{
				delete m_pCubeLights[i];
			}
		}
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			if (m_pSpotLights[i] != nullptr)
			{
				delete m_pSpotLights[i];
			}
		}
	}
}