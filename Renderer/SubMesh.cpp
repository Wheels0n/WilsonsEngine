#include "SubMesh.h"
#include "HeapManager.h"
namespace wilson
{
	bool SubMesh::SortSubMeshByMaterial(const SubMesh* pSubMesh1, const SubMesh* pSubMesh2)
	{
		
		return pSubMesh1->GetMaterialName() < pSubMesh2->GetMaterialName();
		
	}
	SubMesh::SubMesh(HeapManager* pHeapManager,
		D3D12_VERTEX_BUFFER_VIEW vbv, std::vector<D3D12_INDEX_BUFFER_VIEW> ibvs, 
		std::vector<unsigned int> clusterPos, 
		VertexData* pVertexData, unsigned long* pIndices,
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textures, MatBuffer12* pMatBuffer, PerModel perModel, std::string matName)
	{
		m_vbV = vbv;
		m_ibVs = ibvs;
		m_pTextures = textures;
		m_pMatBuffer = pMatBuffer;
		m_PerModel = perModel;
		m_clusterPos = clusterPos;
		m_matName = matName;

		m_pAABBs.resize(clusterPos.size()-1);
		m_aabbVbvs.resize(m_pAABBs.size());
		m_pSpheres.resize(clusterPos.size()-1);
		m_bOccluded.resize(clusterPos.size()-1);
		//Gen AABB
		{
			//정렬용 전체 SubMesh바운딩박스 생성
			{

				DirectX::XMFLOAT3 minAABB(FLT_MAX, FLT_MAX, FLT_MAX);
				DirectX::XMFLOAT3 maxAABB(FLT_MIN, FLT_MIN, FLT_MIN);
				UINT idxCnt = m_clusterPos[clusterPos.size() - 1];
				for (UINT i = m_clusterPos[0]; i < idxCnt; ++i)
				{
					UINT idx = pIndices[i];
					minAABB.x = min(minAABB.x, pVertexData[idx].position.x);
					minAABB.y = min(minAABB.y, pVertexData[idx].position.y);
					minAABB.z = min(minAABB.z, pVertexData[idx].position.z);

					maxAABB.x = max(maxAABB.x, pVertexData[idx].position.x);
					maxAABB.y = max(maxAABB.y, pVertexData[idx].position.y);
					maxAABB.z = max(maxAABB.z, pVertexData[idx].position.z);
					
				}
				m_pAABB = new AABB(minAABB, maxAABB);

				DirectX::XMFLOAT3 center((maxAABB.x + minAABB.x) * 0.5f,
					(maxAABB.y + minAABB.y) * 0.5f, (maxAABB.z + minAABB.z) * 0.5f);
				DirectX::XMFLOAT3 len((minAABB.x - maxAABB.x),
					(minAABB.y - maxAABB.y), (minAABB.z - maxAABB.z));
				DirectX::XMVECTOR lenV = DirectX::XMLoadFloat3(&len);
				lenV = DirectX::XMVector4Length(lenV);
				m_pSphere = new Sphere(center, lenV.m128_f32[0]);
			}
			
			//컬링용 클러스터 바운딩박스 생성
			for (UINT i = 0; i < clusterPos.size()-1; ++i)
			{	
				DirectX::XMFLOAT3 minAABB(FLT_MAX, FLT_MAX, FLT_MAX);
				DirectX::XMFLOAT3 maxAABB(FLT_MIN, FLT_MIN, FLT_MIN);
				for (int j = clusterPos[i]; j < clusterPos[i + 1]; ++j)
				{
					UINT idx = pIndices[j];
					minAABB.x = min(minAABB.x, pVertexData[idx].position.x);
					minAABB.y = min(minAABB.y, pVertexData[idx].position.y);
					minAABB.z = min(minAABB.z, pVertexData[idx].position.z);

					maxAABB.x = max(maxAABB.x, pVertexData[idx].position.x);
					maxAABB.y = max(maxAABB.y, pVertexData[idx].position.y);
					maxAABB.z = max(maxAABB.z, pVertexData[idx].position.z);
				}
				
				m_pAABBs[i] = new AABB(minAABB, maxAABB);

				pHeapManager->AllocateVertexData((UINT8*)(m_pAABBs[i]->GetVertices()), sizeof(XMFLOAT3) * 8);
				m_aabbVbvs[i] = pHeapManager->GetVBV(sizeof(XMFLOAT3) * 8, sizeof(XMFLOAT3));

				DirectX::XMFLOAT3 center((maxAABB.x + minAABB.x) * 0.5f,
					(maxAABB.y + minAABB.y) * 0.5f, (maxAABB.z + minAABB.z) * 0.5f);
				DirectX::XMFLOAT3 len((minAABB.x - maxAABB.x),
					(minAABB.y - maxAABB.y), (minAABB.z - maxAABB.z));
				DirectX::XMVECTOR lenV = DirectX::XMLoadFloat3(&len);
				lenV = DirectX::XMVector4Length(lenV);
				m_pSpheres[i] = new Sphere(center, lenV.m128_f32[0]);
			}
		}
	}

