#include "Model.h"
#include <random>
#include <chrono>
namespace wilson {
	Model::Model(VertexData* pVertices,
		unsigned long* pIndices,
		std::vector<unsigned int> vertexDataPos,
		std::vector<unsigned int> indicesPos,
		std::vector<MaterialInfo> materials,
		std::vector<TextureData> textures,
		wchar_t* pName)
	{   
		m_pVertexData = pVertices;
		m_pIndices = pIndices;
		m_vertexCount = vertexDataPos[vertexDataPos.size()-1];
		m_indexCount = indicesPos[indicesPos.size()-1];
		m_vertexDataPos = vertexDataPos;
		m_indicesPos = indicesPos;

		m_pVertexBuffer = nullptr;
		m_pIndexBuffer = nullptr;

		m_diffuseMap = nullptr;

		m_materials = materials;
		m_textures = textures;

		wchar_t* ptr = nullptr;
		wchar_t* lpName = wcstok(pName, (const wchar_t*)L".", &ptr);
		std::wstring wstr(lpName);
		m_Name = std::string(wstr.begin(), wstr.end());
		delete[] lpName;

		m_scMat = DirectX::XMMatrixIdentity();
		m_rtMat = DirectX::XMMatrixIdentity();
		m_trMat = DirectX::XMMatrixIdentity();
		m_angleVec = DirectX::XMVectorZero();

		for (int i = 0; i < m_vertexDataPos.size()-1; ++i)
		{
			m_numVertexData.push_back(m_vertexDataPos[i + 1] - m_vertexDataPos[i]);
			m_numIndices.push_back(m_indicesPos[i + 1] - m_indicesPos[i]);
		}

		m_numInstance = 1;
		m_instancedData = nullptr;
		m_isInstanced = false;
		m_pInstancePosBuffer = nullptr;
		m_pPerModelBuffer = nullptr;
	}
	Model::Model(VertexData* pVertices,
		unsigned long* pIndices,
		unsigned int vertexCount,
		unsigned int indexCount,
		wchar_t* pName,
		std::string matName)
	{

		m_pVertexData = pVertices;
		m_pIndices = pIndices;
		m_vertexCount = vertexCount;
		m_indexCount = indexCount;

		m_pVertexBuffer = nullptr;
		m_pIndexBuffer = nullptr;

		m_diffuseMap = nullptr;
		
		int len=wcslen(pName);
		wchar_t* lpName = new wchar_t[len+1];
		wcscpy(lpName,pName);
		pName[len] = L'\0';
		std::wstring wstr(lpName);
		m_Name = std::string(wstr.begin(), wstr.end());
		delete[] lpName;

		m_matName = matName;

		m_scMat = DirectX::XMMatrixIdentity();
		m_rtMat = DirectX::XMMatrixIdentity();
		m_trMat = DirectX::XMMatrixIdentity();
		m_angleVec = DirectX::XMVectorZero();

		m_instancedData = nullptr;
		m_isInstanced = true;
		m_pInstancePosBuffer = nullptr;
		m_pPerModelBuffer = nullptr;
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
		m_vertexDataPos.clear();
		m_indicesPos.clear();
		m_numVertexData.clear();
		m_numIndices.clear();
		m_textures.clear();
		m_materials.clear();

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

		if (m_pInstancePosBuffer != nullptr)
		{
			m_pInstancePosBuffer->Release();
			m_pInstancePosBuffer = nullptr;
		}

		if (m_pPerModelBuffer != nullptr)
		{
			m_pPerModelBuffer->Release();
			m_pPerModelBuffer = nullptr;
		}

		if (m_instancedData != nullptr)
		{
			delete[] m_instancedData;
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

	void Model::CreateInstanceMatrices()
	{	
		std::random_device rd;
		std::mt19937 rng(rd());
		std::uniform_real_distribution floatGen (5.0f, 50.0f);
		float x, y, z;
		m_instancedData = new DirectX::XMMATRIX[MAX_INSTANCES];
		
		for (int i = 0; i < MAX_INSTANCES; ++i)
		{
			x = floatGen(rng);
			y = floatGen(rng);
			z = floatGen(rng);
		    m_instancedData[i] = DirectX::XMMatrixTranslation(x, y, z);
		}
		
		D3D11_BUFFER_DESC instancePosBD;
		instancePosBD.Usage = D3D11_USAGE_DYNAMIC;
		instancePosBD.ByteWidth = sizeof(DirectX::XMMATRIX) * MAX_INSTANCES;
		instancePosBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instancePosBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		instancePosBD.MiscFlags = 0;
		instancePosBD.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA instanceSubResource = { 0, };
		instanceSubResource.pSysMem = m_instancedData;

		m_pDevice->CreateBuffer(&instancePosBD, &instanceSubResource, &m_pInstancePosBuffer);

	}
	bool Model::Init(ID3D11Device* pDevice)
	{	
		m_pDevice = pDevice;

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


		D3D11_BUFFER_DESC perModelBD;
		perModelBD.Usage = D3D11_USAGE_DYNAMIC;
		perModelBD.ByteWidth = sizeof(CbPerModel);
		perModelBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perModelBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perModelBD.MiscFlags = 0;
		perModelBD.StructureByteStride = 0;

		hr = pDevice->CreateBuffer(&perModelBD, 0, &m_pPerModelBuffer);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}
	bool Model::Init(ID3D11Device* pDevice, Material* pMaterial, ID3D11ShaderResourceView* pDiffuse)
	{
		m_pMaterial = pMaterial;
		m_diffuseMap = pDiffuse;
		return Init(pDevice);
		
	}

	void Model::UploadBuffers(ID3D11DeviceContext* context, int i)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Material* pMaterial;
		DirectX::XMMATRIX* pMatrices;
		CbPerModel* pPerModel;

		unsigned int stride[2];
		unsigned int vOffset[2] = { 0, };
		unsigned int idxOffset;
		stride[0] = sizeof(VertexData);
		stride[1] = sizeof(DirectX::XMMATRIX);
		vOffset[0] = sizeof(VertexData) * m_vertexDataPos[i];
		idxOffset = sizeof(unsigned long) * m_indicesPos[i];


		if (m_isInstanced)
		{	

			if (m_instancedData == nullptr)
			{
				CreateInstanceMatrices();
			}

			ID3D11Buffer* vbs[2] = { m_pVertexBuffer, m_pInstancePosBuffer };
			context->IASetVertexBuffers(0, 2, vbs, stride, vOffset);
		}
		else
		{	
			context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, stride, vOffset);
			context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, idxOffset);
		}
		
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
			pMaterial->ambient = m_materials[i].material.ambient;
			pMaterial->diffuse = m_materials[i].material.diffuse;
			pMaterial->specular = m_materials[i].material.specular;
			context->Unmap(m_pMaterialBuffer, 0);
			context->PSSetConstantBuffers(1, 1, &m_pMaterialBuffer);
		}

		hr = context->Map(m_pPerModelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			return;
		}

		pPerModel = reinterpret_cast<CbPerModel*>(mappedResource.pData);
		pPerModel->isInstanced = m_isInstanced;
		context->Unmap(m_pPerModelBuffer, 0);
		context->VSSetConstantBuffers(2, 1, &m_pPerModelBuffer);
	}
	void Model::UploadBuffers(ID3D11DeviceContext* context)
	{	
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		Material* pMaterial;

		unsigned int stride[2];
		unsigned int offset;

		stride[0] = sizeof(VertexData);
		stride[1] = sizeof(DirectX::XMMATRIX);
		offset = 0;

		context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, stride, &offset);
		context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	    context->PSSetShaderResources(0, 1, &m_diffuseMap);

		hr = context->Map(m_pMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			return;
		}

		pMaterial = reinterpret_cast<Material*>(mappedResource.pData);
		pMaterial->ambient = m_pMaterial->ambient;
		pMaterial->diffuse = m_pMaterial->diffuse;
		pMaterial->specular = m_pMaterial->specular;
		context->Unmap(m_pMaterialBuffer, 0);
		context->PSSetConstantBuffers(1, 1, &m_pMaterialBuffer);
		return;
	}

	DirectX::XMMATRIX Model::GetTransformMatrix()
	{
		DirectX::XMMATRIX srMat = XMMatrixMultiply(m_scMat, m_rtMat);
		DirectX::XMMATRIX srtMat = XMMatrixMultiply(srMat, m_trMat);

		return srtMat;
	}

}