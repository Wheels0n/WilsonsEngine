
#include "Mesh12.h"
#include "SubMesh.h"
#include "MatrixBuffer12.h"
#include "HeapManager.h"
namespace wilson 
{
	//Pos를 담는 변수들은 가장 끝 원소로 전체 크기를 담고 있음에 유의
	Mesh12::Mesh12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandList, HeapManager* const pHeapManager,
		VertexData* const pVertices,
		unsigned long* const pIndices,
		const UINT nVertex,
		const std::vector<unsigned int> subMeshPos,
		const std::vector<std::vector<unsigned int>> clusterPos,
		wchar_t* const pName,
		const std::vector<std::string> matNames)
	{
		HRESULT hr;
		//Init Variables
		{
			m_pDevice = pDevice;
			m_pHeapManager = pHeapManager;
			m_pVertexData = pVertices;
			m_pIndices = pIndices;
			
			m_subMeshPos = subMeshPos;
			m_subIbVs.resize(clusterPos.size());

			for (int i = 0; i < clusterPos.size(); ++i)
			{	
				m_subIbVs[i].resize(clusterPos[i].size());
			}
			
			m_nVertex = nVertex;
			UINT lastR = clusterPos.size() - 1;
			UINT lastC = clusterPos[lastR].size() - 1;
			m_nIndex = clusterPos[lastR][lastC];
			m_clusterPos = clusterPos;

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
			m_pMatricesCb = nullptr;
		}

		//Gen MatBuffer11
		DirectX::XMMATRIX iMat =  DirectX::XMMatrixIdentity();
		m_pMatricesCb = new MatBuffer12(pDevice, pCommandList, pHeapManager, &iMat, &iMat);
	
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
			pHeapManager->AllocateVertexData((UINT8*)m_pVertexData, vbSize);
			m_vbV = pHeapManager->GetVbv(vbSize, sizeof(VertexData));
		}

		//Gen IB
		{
			const UINT ibSize = sizeof(UINT) * m_nIndex;
			pHeapManager->AllocateIndexData((UINT8*)m_pIndices, ibSize);

			//SubMesh루프
			for (int i = 0; i < m_subIbVs.size(); ++i)
			{
				//Cluster루프
				for (int j = 0; j < m_subIbVs[i].size()-1; ++j)
				{
			
					m_subIbVs[i][j] = pHeapManager->GetIbv(
						sizeof(UINT) * (clusterPos[i][j+1] - clusterPos[i][j]),
						sizeof(UINT) * clusterPos[i][j]);
				}
				
			}
			m_ibV = pHeapManager->GetIbv(sizeof(UINT) * m_nIndex, 0);
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

		m_pSubMeshs.resize(m_matNames.size());

	}

	Mesh12::~Mesh12()
	{
		for (int i = 0; i < m_pSubMeshs.size(); ++i)
		{
			delete m_pSubMeshs[i];
		}

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
		m_subMeshPos.clear();
		m_texSrvs.clear();
		m_texHash.clear();
		m_matInfos.clear();
		m_matNames.clear();
		m_perModels.clear();

		if (m_pMatricesCb != nullptr)
		{
			delete m_pMatricesCb;
		}

	}

	void Mesh12::BindMaterial(const std::unordered_map<std::string, int>&  mathash, const std::vector<MaterialInfo>& matInfos,
		const std::unordered_map<std::string, int>& texhash, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& textures)
	{	
		m_matInfos.reserve(m_matNames.size());
		m_perModels.reserve(m_matNames.size());
		for (int i = 0; i < m_matNames.size(); ++i)
		{
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> texSrvs;
			PerModel perModel = { false, };

			int idx = mathash.at(m_matNames[i]);
			MaterialInfo matInfo = matInfos[idx];
			m_matInfos.push_back(matInfo);

			UINT nullCount = 0;
			//Diffuse
			idx = texhash.at(matInfo.diffuseMap);
			m_texHash[matInfo.diffuseMap] = m_texSrvs.size();
			m_texSrvs.push_back(textures[idx]);
			texSrvs.push_back(textures[idx]);

			if (!matInfo.normalMap.empty())
			{
				idx = texhash.at(matInfo.normalMap);
				m_texHash[matInfo.normalMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				texSrvs.push_back(textures[idx]);
				perModel.hasNormal = true;
			}
			else
			{
				texSrvs.push_back(m_nullSrvs[nullCount++]);
			}

			if (!matInfo.specularMap.empty())
			{
				idx = texhash.at(matInfo.specularMap);
				m_texHash[matInfo.specularMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasSpecular = true;
				texSrvs.push_back(textures[idx]);
			}
			else
			{
				texSrvs.push_back(m_nullSrvs[nullCount++]);
			}

			if (!matInfo.emissiveMap.empty())
			{
				idx = texhash.at(matInfo.emissiveMap);
				m_texHash[matInfo.emissiveMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				texSrvs.push_back(textures[idx]);
				perModel.hasEmissive = true;
			}
			else
			{
				texSrvs.push_back(m_nullSrvs[nullCount++]);
			}

			if (!matInfo.alphaMap.empty())
			{
				idx = texhash.at(matInfo.alphaMap);
				m_texHash[matInfo.alphaMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				texSrvs.push_back(textures[idx]);
				perModel.hasAlpha = true;
			}
			else
			{
				texSrvs.push_back(m_nullSrvs[nullCount++]);
			}


			m_perModels.push_back(perModel);
			m_pSubMeshs[i] = new SubMesh(m_pHeapManager, 
				m_vbV, m_subIbVs[i], m_clusterPos[i],
				m_pVertexData, m_pIndices,
				texSrvs, m_pMatricesCb, perModel, m_matNames[i]);
		}

	}

	void Mesh12::SetVBandIB(ID3D12GraphicsCommandList* const pCommandList)
	{
		pCommandList->IASetVertexBuffers(0, 1, &m_vbV);
		pCommandList->IASetIndexBuffer(&m_ibV);
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