#include "MatrixBuffer.h"

CMBuffer::CMBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX* viewMatrix, XMMATRIX* projectionMatrix)
{
 	m_pContext = context;
	m_pDevice = device;
	m_pMatrixBuffer = nullptr;

	m_viewMat =  *viewMatrix;
	m_projMat = *projectionMatrix;
}

CMBuffer::~CMBuffer()
{
}

bool CMBuffer::Init()
{  
	HRESULT hr;
	D3D11_BUFFER_DESC mbDesc;

	mbDesc.Usage = D3D11_USAGE_DYNAMIC;
	mbDesc.ByteWidth = sizeof(MatrixBuffer);
	mbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbDesc.MiscFlags = 0;
	mbDesc.StructureByteStride = 0;
	hr = m_pDevice->CreateBuffer(&mbDesc, 0, &m_pMatrixBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

void CMBuffer::ShutDown()
{
	if (m_pMatrixBuffer != nullptr)
	{
		m_pMatrixBuffer->Release();
		m_pMatrixBuffer = nullptr;
	}

}

void CMBuffer::Update()
{ 
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* pMatrices;

	//ROW-MAJOR(CPU) TO COL-MAJOR(GPU)
	//m_worldMatrix = XMMatrixTranspose(m_worldMatrix);
	//m_viewMatrix  = XMMatrixTranspose(m_viewMatrix);
	//m_projectionMatrix = XMMatrixTranspose(m_projectionMatrix);
	//write CPU data into GPU mem;
	hr = m_pContext->Map(m_pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
	{
		return;
	}
	pMatrices = reinterpret_cast<MatrixBuffer*>(mappedResource.pData);
	pMatrices->world = XMMatrixTranspose(m_worldMatrix);
	pMatrices->view = XMMatrixTranspose(m_viewMat);
	pMatrices->projection = m_projMat;
	m_pContext->Unmap(m_pMatrixBuffer, 0);

	m_pContext->VSSetConstantBuffers(0, 1, &m_pMatrixBuffer);
	return;
}

void CMBuffer::SetWorldMatrix(XMMATRIX* worldMatrix)
{
	m_worldMatrix = *worldMatrix;
}

void CMBuffer::SetViewMatrix(XMMATRIX* viewMatrix)
{
	m_viewMat = *viewMatrix;
}
