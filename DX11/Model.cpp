#include "Model.h"
#include <random>
#include <chrono>
namespace wilson {
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

		m_AABB =AABB::GenAABB(pVertices, m_vertexCount);

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

		m_trMat = DirectX::XMMatrixIdentity();
		m_rtMat = m_trMat;
		m_scMat = m_trMat;
		m_outlinerScaleMat = m_scMat;
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


	}

	void Model::CreateInstanceMatrices()
	{	
		std::random_device rd;
		std::mt19937 rng(rd());
		std::uniform_real_distribution floatGen (5.0f, 50.0f);
		float x, y, z;
		m_instancedData = new DirectX::XMMATRIX[_MAX_INSTANCES];
		
		for (int i = 0; i < _MAX_INSTANCES; ++i)
		{
			x = floatGen(rng);
			y = floatGen(rng);
			z = floatGen(rng);
		    m_instancedData[i] = DirectX::XMMatrixTranslation(x, y, z);
		}
		
		D3D11_BUFFER_DESC instancePosBD;
		instancePosBD.Usage = D3D11_USAGE_DYNAMIC;
		instancePosBD.ByteWidth = sizeof(DirectX::XMMATRIX) * _MAX_INSTANCES;
		instancePosBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instancePosBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		instancePosBD.MiscFlags = 0;
		instancePosBD.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA instanceSubResource = { 0, };
		instanceSubResource.pSysMem = m_instancedData;

		m_pDevice->CreateBuffer(&instancePosBD, &instanceSubResource, &m_pInstancePosBuffer);

	}
	bool Model::CreateBuffer(ID3D11Device* pDevice)
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
		m_pVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Model::m_pVertexBuffer") - 1, "Model::m_pVertexBuffer");

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
		m_pIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Model::m_pIndexBuffer") - 1, "Model::m_pIndexBuffer");


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
		m_pMaterialBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Model::m_pMaterialBuffer") - 1, "Model::m_pMaterialBuffer");


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
		m_pPerModelBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Model::m_pPerModelBuffer") - 1, "Model::m_pPerModelBuffer");

		return true;
	}
	bool Model::Init(ID3D11Device* pDevice, std::unordered_map<std::string, int>& mathash, std::vector<MaterialInfo>& matInfos,
		std::unordered_map<std::string, int>& texhash, std::vector<ID3D11ShaderResourceView*>& textures)
	{	
		m_matInfos.reserve(m_matNames.size());
		m_perModels.reserve(m_matNames.size());
		for (int i = 0; i < m_matNames.size(); ++i)
		{
			PerModel perModel = {false, };
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
			if (!matInfo.emissiveMap.empty())
			{
				idx = texhash[matInfo.emissiveMap];
				m_texHash[matInfo.emissiveMap] = m_textures.size();
				m_textures.push_back(textures[idx]);
				perModel.hasEmissive = true;
			}
			m_perModels.push_back(perModel);
		}
		return CreateBuffer(pDevice);
		
	}

	void Model::UploadBuffers(ID3D11DeviceContext* pContext, int i, bool bGeoPass)
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
			pContext->IASetVertexBuffers(0, 2, vbs, stride, &vOffset);
		}
		else
		{
			pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, stride, &vOffset);
		}
		
		pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, iOffset);
		
		UINT texCnt = 0;
		UINT idx = m_texHash[matInfo.diffuseMap];
		ID3D11ShaderResourceView* nullSRV[5]={nullptr,};
		pContext->PSSetShaderResources(0, 1, &m_textures[idx]);
		if (bGeoPass)
		{	
			if (m_perModels[i].hasNormal)
			{
				idx = m_texHash[matInfo.normalMap];
				pContext->PSSetShaderResources(1, 1, &m_textures[idx]);
			}
			else
			{
				pContext->PSSetShaderResources(1, 1, &nullSRV[1]);
			}

			if (m_perModels[i].hasSpecular)
			{
				idx = m_texHash[matInfo.specularMap];
				pContext->PSSetShaderResources(2, 1, &m_textures[idx]);
			}
			else
			{
				pContext->PSSetShaderResources(2, 1, &nullSRV[2]);
			}
		
			if (m_perModels[i].hasEmissive)
			{
				idx = m_texHash[matInfo.emissiveMap];
				pContext->PSSetShaderResources(3, 1, &m_textures[idx]);
			}
			else
			{
				pContext->PSSetShaderResources(3, 1, &nullSRV[3]);
			}

			
			if (m_perModels[i].hasAlpha)
			{
				idx = m_texHash[matInfo.alphaMap];
				pContext->PSSetShaderResources(4, 1, &m_textures[idx]);
			}
			else
			{
				pContext->PSSetShaderResources(4, 1, &nullSRV[4]);
			}


			hr = pContext->Map(m_pMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				return;
			}

			pMaterial = reinterpret_cast<Material*>(mappedResource.pData);
			pMaterial->ambient = matInfo.material.ambient;
			pMaterial->diffuse = matInfo.material.diffuse;
			pMaterial->specular = matInfo.material.specular;
			pMaterial->reflect = matInfo.material.reflect;
			pContext->Unmap(m_pMaterialBuffer, 0);
			pContext->PSSetConstantBuffers(2, 1, &m_pMaterialBuffer);


			hr = pContext->Map(m_pPerModelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr))
			{
				return;
			}

			pPerModel = reinterpret_cast<PerModel*>(mappedResource.pData);
			pPerModel->hasSpecular = m_perModels[i].hasSpecular;
			pPerModel->hasNormal = m_perModels[i].hasNormal;
			pPerModel->hasAlpha = m_perModels[i].hasAlpha;
			pPerModel->hasEmissive=m_perModels[i].hasEmissive;

			pContext->Unmap(m_pPerModelBuffer, 0);
			pContext->VSSetConstantBuffers(1, 1, &m_pPerModelBuffer);
			pContext->PSSetConstantBuffers(1, 1, &m_pPerModelBuffer);
		}
		return;
	}

	AABB Model::GetGlobalAABB()
	{	
		DirectX::XMMATRIX transform =  GetTransformMatrix(false);
		

		DirectX::XMVECTOR centerV = m_AABB.GetCenter();
		DirectX::XMVECTOR globalCenterV = DirectX::XMVector3Transform(centerV, transform);
		DirectX::XMFLOAT4 globalCenter;
		DirectX::XMStoreFloat4(&globalCenter, globalCenterV);

		DirectX::XMVECTOR extentsV = m_AABB.GetExtents();
		DirectX::XMFLOAT3 extents;
		DirectX::XMStoreFloat3(&extents, extentsV);

		 transform = DirectX::XMMatrixTranspose(transform);
		DirectX::XMVECTOR right = DirectX::XMVectorScale(transform.r[0], extents.x);
		DirectX::XMVECTOR up = DirectX::XMVectorScale(transform.r[1], extents.y);
		DirectX::XMVECTOR forward = DirectX::XMVectorScale(transform.r[2], extents.z);


		DirectX::XMVECTOR x = DirectX::XMVectorSet(1.0, 0.0f, 0.0f, 0.f);
		DirectX::XMVECTOR y = DirectX::XMVectorSet(0.0, 1.0f, 0.0f, 0.f);
		DirectX::XMVECTOR z = DirectX::XMVectorSet(0.0, 0.0f, 1.0f, 0.f);

		float dotRight;
		float dotUp;
		float dotForward;

		DirectX::XMStoreFloat(&dotRight, DirectX::XMVector3Dot(x, right));
		DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(x, up));
		DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(x, up));

		const float newli = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);

		DirectX::XMStoreFloat(&dotRight, DirectX::XMVector3Dot(y, right));
		DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(y, up));
		DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(y, up));

		const float newlj = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);

		DirectX::XMStoreFloat(&dotRight, DirectX::XMVector3Dot(z, right));
		DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(z, up));
		DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(z, up));

		const float newlk = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);


		const AABB globalAABB(globalCenter, newli, newlj, newlk);


		return globalAABB;
	}

	DirectX::XMMATRIX Model::GetTransformMatrix(bool bOutliner)
	{	
		
		DirectX::XMMATRIX srMat;
		if (bOutliner)
		{
			srMat = XMMatrixMultiply(m_outlinerScaleMat, m_rtMat);
		}
		else
		{
			srMat = XMMatrixMultiply(m_scMat, m_rtMat);
		}
		DirectX::XMMATRIX rtMat = XMMatrixMultiply(srMat,m_trMat);

		return rtMat;
	}

}