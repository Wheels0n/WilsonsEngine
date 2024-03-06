
#include <random>
#include <chrono>

#include "Model12.h"
#include "MatrixBuffer12.h"
#include "DescriptorHeapManager.h"
namespace wilson 
{
	//Pos를 담는 변수들은 가장 끝 원소로 전체 크기를 담고 있음에 유의
	Model12::Model12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, DescriptorHeapManager* pDescriptorHeapManager,
		VertexData* pVertices,
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

			m_pVertexData = pVertices;
			m_pIndices = pIndices;
			m_vertexDataPos = vertexDataPos;
			m_indicesPos = indicesPos;
			m_ibVs.resize(indicesPos.size()-1);
			m_vertexCount = vertexDataPos[vertexDataPos.size() - 1];
			m_indexCount = indicesPos[indicesPos.size() - 1];

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

			m_pVB = nullptr;
			m_pIB = nullptr;
			m_pAABB = nullptr;
			m_instancedData = nullptr;
			m_isInstanced = false;
			m_pInstancePosCB = nullptr;
			m_pMatBuffer = nullptr;
			m_pMaterial = nullptr;
		}

		//Gen MatBuffer
		DirectX::XMMATRIX iMat =  DirectX::XMMatrixIdentity();
		m_pMatBuffer = new MatBuffer12(pDevice, pCommandList, pDescriptorHeapManager, &iMat, &iMat);

		//Gen AABB
		{
			DirectX::XMFLOAT3 minAABB(FLT_MAX, FLT_MAX, FLT_MAX);
			DirectX::XMFLOAT3 maxAABB(FLT_MIN, FLT_MIN, FLT_MIN);
			for (UINT i = 0; i < m_vertexCount; ++i)
			{
				minAABB.x = min(minAABB.x, m_pVertexData[i].position.x);
				minAABB.y = min(minAABB.y, m_pVertexData[i].position.y);
				minAABB.z = min(minAABB.z, m_pVertexData[i].position.z);

				maxAABB.x = max(maxAABB.x, m_pVertexData[i].position.x);
				maxAABB.y = max(maxAABB.y, m_pVertexData[i].position.y);
				maxAABB.z = max(maxAABB.z, m_pVertexData[i].position.z);
			}
			m_pAABB = new AABB(minAABB, maxAABB);
		}
	
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

			const UINT vbSize= sizeof(VertexData) * m_vertexCount;

			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC vbDesc = {};
			vbDesc.Width = vbSize;
			vbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			vbDesc.Alignment = 0;
			vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			vbDesc.Format = DXGI_FORMAT_UNKNOWN;
			vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			vbDesc.Height = 1;
			vbDesc.DepthOrArraySize = 1;
			vbDesc.MipLevels = 1;
			vbDesc.SampleDesc.Count = 1;
			vbDesc.SampleDesc.Quality = 0;


			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &vbDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ| D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pVB));
			assert(SUCCEEDED(hr));
			m_pVB->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Model12::m_pVB") - 1, "Model12::m_pVB");


			UINT8* pVB;
			D3D12_RANGE readRange = { 0, };
			hr = m_pVB->Map(0, &readRange, reinterpret_cast<void**>(&pVB));
			assert(SUCCEEDED(hr));
			memcpy(pVB, m_pVertexData, vbSize);
			m_pVB->Unmap(0, 0);

			m_vbV.BufferLocation = m_pVB->GetGPUVirtualAddress();
			m_vbV.SizeInBytes = vbSize; 
			m_vbV.StrideInBytes = sizeof(VertexData);
		}

		//Gen IB
		{	
			const UINT ibSize = sizeof(UINT) * m_indexCount;

			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC ibDesc = {};
			ibDesc.Width = ibSize;
			ibDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			ibDesc.Alignment = 0;
			ibDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			ibDesc.Format = DXGI_FORMAT_UNKNOWN;
			ibDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			ibDesc.Height = 1;
			ibDesc.DepthOrArraySize = 1;
			ibDesc.MipLevels = 1;
			ibDesc.SampleDesc.Count = 1;
			ibDesc.SampleDesc.Quality = 0;


			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &ibDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ| D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(&m_pIB));
			assert(SUCCEEDED(hr));
			m_pIB->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Model12::m_pIB") - 1, "Model12::m_pIB");


			UINT8* pIB;
			D3D12_RANGE readRange = { 0, };
			hr = m_pIB->Map(0, &readRange, reinterpret_cast<void**>(&pIB));
			assert(SUCCEEDED(hr));
			memcpy(pIB, m_pIndices, ibSize);
			m_pIB->Unmap(0, 0);

			for (int i = 0; i < m_ibVs.size(); ++i)
			{
				m_ibVs[i].BufferLocation = m_pIB->GetGPUVirtualAddress()+ sizeof(UINT)*indicesPos[i];
				m_ibVs[i].Format = DXGI_FORMAT_R32_UINT;
				m_ibVs[i].SizeInBytes = sizeof(UINT)*(indicesPos[i+1]-indicesPos[i]);
			}
		}

		//Gen CB
		{
			D3D12_RANGE readRange = { 0, };
			//Gen materialCB
			{

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;

				D3D12_RESOURCE_DESC cbufferDesc = {};
				cbufferDesc.Width = _64KB_ALIGN(sizeof(Material));
				cbufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				cbufferDesc.Alignment = 0;
				cbufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				cbufferDesc.Format = DXGI_FORMAT_UNKNOWN;
				cbufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				cbufferDesc.Height = 1;
				cbufferDesc.DepthOrArraySize = 1;
				cbufferDesc.MipLevels = 1;
				cbufferDesc.SampleDesc.Count = 1;
				cbufferDesc.SampleDesc.Quality = 0;


				UINT constantBufferSize = sizeof(Material);
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ| D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pMaterialCB));
				assert(SUCCEEDED(hr));
				m_pMaterialCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Model12::m_pMaterialCB") - 1, "Model12::m_pMaterialCB");

				hr = m_pMaterialCB->Map(0, &readRange, reinterpret_cast<void**>(&m_pMaterial));
				assert(SUCCEEDED(hr));

				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pMaterialCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
				m_materialCBV = cbvSrvGpuHandle;

				pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
				
			}

			//Gen InstancePosCB
			{
				std::random_device rd;
				std::mt19937 rng(rd());
				std::uniform_real_distribution floatGen(5.0f, 50.0f);
				float x, y, z;
				m_instancedData = new DirectX::XMMATRIX[_MAX_INSTANCES];

				for (int i = 0; i < _MAX_INSTANCES; ++i)
				{
					x = floatGen(rng);
					y = floatGen(rng);
					z = floatGen(rng);
					m_instancedData[i] = DirectX::XMMatrixTranslation(x, y, z);
				}

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;

				D3D12_RESOURCE_DESC cbufferDesc = {};
				cbufferDesc.Width = _64KB_ALIGN(sizeof(DirectX::XMMATRIX) * _MAX_INSTANCES); 
				cbufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				cbufferDesc.Alignment = 0;
				cbufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				cbufferDesc.Format = DXGI_FORMAT_UNKNOWN;
				cbufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				cbufferDesc.Height = 1;
				cbufferDesc.DepthOrArraySize = 1;
				cbufferDesc.MipLevels = 1;
				cbufferDesc.SampleDesc.Count = 1;
				cbufferDesc.SampleDesc.Quality = 0;


				UINT constantBufferSize = sizeof(DirectX::XMMATRIX) * _MAX_INSTANCES;
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ| D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pInstancePosCB));
				assert(SUCCEEDED(hr));
				m_pInstancePosCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Model12::m_pInstancePosCB") - 1, "Model12::m_pInstancePosCB");


				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pMaterialCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
				m_materialCBV = cbvSrvGpuHandle;
				pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}


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

			m_nullSrvs.resize(5);
			for (int i = 0; i < 5; ++i)
			{	
				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				m_pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvCpuHandle);
				m_nullSrvs[i] = cbvSrvGpuHandle;
				pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
			
		}
	}

	Model12::~Model12()
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
		m_texSrvs.clear();
		m_texHash.clear();
		m_matInfos.clear();
		m_matNames.clear();
		m_perModels.clear();

		if (m_pVB != nullptr)
		{
			m_pVB->Release();
			m_pVB = nullptr;
		}

		if (m_pIB != nullptr)
		{
			m_pIB->Release();
			m_pIB = nullptr;
		}

		if (m_pMaterialCB != nullptr)
		{
			m_pMaterialCB->Release();
			m_pMaterialCB = nullptr;
		}

		if (m_pInstancePosCB != nullptr)
		{
			m_pInstancePosCB->Release();
			m_pInstancePosCB = nullptr;
		}

		if (m_instancedData != nullptr)
		{
			delete[] m_instancedData;
		}

		if (m_pAABB != nullptr)
		{
			delete m_pAABB;
		}

		if (m_pMatBuffer != nullptr)
		{
			delete m_pMatBuffer;
		}

	}

	void Model12::BindMaterial(std::unordered_map<std::string, int>& mathash, std::vector<MaterialInfo>& matInfos,
		std::unordered_map<std::string, int>& texhash, std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& textures)
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
			m_texHash[matInfo.diffuseMap] = m_texSrvs.size();
			m_texSrvs.push_back(textures[idx]);

			if (!matInfo.specularMap.empty())
			{
				idx = texhash[matInfo.specularMap];
				m_texHash[matInfo.specularMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasSpecular = true;
			}

			if (!matInfo.normalMap.empty())
			{
				idx = texhash[matInfo.normalMap];
				m_texHash[matInfo.normalMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasNormal = true;
			}

			if (!matInfo.alphaMap.empty())
			{
				idx = texhash[matInfo.alphaMap];
				m_texHash[matInfo.alphaMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasAlpha = true;
			}
			if (!matInfo.emissiveMap.empty())
			{
				idx = texhash[matInfo.emissiveMap];
				m_texHash[matInfo.emissiveMap] = m_texSrvs.size();
				m_texSrvs.push_back(textures[idx]);
				perModel.hasEmissive = true;
			}
			m_perModels.push_back(perModel);
		}

	}

	void Model12::UploadBuffers(ID3D12GraphicsCommandList* pCommandlist, int i, ePass curPass)
	{	
		HRESULT hr;

		//SetVB&IB
		pCommandlist->IASetVertexBuffers(0, 1, &m_vbV);
		pCommandlist->IASetIndexBuffer(&m_ibVs[i]);
		

		//Upload CBV
		if(curPass==eGeoPass)
		{
			MaterialInfo matInfo = m_matInfos[i];
			//Upload materialCB
			{
				memcpy(m_pMaterial, &matInfo.material, sizeof(Material));
				pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsMaterial, m_materialCBV);
			}

			//Set SRV
			{
				const UINT srvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				UINT texCnt = 0;
				UINT idx = m_texHash[matInfo.diffuseMap];
				UINT nIdx = 0;
				//SetDiffuseMap //GPU핸들이 필요하다
				pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsDiffuse, m_texSrvs[idx]);

				
				if (m_perModels[i].hasNormal)
				{
					idx = m_texHash[matInfo.normalMap];
					pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsNormal, m_texSrvs[idx]);
				}
				else
				{
					pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsNormal, m_nullSrvs[nIdx++]);
				}

				if (m_perModels[i].hasSpecular)
				{
					idx = m_texHash[matInfo.specularMap];
					pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsSpecular, m_texSrvs[idx]);
				}
				else
				{
					pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsSpecular, m_nullSrvs[nIdx++]);
				}

				if (m_perModels[i].hasEmissive)
				{
					idx = m_texHash[matInfo.emissiveMap];
					pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsEmissive, m_texSrvs[idx]);
				}
				else
				{
					pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsEmissive, m_nullSrvs[nIdx++]);
				}
					

				if (m_perModels[i].hasAlpha)
				{
					idx = m_texHash[matInfo.alphaMap];
					pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsAlpha, m_texSrvs[idx]);
				}
				else
				{
					pCommandlist->SetGraphicsRootDescriptorTable(ePbrGeo_ePsAlpha, m_nullSrvs[nIdx++]);
				}
				
			}
		}

		return;
	}

	AABB Model12::GetGlobalAABB()
	{
		DirectX::XMMATRIX transform = GetTransformMatrix(false);


		DirectX::XMVECTOR centerV = m_pAABB->GetCenter();
		DirectX::XMVECTOR globalCenterV = DirectX::XMVector3Transform(centerV, transform);
		DirectX::XMFLOAT4 globalCenter;
		DirectX::XMStoreFloat4(&globalCenter, globalCenterV);

		DirectX::XMVECTOR extentsV = m_pAABB->GetExtents();
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

	void Model12::UpdateWorldMatrix()
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
	D3D12_GPU_DESCRIPTOR_HANDLE* Model12::GetTextureSrv(int matIndex, eTexType texType)
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