#include "Model.h"

namespace wilson {
	Model::Model(VertexData* pVertices,
		unsigned long* pIndices,
		unsigned int vertexCount,
		unsigned int indexCount,
		std::vector<Material> materialV,
		std::vector<TextureData> texDataV,
		wchar_t* pName)
	{   
		m_eObjectType = FBX;

		m_pVertexData = pVertices;
		m_pIndices = pIndices;
		m_vertexCount = vertexCount;
		m_indexCount = indexCount;

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

	}

	bool Model::Init(ID3D11Device* device)
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

		materialBD.Usage = D3D11_USAGE_DYNAMIC;
		materialBD.ByteWidth = sizeof(Material);
		materialBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		materialBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		materialBD.MiscFlags = 0;
		materialBD.StructureByteStride = 0;

		hr = device->CreateBuffer(&materialBD, 0, &m_pMaterialBuffer);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
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
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		
		
		if (m_eObjectType == EObjectType::FBX)
		{
			context->PSSetShaderResources(0, 1, &m_textures[0].texture);
			hr = context->Map(m_pMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				return;
			}

			pMaterial = reinterpret_cast<Material*>(mappedResource.pData);
			pMaterial->ambient = m_materials[0].ambient;
			pMaterial->diffuse = m_materials[0].diffuse;
			pMaterial->specular = m_materials[0].specular;
			context->Unmap(m_pMaterialBuffer, 0);
			context->PSSetConstantBuffers(1, 1, &m_pMaterialBuffer);
		}
		else
		{
			context->PSSetShaderResources(0, 1, &m_SRV);
		}
		return;
	}


	DirectX::XMMATRIX Model::GetTransformMatrix()
	{
		DirectX::XMMATRIX srMat = XMMatrixMultiply(m_scMat, m_rtMat);
		DirectX::XMMATRIX srtMat = XMMatrixMultiply(srMat, m_trMat);

		return srtMat;
	}

}