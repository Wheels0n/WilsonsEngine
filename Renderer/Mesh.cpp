
#include <random>
#include <chrono>

#include "Mesh.h"
#include "SubMesh.h"
#include "MatrixBuffer12.h"
#include "HeapManager.h"
namespace wilson 
{
	//Pos를 담는 변수들은 가장 끝 원소로 전체 크기를 담고 있음에 유의
	Mesh::Mesh(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, HeapManager* pHeapManager,
		VertexData* pVertices,
		unsigned long* pIndices,
		UINT nVertex,
		std::vector<unsigned int> subMeshPos,
		std::vector<std::vector<unsigned int>> clusterPos,
		wchar_t* pName,
		std::vector<std::string> matNames)
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
			
			m_vertexCount = nVertex;
			UINT lastR = clusterPos.size() - 1;
			UINT lastC = clusterPos[lastR].size() - 1;
			m_indexCount = clusterPos[lastR][lastC];
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
			m_pMatBuffer = nullptr;
		}

		//Gen MatBuffer
		DirectX::XMMATRIX iMat =  DirectX::XMMatrixIdentity();
		m_pMatBuffer = new MatBuffer12(pDevice, pCommandList, pHeapManager, &iMat, &iMat);
	
		//Gen Name;
		{
			int len = wcslen(pName);
			wchar_t* lpName = new wchar_t[len + 1];
			wcscpy(lpName, pName);
			pName[len] = L'\0';
			std::wstring wstr(lpName);
			m_Name = std::string(wstr.begin(), wstr.end());
			delete[] lpName;
		}
	
		//Gen VB
		{	
			const UINT64 vbSize= sizeof(VertexData) * m_vertexCount;
			pHeapManager->AllocateVertexData((UINT8*)m_pVertexData, vbSize);
			m_vbV = pHeapManager->GetVBV(vbSize, sizeof(VertexData));
		}

		//Gen IB
		{
			const UINT ibSize = sizeof(UINT) * m_indexCount;
			pHeapManager->AllocateIndexData((UINT8*)m_pIndices, ibSize);

			//SubMesh루프
			for (int i = 0; i < m_subIbVs.size(); ++i)
			{
				//Cluster루프
				for (int j = 0; j < m_subIbVs[i].size()-1; ++j)
				{
			
					m_subIbVs[i][j] = pHeapManager->GetIBV(
						sizeof(UINT) * (clusterPos[i][j+1] - clusterPos[i][j]),
						sizeof(UINT) * clusterPos[i][j]);
				}
				
			}
			m_ibV = pHeapManager->GetIBV(sizeof(UINT) * m_indexCount, 0);
			UINT   idx = pHeapManager->GetIndexBufferHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = pHeapManager->GetIndexBufferBlockOffset(idx);
			pHeapManager->SetIndexBufferBlockOffset(idx, curBlockOffset + m_ibV.SizeInBytes);
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

			m_nullSrvs.resize(nTexType);
			for (int i = 0; i < nTexType; ++i)
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

	Mesh::~Mesh()
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

		if (m_pMatBuffer != nullptr)
		{
			delete m_pMatBuffer;
		}

	}

	void Mesh::BindMaterial(std::unordered_map<std::string, int>& mathash, std::vector<MaterialInfo>& matInfos,
		std::unordered_map<std::string, int>& texhash, std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& textures)
	{	
		m_matInfos.reserve(m_matNames.size());
		m_perModels.reserve(m_matNames.size());
		for (int i = 0; i < m_matNames.size(); ++i)
		{
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> texSrvs;
			PerModel perModel = { false, };

			int idx = mathash[m_matNames[i]];
			MaterialInfo matInfo = matInfos[idx];
			m_matInfos.push_back(matInfo);

			UINT nullCount = 0;
			//Diffuse
			idx = texhash[matInfo.diffuseMap];
			m_texHash[matInfo.diffuseMap] = m_texSrvs.size();
			m_texSrvs.push_back(textures[idx]);
			texSrvs.push_back(textures[idx]);

			if (!matInfo.normalMap.empty())
			{
				idx = texhash[matInfo.normalMap];
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
				idx = texhash[matInfo.specularMap];
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
				idx = texhash[matInfo.emissiveMap];
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
				idx = texhash[matInfo.alphaMap];
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
				texSrvs, m_pMatBuffer, perModel, m_matNames[i]);
		}

	}

	void Mesh::SetVBandIB(ID3D12GraphicsCommandList* pCommandList)
	{
		pCommandList->IASetVertexBuffers(0, 1, &m_vbV);
		pCommandList->IASetIndexBuffer(&m_ibV);
	}

	void Mesh::UpdateWorldMatrix()
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
	D3D12_GPU_DESCRIPTOR_HANDLE* Mesh::GetTextureSrv(int matIndex, eTexType texType)
	{
		UINT idx;
		MaterialInfo matInfo = m_matInfos[matIndex];
		switch (texType)
		{
		case Diffuse:
			idx = m_texHash[matInfo.diffuseMap];
			return &m_texSrvs[idx];
			break;
		case Normal:
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
		case Specular:
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
		case Emissive:
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
		case Alpha:
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