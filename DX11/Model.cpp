#include "Model.h"

namespace wilson {
	Model::Model(VertexData* pVertices,
		unsigned long* pIndices,
		unsigned int vertexCount,
		unsigned int indexCount,
		wchar_t* pName)
	{
		m_pVertexData = pVertices;
		m_pIndices = pIndices;
		m_pSRV = nullptr;
		m_vertexCount = vertexCount;
		m_indexCount = indexCount;

		m_pVertexBuffer = nullptr;
		m_pIndexBuffer = nullptr;

		wchar_t* ptr = nullptr;
		m_pName = wcstok(pName, (const wchar_t*)L".", &ptr);

		m_scMat = DirectX::XMMatrixIdentity();
		m_rtMat = DirectX::XMMatrixIdentity();
		m_trMat = DirectX::XMMatrixIdentity();
		m_angleVec = DirectX::XMVectorZero();
	}

	Model::~Model()
	{
		if (m_pVertexData != nullptr)
		{
			delete m_pVertexData;
			m_pVertexData = nullptr;
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

		if (m_pSRV != nullptr)
		{
			m_pSRV->Release();
			m_pSRV = nullptr;
		}
	}

	bool Model::Init(ID3D11Device* device)
	{
		HRESULT hr;
		D3D11_BUFFER_DESC vertexBD;
		D3D11_BUFFER_DESC indexBD;
		D3D11_SUBRESOURCE_DATA vertexData;
		D3D11_SUBRESOURCE_DATA indexData;

		vertexData.pSysMem = m_pVertexData;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		vertexBD.Usage = D3D11_USAGE_DEFAULT;
		vertexBD.ByteWidth = sizeof(VertexData) * m_vertexCount;
		vertexBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBD.CPUAccessFlags = 0;
		vertexBD.MiscFlags = 0;
		vertexBD.StructureByteStride = 0;

		hr = device->CreateBuffer(&vertexBD, &vertexData, &m_pVertexBuffer);
		if (FAILED(hr))
		{
			return false;
		}


		indexData.pSysMem = m_pIndices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		indexBD.Usage = D3D11_USAGE_DEFAULT;
		indexBD.ByteWidth = sizeof(unsigned long) * m_indexCount;
		indexBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBD.CPUAccessFlags = 0;
		indexBD.MiscFlags = 0;
		indexBD.StructureByteStride = 0;

		hr = device->CreateBuffer(&indexBD, &indexData, &m_pIndexBuffer);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}


	void Model::UploadBuffers(ID3D11DeviceContext* context)
	{
		unsigned int stride;
		unsigned int offset;

		stride = sizeof(VertexData);
		offset = 0;

		context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		context->PSSetShaderResources(0, 1, &m_pSRV);
	}


	DirectX::XMMATRIX Model::GetTransformMatrix()
	{
		DirectX::XMMATRIX srMat = XMMatrixMultiply(m_scMat, m_rtMat);
		DirectX::XMMATRIX srtMat = XMMatrixMultiply(srMat, m_trMat);

		return srtMat;
	}

}