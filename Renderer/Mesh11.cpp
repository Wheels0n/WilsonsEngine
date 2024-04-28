#include "Mesh11.h"
namespace wilson {
	Mesh11::Mesh11(ID3D11Device* const pDevice, VertexData* const pVertices,
		unsigned long* const pIndices,
		const std::vector<unsigned int> vertexDataPos,
		const std::vector<unsigned int> indicesPos,
		wchar_t* const pName,
		const std::vector<std::string> matNames)
	{

		m_pVertexData = pVertices;
		m_pIndices = pIndices;
		m_vertexDataPos = vertexDataPos;
		m_indicesPos = indicesPos;
		m_nVertex = vertexDataPos[vertexDataPos.size() - 1];
		m_nIndex = indicesPos[indicesPos.size() - 1];


		DirectX::XMFLOAT3 minAABB(FLT_MAX, FLT_MAX, FLT_MAX);
		DirectX::XMFLOAT3 maxAABB(FLT_MIN, FLT_MIN, FLT_MIN);
		for (UINT i = 0; i < m_nVertex; ++i)
		{
			minAABB.x = min(minAABB.x, m_pVertexData[i].position.x);
			minAABB.y = min(minAABB.y, m_pVertexData[i].position.y);
			minAABB.z = min(minAABB.z, m_pVertexData[i].position.z);

			maxAABB.x = max(maxAABB.x, m_pVertexData[i].position.x);
			maxAABB.y = max(maxAABB.y, m_pVertexData[i].position.y);
			maxAABB.z = max(maxAABB.z, m_pVertexData[i].position.z);
		}
		m_pAABB = nullptr;
		m_pAABB = new AABB(minAABB, maxAABB);

		m_pVb = nullptr;
		m_pIb = nullptr;

		int len=wcslen(pName);
		wchar_t* lpName = new wchar_t[len+1];
		wcscpy(lpName,pName);
		pName[len] = L'\0';
		std::wstring wstr(lpName);
		m_name = std::string(wstr.begin(), wstr.end());
		delete[] lpName;

		m_matNames = matNames;

		m_wMat = DirectX::XMMatrixIdentity();
		m_trMat = m_wMat;
		m_rtMat = m_trMat;
		m_scMat = m_trMat;
		m_invWMat = m_wMat;

		DirectX::XMVECTOR xv = DirectX::XMVectorSet(1.01f, 1.01f, 1.01f, 1.0f);
		m_outlinerScaleMat = DirectX::XMMatrixScalingFromVector(xv);
		m_outlinerMat = m_outlinerScaleMat;

		m_angleVec = DirectX::XMVectorZero();

		m_instancedData = nullptr;
		m_bInstanced = false;
		m_pInstancePosCb = nullptr;
		m_pPerModelCb = nullptr;

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
			vertexBD.ByteWidth = sizeof(VertexData) * m_nVertex;
			vertexBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBD.CPUAccessFlags = 0;
			vertexBD.MiscFlags = 0;
			vertexBD.StructureByteStride = 0;

			hr = pDevice->CreateBuffer(&vertexBD, &vertexData, &m_pVb);
			assert(SUCCEEDED(hr));
			m_pVb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Mesh11::m_pVb") - 1, "Mesh11::m_pVb");

			indexData.pSysMem = m_pIndices;
			indexData.SysMemPitch = 0;
			indexData.SysMemSlicePitch = 0;

			indexBD.Usage = D3D11_USAGE_DEFAULT;
			indexBD.ByteWidth = sizeof(unsigned long) * m_nIndex;
			indexBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBD.CPUAccessFlags = 0;
			indexBD.MiscFlags = 0;
			indexBD.StructureByteStride = 0;

			hr = pDevice->CreateBuffer(&indexBD, &indexData, &m_pIb);
			assert(SUCCEEDED(hr));
			m_pIb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Mesh11::m_pIb") - 1, "Mesh11::m_pIb");


			materialBD.Usage = D3D11_USAGE_DYNAMIC;
			materialBD.ByteWidth = sizeof(Material);
			materialBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			materialBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			materialBD.MiscFlags = 0;
			materialBD.StructureByteStride = 0;

			hr = pDevice->CreateBuffer(&materialBD, 0, &m_pMaterialCb);
			assert(SUCCEEDED(hr));
			m_pMaterialCb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Mesh11::m_pMaterialCb") - 1, "Mesh11::m_pMaterialCb");


			D3D11_BUFFER_DESC perModelBD;
			perModelBD.Usage = D3D11_USAGE_DYNAMIC;
			perModelBD.ByteWidth = sizeof(PerModel);
			perModelBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			perModelBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			perModelBD.MiscFlags = 0;
			perModelBD.StructureByteStride = 0;

			hr = pDevice->CreateBuffer(&perModelBD, 0, &m_pPerModelCb);
			assert(SUCCEEDED(hr));
			m_pPerModelCb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Mesh11::m_pPerModelCb") - 1, "Mesh11::m_pPerModelCb");

		}
	}

	Mesh11::~Mesh11()
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
		m_nVertexData.clear();
		m_nIndices.clear();
		m_texSrvs.clear();
		m_texHash.clear();
		m_matInfos.clear();
		m_matNames.clear();
		m_perModels.clear();

		if (m_pVb != nullptr)
		{
			m_pVb->Release();
			m_pVb = nullptr;
		}

		if (m_pIb != nullptr)
		{
			m_pIb->Release();
			m_pIb = nullptr;
		}

		if (m_pMaterialCb != nullptr)
		{
			m_pMaterialCb->Release();
			m_pMaterialCb = nullptr;
		}

		if (m_pInstancePosCb != nullptr)
		{
			m_pInstancePosCb->Release();
			m_pInstancePosCb = nullptr;
		}

		if (m_pPerModelCb != nullptr)
		{
			m_pPerModelCb->Release();
			m_pPerModelCb = nullptr;
		}

		if (m_instancedData != nullptr)
		{
			delete[] m_instancedData;
		}

		if (m_pAABB != nullptr)
		{
			delete m_pAABB;
		}

	}

	void Mesh11::CreateInstanceMatrices()
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

		m_pDevice->CreateBuffer(&instancePosBD, &instanceSubResource, &m_pInstancePosCb);

	}
	void Mesh11::BindMaterial(const std::unordered_map<std::string, int>& mathash, const std::vector<MaterialInfo>& matInfos,
		const std::unordered_map<std::string, int>& texhash, const std::vector<ID3D11ShaderResourceView*>& textures)
	{	
		m_matInfos.reserve(m_matNames.size());
		m_perModels.reserve(m_matNames.size());
		for (int i = 0; i < m_matNames.size(); ++i)
		{
			PerModel perModel = {false, };
			int idx = mathash.at(m_matNames[i]);
			MaterialInfo matInfo = matInfos[idx];
			m_matInfos.push_back(matInfo);

			idx = texhash.at(matInfo.diffuseMap);
			m_texHash[matInfo.diffuseMap] = m_texSrvs.size();
			m_texSrvs.push_back(textures[idx]);

			if (!matInfo.specularMap.empty())
			{
				idx = texhash.at(matInfo.specularMap);
				m_texHash[matInfo.specularMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasSpecular = true;
			}
		
			if (!matInfo.normalMap.empty())
			{
				idx = texhash.at(matInfo.normalMap);
				m_texHash[matInfo.normalMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasNormal = true;
			}
			
			if (!matInfo.alphaMap.empty())
			{
				idx = texhash.at(matInfo.alphaMap);
				m_texHash[matInfo.alphaMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasAlpha = true;
			}
			if (!matInfo.emissiveMap.empty())
			{
				idx = texhash.at(matInfo.emissiveMap);
				m_texHash[matInfo.emissiveMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasEmissive = true;
			}
			m_perModels.push_back(perModel);
		}
		
	}

	void Mesh11::UploadBuffers(ID3D11DeviceContext* const pContext, UINT i, bool bGeoPass)
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
		if (m_bInstanced)
		{

			if (m_instancedData == nullptr)
			{
				CreateInstanceMatrices();
			}

			ID3D11Buffer* vbs[2] = { m_pVb, m_pInstancePosCb };
			pContext->IASetVertexBuffers(0, 2, vbs, stride, &vOffset);
		}
		else
		{
			pContext->IASetVertexBuffers(0, 1, &m_pVb, stride, &vOffset);
		}
		
		pContext->IASetIndexBuffer(m_pIb, DXGI_FORMAT_R32_UINT, iOffset);
		
		UINT texCnt = 0;
		UINT idx = m_texHash[matInfo.diffuseMap];
		ID3D11ShaderResourceView* nullSRV[5]={nullptr,};
		pContext->PSSetShaderResources(0, 1, &m_texSrvs[idx]);
		if (bGeoPass)
		{	
			if (m_perModels[i].hasNormal)
			{
				idx = m_texHash[matInfo.normalMap];
				pContext->PSSetShaderResources(1, 1, &m_texSrvs[idx]);
			}
			else
			{
				pContext->PSSetShaderResources(1, 1, &nullSRV[1]);
			}

			if (m_perModels[i].hasSpecular)
			{
				idx = m_texHash[matInfo.specularMap];
				pContext->PSSetShaderResources(2, 1, &m_texSrvs[idx]);
			}
			else
			{
				pContext->PSSetShaderResources(2, 1, &nullSRV[2]);
			}
		
			if (m_perModels[i].hasEmissive)
			{
				idx = m_texHash[matInfo.emissiveMap];
				pContext->PSSetShaderResources(3, 1, &m_texSrvs[idx]);
			}
			else
			{
				pContext->PSSetShaderResources(3, 1, &nullSRV[3]);
			}

			
			if (m_perModels[i].hasAlpha)
			{
				idx = m_texHash[matInfo.alphaMap];
				pContext->PSSetShaderResources(4, 1, &m_texSrvs[idx]);
			}
			else
			{
				pContext->PSSetShaderResources(4, 1, &nullSRV[4]);
			}


			hr = pContext->Map(m_pMaterialCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			assert(SUCCEEDED(hr));

			pMaterial = reinterpret_cast<Material*>(mappedResource.pData);
			pMaterial->ambient = matInfo.material.ambient;
			pMaterial->diffuse = matInfo.material.diffuse;
			pMaterial->specular = matInfo.material.specular;
			pMaterial->reflect = matInfo.material.reflect;
			pContext->Unmap(m_pMaterialCb, 0);
			pContext->PSSetConstantBuffers(2, 1, &m_pMaterialCb);


			hr = pContext->Map(m_pPerModelCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			assert(SUCCEEDED(hr));

			pPerModel = reinterpret_cast<PerModel*>(mappedResource.pData);
			pPerModel->hasSpecular = m_perModels[i].hasSpecular;
			pPerModel->hasNormal = m_perModels[i].hasNormal;
			pPerModel->hasAlpha = m_perModels[i].hasAlpha;
			pPerModel->hasEmissive=m_perModels[i].hasEmissive;

			pContext->Unmap(m_pPerModelCb, 0);
			pContext->VSSetConstantBuffers(1, 1, &m_pPerModelCb);
			pContext->PSSetConstantBuffers(1, 1, &m_pPerModelCb);
		}
		return;
	}

	AABB Mesh11::GetGlobalAabb()
	{	
		DirectX::XMMATRIX transform =  GetTransformMatrix(false);
		

		DirectX::XMVECTOR centerV = m_pAABB->GetCenter();
		DirectX::XMVECTOR globalCenterV = DirectX::XMVector3Transform(centerV, transform);
		DirectX::XMFLOAT4 globalCenter;
		DirectX::XMStoreFloat4(&globalCenter, globalCenterV);

		DirectX::XMVECTOR extentsV = m_pAABB->GetExtent();
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

	void Mesh11::UpdateWorldMatrix()
	{
		DirectX::XMMATRIX srMat, osrMat;
		osrMat = XMMatrixMultiply(m_outlinerScaleMat, m_rtMat);
		srMat = XMMatrixMultiply(m_scMat, m_rtMat);

		DirectX::XMMATRIX srtMat = XMMatrixMultiply(srMat, m_trMat);
		DirectX::XMMATRIX osrtMat = XMMatrixMultiply(osrMat, m_trMat);

		m_wMat = DirectX::XMMatrixTranspose(srtMat);
		m_invWMat = DirectX::XMMatrixInverse(nullptr, srtMat);
		m_invWMat = DirectX::XMMatrixTranspose(m_invWMat);
		m_outlinerMat = DirectX::XMMatrixTranspose(osrtMat);
	}
}