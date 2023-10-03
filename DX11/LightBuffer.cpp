#include "LightBuffer.h"

namespace wilson
{
	void LightBuffer::UpdateDirLightMatrices(ID3D11DeviceContext* pContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		DirLightMatrices* pMatrices;
		HRESULT hr = pContext->Map(m_pDirLitMatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			return;
		}
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
		pContext->Unmap(m_pDirLitMatricesBuffer, 0);
		pContext->PSSetConstantBuffers(3, 1, &m_pDirLitMatricesBuffer);
	}
	void LightBuffer::UpdateSpotLightMatrices(ID3D11DeviceContext* pContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		SpotLightMatrices* pMatrices;
		HRESULT hr = pContext->Map(m_pSpotLitMatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			return;
		}
		pMatrices = reinterpret_cast<SpotLightMatrices*>(mappedResource.pData);
		pMatrices->spotCnt = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			pMatrices->matrices[i] = *(m_pSpotLights[i]->GetLightSpaceMat());
		}
		pContext->Unmap(m_pSpotLitMatricesBuffer, 0);
		pContext->PSSetConstantBuffers(4, 1, &m_pSpotLitMatricesBuffer);
	}
	void LightBuffer::UpdateLightBuffer(ID3D11DeviceContext* pContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		LightBufferProperty* plbProperty;
		HRESULT hr = pContext->Map(m_pLightPropertyBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			return;
		}
		plbProperty = reinterpret_cast<LightBufferProperty*>(mappedResource.pData);
		plbProperty->dirCnt = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			plbProperty->dirLights[i] = *m_pDirLights[i]->GetProperty(); 
		}
		
		plbProperty->pntCnt = m_pPointLights.size();
		for (int i = 0; i < m_pPointLights.size(); ++i)
		{
			plbProperty->pntLights[i] = * m_pPointLights[i]->GetProperty();
		}
		plbProperty->sptCnt = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			plbProperty->sptLights[i] = *m_pSpotLights[i]->GetProperty(); 
		}
		pContext->Unmap(m_pLightPropertyBuffer,0);
		pContext->PSSetConstantBuffers(2, 1, &m_pLightPropertyBuffer);
		
		
	}
	LightBuffer::LightBuffer(ID3D11Device* pDevice)
	{	
		m_pLightPropertyBuffer = nullptr;
		m_pDirLitMatricesBuffer = nullptr;
		m_pSpotLitMatricesBuffer = nullptr;

		D3D11_BUFFER_DESC lighCBD;
		lighCBD.Usage = D3D11_USAGE_DYNAMIC;
		lighCBD.ByteWidth = sizeof(LightBufferProperty);
		lighCBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lighCBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		lighCBD.MiscFlags = 0;
		lighCBD.StructureByteStride = 0;
		pDevice->CreateBuffer(&lighCBD, 0, &m_pLightPropertyBuffer);
		m_pLightPropertyBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("LightBuffer::m_pLightPropertyBuffer") - 1, "LightBuffer::m_pLightPropertyBuffer");


		lighCBD.ByteWidth = sizeof(DirLightMatrices);
		pDevice->CreateBuffer(&lighCBD, 0, &m_pDirLitMatricesBuffer);
		m_pDirLitMatricesBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("LightBuffer::m_pDirLitMatricesBuffer") - 1, "LightBuffer::m_pDirLitMatricesBuffer");

		lighCBD.ByteWidth = sizeof(SpotLightMatrices);
		pDevice->CreateBuffer(&lighCBD, 0, &m_pSpotLitMatricesBuffer);
		m_pSpotLitMatricesBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("LightBuffer::m_pSpotLitMatricesBuffer") - 1, "LightBuffer::m_pSpotLitMatricesBuffer");

		m_pDirLights.reserve(_MAX_DIR_LIGHTS);
		m_pPointLights.reserve(_MAX_PNT_LIGHTS);
		m_pSpotLights.reserve(_MAX_SPT_LIGHTS);

		m_pNullSRVs.resize(_MAX_DIR_LIGHTS + _MAX_PNT_LIGHTS + _MAX_SPT_LIGHTS+4, nullptr);
	

	}
	LightBuffer::~LightBuffer()
	{	
		if (m_pLightPropertyBuffer != nullptr)
		{
			m_pLightPropertyBuffer->Release();
			m_pLightPropertyBuffer = nullptr;
		}

		if (m_pDirLitMatricesBuffer != nullptr)
		{
			m_pDirLitMatricesBuffer->Release();
			m_pDirLitMatricesBuffer = nullptr;
		}

		if (m_pSpotLitMatricesBuffer != nullptr)
		{
			m_pSpotLitMatricesBuffer->Release();
			m_pSpotLitMatricesBuffer = nullptr;
		}

		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			if (m_pDirLights[i] != nullptr)
			{
				delete m_pDirLights[i];
			}
		}
		for (int i = 0; i < m_pPointLights.size(); ++i)
		{
			if (m_pPointLights[i] != nullptr)
			{
				delete m_pPointLights[i];
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