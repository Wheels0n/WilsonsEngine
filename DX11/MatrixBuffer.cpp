#include "MatrixBuffer.h"

MatBuffer::MatBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, 
	XMMATRIX* pViewMat, XMMATRIX* pProjMat)
{
 	m_pContext = pContext;
	m_pDevice = pDevice;
	m_pMatBuffer = nullptr;

	m_viewMat =  *pViewMat;
	m_projMat = *pProjMat;
}

MatBuffer::~MatBuffer()
{
	ShutDown();
}

bool MatBuffer::Init()
{  
	HRESULT hr;
	D3D11_BUFFER_DESC matCBD;

	matCBD.Usage = D3D11_USAGE_DYNAMIC;
	matCBD.ByteWidth = sizeof(MatrixBuffer);
	matCBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matCBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matCBD.MiscFlags = 0;
	matCBD.StructureByteStride = 0;
	hr = m_pDevice->CreateBuffer(&matCBD, 0, &m_pMatBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

void MatBuffer::ShutDown()
{
	if (m_pMatBuffer != nullptr)
	{
		m_pMatBuffer->Release();
		m_pMatBuffer = nullptr;
	}

}

void MatBuffer::Update()
{ 
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* pMatrices;

	//ROW-MAJOR(CPU) TO COL-MAJOR(GPU)
	//m_worldMatrix = XMMatrixTranspose(m_worldMatrix);
	//m_viewMatrix  = XMMatrixTranspose(m_viewMatrix);
	//m_projectionMatrix = XMMatrixTranspose(m_projectionMatrix);
	//write CPU data into GPU mem;
	hr = m_pContext->Map(m_pMatBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		return;
	}
	pMatrices = reinterpret_cast<MatrixBuffer*>(mappedResource.pData);
	pMatrices->m_worldMat = XMMatrixTranspose(m_worldMat);
	pMatrices->m_viewMat = XMMatrixTranspose(m_viewMat);
	pMatrices->m_projMat = XMMatrixTranspose(m_projMat);
	pMatrices->m_lightSpaceMat = XMMatrixTranspose(m_lightSpaceMat);
	m_pContext->Unmap(m_pMatBuffer, 0);

	m_pContext->VSSetConstantBuffers(0, 1, &m_pMatBuffer);
	return;
}
