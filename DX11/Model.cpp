#include "Model.h"

namespace wilson {
	Model::Model(VertexData* pVertices,
		unsigned long* pIndices,
		std::vector<unsigned int> vertexDataPos,
		std::vector<unsigned int> indicesPos,
		std::vector<Material> materialV,
		std::vector<TextureData> texDataV,
		wchar_t* pName)
	{   
		m_eObjectType = FBX;

		m_pVertexData = pVertices;
		m_pIndices = pIndices;
		m_vertexCount = vertexDataPos[vertexDataPos.size()-1];
		m_indexCount = indicesPos[indicesPos.size()-1];
		m_vertexDataPos = vertexDataPos;
		m_indicesPos = indicesPos;

		m_pVertexBuffer = nullptr;
		m_pIndexBuffer = nullptr;

		m_materials = materialV;
		m_textures = texDataV;
		m_SRV = nullptr;

		wchar_t* ptr = nullptr;
		m_pName = wcstok(pName, (const wchar_t*)L".", &ptr);

		m_scMat = DirectX::XMMatrixIdentity();
		m_rtMat = DirectX::XMMatrixIdentity();
		m_trMat = DirectX::XMMatrixIdentity();
		m_angleVec = DirectX::XMVectorZero();

		for (int i = 0; i < m_vertexDataPos.size()-1; ++i)
		{
			m_numVertexData.push_back(m_vertexDataPos[i + 1] - m_vertexDataPos[i]);
			m_numIndices.push_back(m_indicesPos[i + 1] - m_indicesPos[i]);
		}

	}
	Model::Model(VertexData* pVertices,
		unsigned long* pIndices,
		unsigned int vertexCount,
		unsigned int indexCount,
		wchar_t* pName)
	{
		m_eObjectType = OBJ;

		m_pVertexData = pVertices;
		m_pIndices = pIndices;
		m_vertexCount = vertexCount;
		m_indexCount = indexCount;

		m_pVertexBuffer = nullptr;
		m_pIndexBuffer = nullptr;

		m_SRV = nullptr;
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

		if (m_pMaterialBuffer != nullptr)
		{
			m_pMaterialBuffer->Release();
			m_pMaterialBuffer = nullptr;
		}

		if (m_SRV != nullptr)
		{
			m_SRV->Release();
			m_SRV = nullptr;	
		}

		for (int i = 0; i < m_textures.size(); ++i)
		{
			if (m_textures[i].texture != nullptr)
			{
				m_textures[i].texture->Release();
				m_textures[i].texture = nullptr;
			}
		}

	}

	bool Model::Init(ID3D11Device* pDevice)
	{
		HRESULT hr;
		D3D11_BUFFER_DESC vertexBD;
		D3D11_BUFFER_DESC indexBD;
		D3D11_BUFFER_DESC materialBD;
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

		hr = pDevice->CreateBuffer(&vertexBD, &vertexData, &m_pVertexBuffer);
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

		hr = pDevice->CreateBuffer(&indexBD, &indexData, &m_pIndexBuffer);
		if (FAILED(hr))
		{
			return false;
		}

		materialBD.Usage = D3D11_USAGE_DYNAMIC;
		materialBD.ByteWidth = sizeof(Material);
		materialBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		materialBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		materialBD.MiscFlags = 0;
		materialBD.StructureByteStride = 0;

		hr = pDevice->CreateBuffer(&materialBD, 0, &m_pMaterialBuffer);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}
	void Model::UploadBuffers(ID3D11DeviceContext* context, int i)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Material* pMaterial;

		unsigned int stride;
		unsigned int vOffset;
		unsigned int idxOffset;

		stride = sizeof(VertexData);
		vOffset = sizeof(VertexData)* m_vertexDataPos[i];
		idxOffset = sizeof(unsigned long) * m_indicesPos[i];
		context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &vOffset);
		context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, idxOffset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		if (i < m_textures.size())
		{
			hr = context->Map(m_pMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				return;
			}

			context->PSSetShaderResources(0, 1, &m_textures[i].texture);
			pMaterial = reinterpret_cast<Material*>(mappedResource.pData);
			pMaterial->ambient = m_materials[i].ambient;
			pMaterial->diffuse = m_materials[i].diffuse;
			pMaterial->specular = m_materials[i].specular;
			context->Unmap(m_pMaterialBuffer, 0);
			context->PSSetConstantBuffers(1, 1, &m_pMaterialBuffer);
		}
	}
	void Model::UploadBuffers(ID3D11DeviceContext* context)
	{	
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Material* pMaterial;

		unsigned int stride;
		unsigned int offset;

		stride = sizeof(VertexData);
		offset = 0;

		context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	    context->PSSetShaderResources(0, 1, &m_SRV);
		return;
	}


	DirectX::XMMATRIX Model::GetTransformMatrix()
	{
		DirectX::XMMATRIX srMat = XMMatrixMultiply(m_scMat, m_rtMat);
		DirectX::XMMATRIX srtMat = XMMatrixMultiply(srMat, m_trMat);

		return srtMat;
	}

}