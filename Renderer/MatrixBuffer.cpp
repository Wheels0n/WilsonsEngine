#include "MatrixBuffer.h"

namespace wilson
{

	MatBuffer::MatBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
		XMMATRIX* pViewMat, XMMATRIX* pProjMat)
	{
		m_pMatBuffer = nullptr;
		m_pProjMatBuffer = nullptr;

		m_worldMat = XMMatrixIdentity();
		m_invWorldMat = m_worldMat;
		m_lightSpaceMat = m_worldMat;
		m_extraMat = m_worldMat;

		m_viewMat = *pViewMat;
		m_projMat = *pProjMat;


		HRESULT hr;
		D3D11_BUFFER_DESC cbufferDesc;

		cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbufferDesc.ByteWidth = sizeof(MatrixBuffer);
		cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbufferDesc.MiscFlags = 0;
		cbufferDesc.StructureByteStride = 0;
		hr = pDevice->CreateBuffer(&cbufferDesc, 0, &m_pMatBuffer);
		assert(SUCCEEDED(hr));
		m_pMatBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("MatrixBuffer::m_pMatBuffer") - 1, "MatrixBuffer::m_pMatBuffer");

		cbufferDesc.ByteWidth = sizeof(XMMATRIX);
		hr = pDevice->CreateBuffer(&cbufferDesc, 0, &m_pProjMatBuffer);
		assert(SUCCEEDED(hr));
		m_pProjMatBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("MatrixBuffer::m_pProjMatBuffer") - 1, "MatrixBuffer::m_pProjMatBuffer");


	}

	MatBuffer::~MatBuffer()
	{
		if (m_pMatBuffer != nullptr)
		{
			m_pMatBuffer->Release();
			m_pMatBuffer = nullptr;
		}

		if (m_pProjMatBuffer != nullptr)
		{
			m_pProjMatBuffer->Release();
			m_pProjMatBuffer = nullptr;
		}

	}

	void MatBuffer::UploadMatBuffer(ID3D11DeviceContext* pContext, bool bSpotShadowPass)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MatrixBuffer* pMatrices;

		//ROW-MAJOR(CPU) TO COL-MAJOR(GPU)
		//write CPU data into GPU mem;
		hr = pContext->Map(m_pMatBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		pMatrices = reinterpret_cast<MatrixBuffer*>(mappedResource.pData);
		pMatrices->m_worldMat = m_worldMat;
		//pMatrices->m_viewMat = m_viewMat;
		//pMatrices->m_projMat = m_projMat;
		pMatrices->m_extraMat = bSpotShadowPass ? m_lightSpaceMat : m_invWorldMat;
		pContext->Unmap(m_pMatBuffer, 0);

		pContext->VSSetConstantBuffers(0, 1, &m_pMatBuffer);
		return;
	}

	void MatBuffer::UploadProjMat(ID3D11DeviceContext* pContext)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		XMMATRIX* pMatrix;
		hr = pContext->Map(m_pProjMatBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		pMatrix = reinterpret_cast<XMMATRIX*>(mappedResource.pData);
		*pMatrix = m_projMat;

		pContext->Unmap(m_pProjMatBuffer, 0);
		pContext->PSSetConstantBuffers(1, 1, &m_pProjMatBuffer);
		return;
	}

}