	SubMesh::~SubMesh()
	{
		for (int i = 0; i < m_pAABBs.size(); ++i)
		{
			if (m_pAABBs[i] != nullptr)
			{
				delete m_pAABBs[i];
			}
			if (m_pSpheres[i] != nullptr)
			{
				delete m_pSpheres[i];
			}
		}
		
	}

	void SubMesh::SetVBandIB(ID3D12GraphicsCommandList* pCommandList, UINT k)
	{
		pCommandList->IASetVertexBuffers(0, 1, &m_vbV);
		pCommandList->IASetIndexBuffer(&m_ibVs[k]);
	}

	void SubMesh::SetTexture(ID3D12GraphicsCommandList* pCommandList, ePass curPass)
	{
		//Upload CBV
		if (curPass == eGeoPass)
		{
			//Set SRV
			{
				for (int i = 0; i < m_pTextures.size(); ++i)
				{
					//PbrGeo에서는 Diffuse가 1
					pCommandList->SetGraphicsRootDescriptorTable(i + 1, m_pTextures[i]);
				}

			}
		}
		else if (curPass == eCubeShadowPass)
		{
			pCommandList->SetGraphicsRootDescriptorTable(eCubeShadow_ePsDiffuseMap, m_pTextures[Diffuse]);
		}

	}
	AABB SubMesh::GetGlobalAABB()
	{
		MatrixBuffer matBuffer = m_pMatBuffer->GetMatrixBuffer();
		DirectX::XMMATRIX transform = matBuffer.worldMat;

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
	bool SubMesh::SortSubMeshByDepth(const SubMesh* pSubMesh1, const SubMesh* pSubMesh2)
	{
		MatBuffer12* pMatBuffer1 = pSubMesh1->GetMatBuffer();
		DirectX::XMMATRIX mat1 = pMatBuffer1->GetWVPMatrix();
		mat1 = XMMatrixTranspose(mat1);
		Sphere* pSphere1 = pSubMesh1->m_pSphere;
		DirectX::XMFLOAT3 center = pSphere1->GetCenter();
		DirectX::XMVECTOR center1 = XMLoadFloat3(&center);
		center1 = XMVector4Transform(center1, mat1);
		DirectX::XMFLOAT4 ndc1;
		DirectX::XMStoreFloat4(&ndc1, center1);
		ndc1.z /= ndc1.w;

		MatBuffer12* pMatBuffer2 = pSubMesh2->GetMatBuffer();
		DirectX::XMMATRIX mat2 = pMatBuffer2->GetWVPMatrix();
		mat2 = XMMatrixTranspose(mat2);
		Sphere* pSphere2 = pSubMesh2->m_pSphere;
		center = pSphere2->GetCenter();
		DirectX::XMVECTOR center2 = XMLoadFloat3(&center);
		center2 = XMVector4Transform(center2, mat2);
		DirectX::XMFLOAT4 ndc2;
		XMStoreFloat4(&ndc2, center2);
		ndc2.z /= ndc2.w;

		return ndc1.z < ndc2.z;
	}
}

