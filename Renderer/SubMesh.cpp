#include "SubMesh.h"

namespace wilson
{
	bool SubMesh::SortSubMeshByMaterial(const SubMesh* pSubMesh1, const SubMesh* pSubMesh2)
	{
		
		return pSubMesh1->GetMaterialName() < pSubMesh2->GetMaterialName();
		
	}
	SubMesh::SubMesh(D3D12_VERTEX_BUFFER_VIEW vbv, D3D12_INDEX_BUFFER_VIEW ibv, UINT indexCount,
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textures, MatBuffer12* pMatBuffer, PerModel perModel, std::string matName)
	{
		m_vbV = vbv;
		m_ibV = ibv;
		m_pTextures = textures;
		m_pMatBuffer = pMatBuffer;
		m_PerModel = perModel;
		m_indexCount = indexCount;
		m_matName = matName;
	}

	void SubMesh::SetVBandIB(ID3D12GraphicsCommandList* pCommandList)
	{
		pCommandList->IASetVertexBuffers(0, 1, &m_vbV);
		pCommandList->IASetIndexBuffer(&m_ibV);
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

}

