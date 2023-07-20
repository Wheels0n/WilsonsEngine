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

		m_matInfos = materials;
		m_texDatas = textures;

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
		std::vector<unsigned int> vertexDataPos,
		std::vector<unsigned int> indicesPos,
		wchar_t* pName,
		std::vector<std::string> matNames)
	{

		m_pVertexData = pVertices;
		m_pIndices = pIndices;
		m_vertexDataPos = vertexDataPos;
		m_indicesPos = indicesPos;
		m_vertexCount = vertexDataPos[vertexDataPos.size() - 1];
		m_indexCount = indicesPos[indicesPos.size() - 1];

		/*if (matNames.size() > 1)
		{
			++m_indicesPos[indicesPos.size() - 1];
		}*/

		m_pVertexBuffer = nullptr;
		m_pIndexBuffer = nullptr;

		int len=wcslen(pName);
		wchar_t* lpName = new wchar_t[len+1];
		wcscpy(lpName,pName);
		pName[len] = L'\0';
		std::wstring wstr(lpName);
		m_Name = std::string(wstr.begin(), wstr.end());
		delete[] lpName;

		m_matNames = matNames;

		m_scMat = DirectX::XMMatrixIdentity();
		m_rtMat = DirectX::XMMatrixIdentity();
		m_trMat = DirectX::XMMatrixIdentity();
		m_angleVec = DirectX::XMVectorZero();

		m_instancedData = nullptr;
		m_isInstanced = false;
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
		m_texDatas.clear();
		m_textures.clear();
		m_texHash.clear();
		m_matInfos.clear();
		m_matNames.clear();
		m_perModels.clear();

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

		for (int i = 0; i < m_texDatas.size(); ++i)
		{
			if (m_texDatas[i].texture != nullptr)
			{
				m_texDatas[i].texture->Release();
				m_texDatas[i].texture = nullptr;
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
		perModelBD.ByteWidth = sizeof(PerModel);
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
	bool Model::Init(ID3D11Device* pDevice, std::unordered_map<std::string, int>& mathash, std::vector<MaterialInfo>& matInfos,
		std::unordered_map<std::string, int>& texhash, std::vector<ID3D11ShaderResourceView*>& textures)
	{	
		m_matInfos.reserve(m_matNames.size());
		m_perModels.reserve(m_matNames.size());
		for (int i = 0; i < m_matNames.size(); ++i)
		{
			PerModel perModel = { false, };
			int idx = mathash[m_matNames[i]];
			MaterialInfo matInfo = matInfos[idx];
			m_matInfos.push_back(matInfo);

			idx = texhash[matInfo.diffuseMap];
			m_texHash[matInfo.diffuseMap] = m_textures.size();
			m_textures.push_back(textures[idx]);

			if (!matInfo.specularMap.empty())
			{
				idx = texhash[matInfo.specularMap];
				m_texHash[matInfo.specularMap] = m_textures.size();
				m_textures.push_back(textures[idx]);
				perModel.hasSpecular = true;
			}
			if (!matInfo.normalMap.empty())
			{
				idx = texhash[matInfo.normalMap];
				m_texHash[matInfo.normalMap] = m_textures.size();
				m_textures.push_back(textures[idx]);
				perModel.hasNormal = true;
			}
			if (!matInfo.alphaMap.empty())
			{
				idx = texhash[matInfo.alphaMap];
				m_texHash[matInfo.alphaMap] = m_textures.size();
				m_textures.push_back(textures[idx]);
				perModel.hasAlpha = true;
			}
			m_perModels.push_back(perModel);
		}
		return Init(pDevice);
		
	}

	void Model::UploadBuffers(ID3D11DeviceContext* context, int i)
	{	
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MaterialInfo matInfo = m_matInfos[i];
		Material* pMaterial;
		PerModel* pPerModel;
		UINT stride[2];
		UINT vOffset,iOffset;

		stride[0] = sizeof(VertexData);
		stride[1] = sizeof(DirectX::XMMATRIX);
		vOffset = 0;
		iOffset = sizeof(UINT) * m_indicesPos[i];
		if (m_isInstanced)
		{

			if (m_instancedData == nullptr)
			{
				CreateInstanceMatrices();
			}

			ID3D11Buffer* vbs[2] = { m_pVertexBuffer, m_pInstancePosBuffer };
			context->IASetVertexBuffers(0, 2, vbs, stride, &vOffset);
		}
		else
		{
			context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, stride, &vOffset);
		}
		
		context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, iOffset);
		
		
		{
			int idx = m_texHash[matInfo.diffuseMap];
			context->PSSetShaderResources(0, 1, &m_textures[idx]);
			if (m_perModels[i].hasSpecular)
			{
				idx = m_texHash[matInfo.specularMap];
				context->PSSetShaderResources(1, 1, &m_textures[idx]);
			}
			if (m_perModels[i].hasNormal)
			{
				idx = m_texHash[matInfo.normalMap];
				context->PSSetShaderResources(2, 1, &m_textures[idx]);
			}
			if (m_perModels[i].hasAlpha)
			{
				idx = m_texHash[matInfo.alphaMap];
				context->PSSetShaderResources(3, 1, &m_textures[idx]);
			}



			hr = context->Map(m_pMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				return;
			}

			pMaterial = reinterpret_cast<Material*>(mappedResource.pData);
			pMaterial->ambient = matInfo.material.ambient;
			pMaterial->diffuse = matInfo.material.diffuse;
			pMaterial->specular = matInfo.material.specular;
			context->Unmap(m_pMaterialBuffer, 0);
			context->PSSetConstantBuffers(1, 1, &m_pMaterialBuffer);


			hr = context->Map(m_pPerModelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				return;
			}

			pPerModel = reinterpret_cast<PerModel*>(mappedResource.pData);
			pPerModel->isInstanced = m_isInstanced;
			pPerModel->hasSpecular = m_perModels[i].hasSpecular;
			pPerModel->hasNormal = m_perModels[i].hasNormal;
			pPerModel->hasAlpha = m_perModels[i].hasAlpha;

			context->Unmap(m_pPerModelBuffer, 0);
			context->VSSetConstantBuffers(2, 1, &m_pPerModelBuffer);
			context->PSSetConstantBuffers(2, 1, &m_pPerModelBuffer);
		}
		return;
	}

	DirectX::XMMATRIX Model::GetTransformMatrix()
	{
		DirectX::XMMATRIX srMat = XMMatrixMultiply(m_scMat, m_rtMat);
		DirectX::XMMATRIX rtMat = XMMatrixMultiply(srMat,m_trMat);

		return rtMat;
	}

}