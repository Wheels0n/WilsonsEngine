#include "Terrain.h"

Terrain::Terrain()
{
	m_terrainWidth = 0;
	m_terrainHeight = 0;
	m_vertexCount = 0;
	m_indexCount = 0;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
}

Terrain::~Terrain()
{
	if (m_indexBuffer != nullptr)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	if (m_vertexBuffer != nullptr)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

bool Terrain::Init(ID3D11Device* pDevice, int tWidth, int tHeight)
{   
	m_terrainWidth = tWidth;
	m_terrainHeight = tHeight;

	//With a line list, every two vertices in the draw call forms an individual line. so 2n vertices induces n lines.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 8;
	m_indexCount = m_vertexCount;

	GridType* vertices = new GridType[m_vertexCount];
	if (vertices == nullptr)
	{
		return false;
	}

	unsigned long* indices = new unsigned long[m_indexCount];
	if (indices == nullptr)
	{
		return false;
	}

	int idx = 0;

	for (int i = 0; i < (m_terrainHeight - 1); ++i)
	{
		for (int j = 0; j < (m_terrainWidth - 1); ++j)
		{   
			//top-left to top-right
			float posX = (float)i;
			float posZ = (float)(j + 1);

			vertices[idx].pos = XMFLOAT3(posX, 0.0f, posZ);
			vertices[idx].col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[idx] = idx;
			++idx;

			posX = (float)(i + 1);
			posZ = (float)(j + 1);

			vertices[idx].pos = XMFLOAT3(posX, 0.0f, posZ);
			vertices[idx].col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[idx] = idx;
			++idx;

			//top-right to bottom-right
			posX = (float)(i + 1);
			posZ = (float)(j + 1);

			vertices[idx].pos = XMFLOAT3(posX, 0.0f, posZ);
			vertices[idx].col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[idx] = idx;
			++idx;

			posX = (float)(i + 1);
			posZ = (float)j;

			vertices[idx].pos = XMFLOAT3(posX, 0.0f, posZ);
			vertices[idx].col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[idx] = idx;
			++idx;

			//bottom-right to bottom-left
			posX = (float)(i + 1);
			posZ = (float)j;

			vertices[idx].pos = XMFLOAT3(posX, 0.0f, posZ);
			vertices[idx].col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[idx] = idx;
			++idx;

			posX = (float)i;
			posZ = (float)j;

			vertices[idx].pos = XMFLOAT3(posX, 0.0f, posZ);
			vertices[idx].col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[idx] = idx;
			++idx;

			//bottom-left to top-left
			posX = (float)i;
			posZ = (float)j;

			vertices[idx].pos = XMFLOAT3(posX, 0.0f, posZ);
			vertices[idx].col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[idx] = idx;
			++idx;

			posX = (float)i;
			posZ = (float)(j + 1);

			vertices[idx].pos = XMFLOAT3(posX, 0.0f, posZ);
			vertices[idx].col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			indices[idx] = idx;
			++idx;
		}
	}

	D3D11_BUFFER_DESC vBufferDSC;
	vBufferDSC.Usage = D3D11_USAGE_DEFAULT;
	vBufferDSC.ByteWidth = sizeof(GridType) * m_vertexCount;
	vBufferDSC.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBufferDSC.CPUAccessFlags = 0;
	vBufferDSC.MiscFlags = 0;
	vBufferDSC.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vData;
	vData.pSysMem = vertices;
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	if (FAILED(pDevice->CreateBuffer(&vBufferDSC, &vData, &m_vertexBuffer)))
	{
		return false;
	}


	D3D11_BUFFER_DESC idxBufferDSC;
	idxBufferDSC.Usage = D3D11_USAGE_DEFAULT;
	idxBufferDSC.ByteWidth = sizeof(unsigned long) * m_indexCount;
	idxBufferDSC.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxBufferDSC.CPUAccessFlags = 0;
	idxBufferDSC.MiscFlags = 0;
	idxBufferDSC.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA idxData;
	idxData.pSysMem = indices;
	idxData.SysMemPitch = 0;
	idxData.SysMemSlicePitch = 0;

	if (FAILED(pDevice->CreateBuffer(&idxBufferDSC, &idxData, &m_indexBuffer)))
	{
		return false;
	}

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void Terrain::UploadBuffers(ID3D11DeviceContext* pContext)
{
	unsigned int stride = sizeof(GridType);
	unsigned int offset = 0;

	pContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	return;
}
