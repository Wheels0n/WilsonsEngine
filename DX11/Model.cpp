#include "Model.h"

CModel::CModel(VertexType* pVertices,
	unsigned long* pIndices,
	unsigned int vertexCount,
	unsigned int indexCount,
	wchar_t* pName)
{  
	m_pVertices = pVertices;
	m_pIndices = pIndices;
	m_pShaderResourceView = nullptr;
	m_vertexCount = vertexCount;
	m_indexCount = indexCount;

	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;

	m_pName = pName;
	m_worldMatrix = XMMatrixIdentity();
}

CModel::CModel(const CModel&)
{
}

CModel::~CModel()
{  
	if (m_pVertices != nullptr)
	{
		delete m_pVertices;
		m_pVertices = nullptr;
	}

	if (m_pIndices != nullptr)
	{
		delete m_pIndices;
		m_pIndices = nullptr;
	}

	if (m_pVertexBuffer != nullptr)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}

	if (m_pIndexBuffer != nullptr)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}

	if (m_pShaderResourceView != nullptr)
	{
		m_pShaderResourceView->Release();
		m_pShaderResourceView = nullptr;
	}
}

bool CModel::Init(ID3D11Device* device)
{   
	HRESULT hr;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;

	vertexData.pSysMem = m_pVertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer);
	if (FAILED(hr))
	{
		return false;
	}


	indexData.pSysMem = m_pIndices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer);
	if (FAILED(hr))
	{
		return false;
	}


	return true;
}


void CModel::UploadBuffers(ID3D11DeviceContext* context)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->PSSetShaderResources(0, 1, &m_pShaderResourceView);
}

void CModel::SetTex(ID3D11ShaderResourceView* tex)
{
	m_pShaderResourceView = tex;
}
