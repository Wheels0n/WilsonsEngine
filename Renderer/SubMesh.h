#pragma once
#include <D3D12.h>
#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <dxgicommon.h>
#include "typedef.h"
#include "AABB.h"
#include "Sphere.h"
#include "MatrixBuffer12.h"
namespace wilson
{
	class MatBuffer12;
	class SubMesh
	{
	public:
		bool operator==(const SubMesh subMesh2)
		{
			if (this->m_matName != subMesh2.m_matName)
			{
				return false;
			}

			for (int i = 0; i < subMesh2.m_clusterPos.size(); ++i)
			{
				if (m_clusterPos[i] != subMesh2.m_clusterPos[i])
				{
					return false;
				}
				
			}
			
			return true;
			
		}
		inline Sphere* GetSphere(UINT i) const
		{
			return m_pSpheres[i];
		}

		inline AABB* GetAABB(UINT i) const
		{
			return m_pAABBs[i];
		}
		inline D3D12_VERTEX_BUFFER_VIEW* GetAABBVBV(UINT i)
		{
			return &m_aabbVbvs[i];
		}
		inline std::vector<bool>& GetOcclusionResultVec()
		{
			return m_bOccluded;
		}
		inline MatBuffer12* GetMatBuffer() const
		{
			return m_pMatBuffer;
		}
		inline PerModel* GetPerModel()
		{
			return &m_PerModel;
		}
		inline UINT GetClusterCount()
		{
			return m_clusterPos.size()-1;
		}

		inline UINT GetIndexCount(UINT i)
		{
			return m_clusterPos[i+1]- m_clusterPos[i];
		}
		inline std::string GetMaterialName() const
		{
			return m_matName;
		}

		static bool SortSubMeshByMaterial(const SubMesh* pSubMesh1, const SubMesh* pSubMesh2);
		static bool SortSubMeshByDepth(const SubMesh* pMesh1, const SubMesh* pMesh2);
		SubMesh(HeapManager* pHeapManager,
			D3D12_VERTEX_BUFFER_VIEW, std::vector <D3D12_INDEX_BUFFER_VIEW>, 
			std::vector<unsigned int> clusterPos,
			VertexData* pVertices, unsigned long* pIndices,
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textures,
			MatBuffer12*, PerModel, std::string);
		~SubMesh();
		void SetVBandIB(ID3D12GraphicsCommandList* pCommandList, UINT k);
		void SetTexture(ID3D12GraphicsCommandList* pCommandList, ePass curPass);
		AABB GetGlobalAABB();
	private:
		D3D12_VERTEX_BUFFER_VIEW m_vbV;
		std::vector<D3D12_VERTEX_BUFFER_VIEW> m_aabbVbvs;
		std::vector<D3D12_INDEX_BUFFER_VIEW> m_ibVs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pTextures;
		std::vector<unsigned int> m_clusterPos;
		std::vector<bool>m_bOccluded;
		std::vector<AABB*> m_pAABBs;
		std::vector<Sphere*> m_pSpheres;
		std::string m_matName;

		AABB* m_pAABB;
		Sphere* m_pSphere;
		MatBuffer12* m_pMatBuffer;
		PerModel m_PerModel;

	};
}