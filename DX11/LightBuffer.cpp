#include "LightBuffer.h"

namespace wilson
{
	void LightBuffer::UpdateLightBuffer(ID3D11DeviceContext* pContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		LightBufferProperty* plbProperty;
		HRESULT hr = pContext->Map(m_pLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
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
		pContext->Unmap(m_pLightBuffer,0);
		pContext->PSSetConstantBuffers(0, 1, &m_pLightBuffer);
		
		
	}
	LightBuffer::LightBuffer(ID3D11Device* pDevice)
	{	
		m_pLightBuffer = nullptr;
		D3D11_BUFFER_DESC bdc;
		bdc.Usage = D3D11_USAGE_DYNAMIC;
		bdc.ByteWidth = sizeof(LightBufferProperty);
		bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bdc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bdc.MiscFlags = 0;
		bdc.StructureByteStride = 0;
		pDevice->CreateBuffer(&bdc, 0, &m_pLightBuffer);

		m_pDirLights.reserve(MAX_DIR_LIGHTS);
		m_pPointLights.reserve(MAX_PNT_LIGHTS);
		m_pSpotLights.reserve(MAX_SPT_LIGHTS);
	}
	LightBuffer::~LightBuffer()
	{	
		if (m_pLightBuffer != nullptr)
		{
			m_pLightBuffer->Release();
			m_pLightBuffer = nullptr;
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