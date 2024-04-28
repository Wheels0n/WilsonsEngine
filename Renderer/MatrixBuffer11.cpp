#include "MatrixBuffer11.h"

namespace wilson
{

	MatBuffer11::MatBuffer11(ID3D11Device* const pDevice, ID3D11DeviceContext* const pContext,
		XMMATRIX* const pViewMat, XMMATRIX* const pProjMat)
	{
		m_pMatricesCb = nullptr;
		m_pProjMatCb = nullptr;

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
		hr = pDevice->CreateBuffer(&cbufferDesc, 0, &m_pMatricesCb);
		assert(SUCCEEDED(hr));
		m_pMatricesCb->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("MatrixBuffer::m_pMatricesCb") - 1, "MatrixBuffer::m_pMatricesCb");

		cbufferDesc.ByteWidth = sizeof(XMMATRIX);
		hr = pDevice->CreateBuffer(&cbufferDesc, 0, &m_pProjMatCb);
		assert(SUCCEEDED(hr));
		m_pProjMatCb->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("MatrixBuffer::m_pProjMatCb") - 1, "MatrixBuffer::m_pProjMatCb");


	}

	MatBuffer11::~MatBuffer11()
	{
		if (m_pMatricesCb != nullptr)
		{
			m_pMatricesCb->Release();
			m_pMatricesCb = nullptr;
		}

		if (m_pProjMatCb != nullptr)
		{
			m_pProjMatCb->Release();
			m_pProjMatCb = nullptr;
		}

	}

	void MatBuffer11::UploadMatBuffer(ID3D11DeviceContext* const pContext, const bool bSpotShadowPass)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MatrixBuffer* pMatrices;

		//ROW-MAJOR(CPU) TO COL-MAJOR(GPU)
		//write CPU data into GPU mem;
		hr = pContext->Map(m_pMatricesCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		pMatrices = reinterpret_cast<MatrixBuffer*>(mappedResource.pData);
		pMatrices->worldMat = m_worldMat;
		//pMatrices->m_viewMat = m_viewMat;
		//pMatrices->m_projMat = m_projMat;
		//pMatrices->m_extraMat = bSpotShadowPass ? m_lightSpaceMat : m_invWorldMat;
		pContext->Unmap(m_pMatricesCb, 0);

		pContext->VSSetConstantBuffers(0, 1, &m_pMatricesCb);
		return;
	}

	void MatBuffer11::UploadProjMat(ID3D11DeviceContext* const pContext)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		XMMATRIX* pMatrix;
		hr = pContext->Map(m_pProjMatCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		pMatrix = reinterpret_cast<XMMATRIX*>(mappedResource.pData);
		*pMatrix = m_projMat;

		pContext->Unmap(m_pProjMatCb, 0);
		pContext->PSSetConstantBuffers(1, 1, &m_pProjMatCb);
		return;
	}

}
