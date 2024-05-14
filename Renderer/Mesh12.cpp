#include "HeapManager.h"
#include "MatrixBuffer12.h"
#include "Mesh12.h"

namespace wilson 
{
	//Pos를 담는 변수들은 가장 끝 원소로 전체 크기를 담고 있음에 유의
	Mesh12::Mesh12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, HeapManager* pHeapManager,
		VertexData* pVertexData,
		unsigned long* pIndices,
		std::vector<unsigned int> vertexDataPos,
		std::vector<unsigned int> indicesPos,
		wchar_t* pName,
		std::vector<std::string> matNames)
	{
		HRESULT hr;
		//Init Variables
		{
			m_pDevice = pDevice;

			m_pVertexData.reset(pVertexData);
			m_pIndices.reset(pIndices);
			m_vertexDataPos = vertexDataPos;
			m_indicesPos = indicesPos;
			m_subIbVs.resize(indicesPos.size() - 1);
			m_nVertex = vertexDataPos[vertexDataPos.size() - 1];
			m_nIndex = indicesPos[indicesPos.size() - 1];

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

			m_pAABB = nullptr;
			m_pMatricesCb = nullptr;
		}

		//Gen MatBuffer12
		DirectX::XMMATRIX iMat =  DirectX::XMMatrixIdentity();
		m_pMatricesCb = std::make_unique<MatBuffer12>(pDevice, pCommandList, pHeapManager, &iMat, &iMat);
		//Gen AABB
		{
			DirectX::XMFLOAT3 minAABB(FLT_MAX, FLT_MAX, FLT_MAX);
			DirectX::XMFLOAT3 maxAABB(FLT_MIN, FLT_MIN, FLT_MIN);
			for (UINT i = 0; i < m_nVertex; ++i)
			{
				minAABB.x = min(minAABB.x, pVertexData[i].position.x);
				minAABB.y = min(minAABB.y, pVertexData[i].position.y);
				minAABB.z = min(minAABB.z, pVertexData[i].position.z);

				maxAABB.x = max(maxAABB.x, pVertexData[i].position.x);
				maxAABB.y = max(maxAABB.y, pVertexData[i].position.y);
				maxAABB.z = max(maxAABB.z, pVertexData[i].position.z);
			}
			m_pAABB = std::make_unique<AABB>(minAABB, maxAABB);

			DirectX::XMFLOAT3 center((maxAABB.x + minAABB.x) * 0.5f,
				(maxAABB.y + minAABB.y) * 0.5f, (maxAABB.z + minAABB.z) * 0.5f);
			DirectX::XMFLOAT3 len((minAABB.x - maxAABB.x),
				(minAABB.y - maxAABB.y), (minAABB.z - maxAABB.z));
			DirectX::XMVECTOR lenV = DirectX::XMLoadFloat3(&len);
			lenV = DirectX::XMVector4Length(lenV);
			m_pSphere = std::make_unique<Sphere>(center, lenV.m128_f32[0]);
		}
		//Gen Name;
		{
			int len = wcslen(pName);
			wchar_t* lpName = new wchar_t[len + 1];
			wcscpy(lpName, pName);
			pName[len] = L'\0';
			std::wstring wstr(lpName);
			m_name = std::string(wstr.begin(), wstr.end());
			delete[] lpName;
		}
	
		//Gen VB
		{	
			const UINT64 vbSize= sizeof(VertexData) * m_nVertex;
			pHeapManager->AllocateVertexData(reinterpret_cast<UINT8*>(pVertexData), vbSize);
			m_vbV = pHeapManager->GetVbv(vbSize, sizeof(VertexData));
		}

		//Gen IB
		{
			const UINT ibSize = sizeof(UINT) * m_nIndex;
			pHeapManager->AllocateIndexData(reinterpret_cast<UINT8*>(pIndices), ibSize);

			for (int i = 0; i < m_subIbVs.size(); ++i)
			{
				m_subIbVs[i] = pHeapManager->GetIbv(
					sizeof(UINT) * (indicesPos[i + 1] - indicesPos[i]),
					sizeof(UINT) * indicesPos[i]);
			}
			m_ibV = pHeapManager->GetIbv(sizeof(UINT) * indicesPos[indicesPos.size() - 1], 0);
			UINT   idx = pHeapManager->GetIbHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = pHeapManager->GetIbBlockOffset(idx);
			pHeapManager->SetIbBlockOffset(idx, curBlockOffset + m_ibV.SizeInBytes);
		}


		//Gen nullSRV
		{	
			D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
			nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			nullSrvDesc.Texture2D.MipLevels = 1;
			nullSrvDesc.Texture2D.MostDetailedMip = 0;
			nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			m_nullSrvs.resize(static_cast<UINT>(eTexType::cnt));
			for (int i = 0; i < static_cast<UINT>(eTexType::cnt); ++i)
			{	
				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pHeapManager->GetCurCbvSrvGpuHandle();
				m_pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvCpuHandle);
				m_nullSrvs[i] = cbvSrvGpuHandle;
				pHeapManager->IncreaseCbvSrvHandleOffset();
			}
			
		}

	}

	Mesh12::~Mesh12()
	{
	}

	void Mesh12::BindMaterial(const std::unordered_map<std::string, int>&  mathash, const std::vector<MaterialInfo>& matInfos,
		const std::unordered_map<std::string, int>& texhash, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& textures)
	{	
		m_matInfos.reserve(m_matNames.size());
		m_perModels.reserve(m_matNames.size());
		for (int i = 0; i < m_matNames.size(); ++i)
		{
			PerModel perModel = { false, };
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
	void Mesh12::UploadBuffers(ID3D12GraphicsCommandList* pCommandlist, UINT i, ePass curPass)
	{
		HRESULT hr;
		MaterialInfo matInfo = m_matInfos[i];
		//Upload CBV
		switch (curPass)
		{
		case wilson::ePass::zPass:
			pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psDiffuse), m_texSrvs[m_texHash[matInfo.diffuseMap]]);
			pCommandlist->IASetVertexBuffers(0, 1, &m_vbV);
			pCommandlist->IASetIndexBuffer(&m_subIbVs[i]);
			break;
		case wilson::ePass::cubeShadowPass:
		{
			MaterialInfo matInfo = m_matInfos[i];
			const UINT srvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			UINT texCnt = 0;
			UINT idx = m_texHash[matInfo.diffuseMap];
			UINT nIdx = 0;
			pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(eCubeShadowRP::psDiffuseMap), m_texSrvs[idx]);
			pCommandlist->IASetVertexBuffers(0, 1, &m_vbV);
			pCommandlist->IASetIndexBuffer(&m_ibV);
		}
			break;
		case wilson::ePass::geoPass:
		{
			//SetVB&IB
			pCommandlist->IASetVertexBuffers(0, 1, &m_vbV);
			pCommandlist->IASetIndexBuffer(&m_subIbVs[i]);

			//Set SRV
			{
				const UINT srvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				UINT texCnt = 0;
				UINT idx = m_texHash[matInfo.diffuseMap];
				UINT nIdx = 0;
				//SetDiffuseMap //GPU핸들이 필요하다
				pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psDiffuse), m_texSrvs[idx]);


				if (m_perModels[i].hasNormal)
				{
					idx = m_texHash[matInfo.normalMap];
					pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psNormal), m_texSrvs[idx]);
				}
				else
				{
					pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psNormal), m_nullSrvs[nIdx++]);
				}

				if (m_perModels[i].hasSpecular)
				{
					idx = m_texHash[matInfo.specularMap];
					pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psSpecular), m_texSrvs[idx]);
				}
				else
				{
					pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psSpecular), m_nullSrvs[nIdx++]);
				}

				if (m_perModels[i].hasEmissive)
				{
					idx = m_texHash[matInfo.emissiveMap];
					pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psEmissive), m_texSrvs[idx]);
				}
				else
				{
					pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psEmissive), m_nullSrvs[nIdx++]);
				}


				if (m_perModels[i].hasAlpha)
				{
					idx = m_texHash[matInfo.alphaMap];
					pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psAlpha), m_texSrvs[idx]);
				}
				else
				{
					pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psAlpha), m_nullSrvs[nIdx++]);
				}

			}
		}
			break;
		default:
			pCommandlist->IASetVertexBuffers(0, 1, &m_vbV);
			pCommandlist->IASetIndexBuffer(&m_ibV);
			break;
		}

		return;
	}

	AABB Mesh12::GetGlobalAABB()
	{
		DirectX::XMMATRIX transform = GetTransformMatrix(false);


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
	void Mesh12::UpdateWorldMatrix()
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
	D3D12_GPU_DESCRIPTOR_HANDLE* Mesh12::GetTextureSrv(const UINT matIndex, const eTexType texType)
	{
		UINT idx;
		MaterialInfo matInfo = m_matInfos[matIndex];
		switch (texType)
		{
		case eTexType::diffuse:
			idx = m_texHash[matInfo.diffuseMap];
			return &m_texSrvs[idx];
			break;
		case eTexType::normal:
			if (m_perModels[matIndex].hasNormal)
			{
				idx = m_texHash[matInfo.normalMap];
				return &m_texSrvs[idx];
			}
			else
			{
				return &m_nullSrvs[0];
			}
			break;
		case eTexType::specular:
			if (m_perModels[matIndex].hasSpecular)
			{
				idx = m_texHash[matInfo.specularMap];
				return &m_texSrvs[idx];
			}
			else
			{
				return &m_nullSrvs[matIndex];
			}
			break;
		case eTexType::emissive:
			if (m_perModels[matIndex].hasEmissive)
			{
				idx = m_texHash[matInfo.emissiveMap];
				return &m_texSrvs[idx];
			}
			else
			{
				return &m_nullSrvs[0];
			}

			break;
		case eTexType::alpha:
			if (m_perModels[matIndex].hasAlpha)
			{
				idx = m_texHash[matInfo.alphaMap];
				return &m_texSrvs[idx];
			}
			else
			{
				return &m_nullSrvs[0];
			}
			break;
		default:
			return &m_nullSrvs[matIndex];
		}
	}

}