#pragma once 

#include <D3D12.h>
#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <dxgicommon.h>

#include "AABB.h"
#include "Sphere.h"
#include "SubMesh.h"
#include "typedef.h"
namespace wilson {

	class HeapManager;
	class MatBuffer12;
	class SubMesh;
	class Mesh
	{
	private: 
		std::vector<std::string> GetMatNames() const
		{
			return m_matNames;
		}
	public:
		bool operator==(const Mesh mesh2)
		{
			if (this->m_matNames.size()!= mesh2.m_matNames.size())
			{
				return false;
			}
			else
			{
				for (int i = 0; i < mesh2.m_matNames.size(); ++i)
				{
					if (m_matNames[i] != mesh2.m_matNames[i])
					{
						return false;
					}
				}
			}

			if (this->m_indicesPos.size() != mesh2.m_indicesPos.size())
			{
				return false;
			}
			else
			{
				for (int i = 0; i < mesh2.m_indicesPos.size(); ++i)
				{
					if (m_indicesPos[i] != mesh2.m_indicesPos[i])
					{
						return false;
					}
				}
			}

			return true;
		}
		static bool SortMeshByDepth(const Mesh* pMesh1, const Mesh* pMesh2);
		void UpdateWorldMatrix();
		D3D12_GPU_DESCRIPTOR_HANDLE* GetTextureSrv(int matIndex, eTexType texType);
		void BindMaterial(std::unordered_map<std::string, int>& mathash, std::vector<MaterialInfo>& matInfos,
			std::unordered_map<std::string, int>& texhash, std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& texSrvs);
		AABB GetGlobalAABB();
		void SetVBandIB(ID3D12GraphicsCommandList* pCommandList);
		inline Sphere* GetSphere() const
		{
			return m_pSphere;
		}

		inline AABB* GetAABB() const
		{
			return m_pAABB;
		}
		inline UINT GetMatCount() const
		{
			return m_matInfos.size();
		}
		inline MatBuffer12* GetMatBuffer() const
		{
			return m_pMatBuffer;
		}
		inline std::vector<SubMesh*>& GetSubMeshes()
		{
			return m_pSubMeshs;
		}
		inline DirectX::XMMATRIX GetTransformMatrix(bool bOutliner)
		{
			return bOutliner ? m_outlinerMat : m_wMat;
		}
		inline DirectX::XMMATRIX GetInverseWorldMatrix()
		{
			return m_invWMat;
		}
		inline DirectX::XMMATRIX* GetTranslationMatrix()
		{
			return &m_trMat;
		}
		inline DirectX::XMMATRIX* GetScaleMatrix()
		{
			return &m_scMat;
		}
		inline DirectX::XMMATRIX* GetOutlinerScaleMatrix()
		{
			return &m_outlinerScaleMat;
		}
		inline DirectX::XMMATRIX* GetRoatationMatrix()
		{
			return &m_rtMat;
		}
		inline DirectX::XMVECTOR* GetAngle()
		{
			return &m_angleVec;
		}
		inline PerModel* GetPerModel(UINT i)
		{
			return &m_perModels[i];
		}
		inline UINT GetTotalIndexCount()
		{
			return m_indexCount;
		}
		inline UINT GetIndexCount(UINT i)
		{
			return m_indicesPos[i + 1] - m_indicesPos[i];
		}
		inline UINT GetIndexOffset(UINT i)
		{
			return sizeof(UINT)* m_indicesPos[i];
		}
		inline std::string GetName() const
		{
			return m_Name;
		}

		inline std::vector<unsigned int>& GetNumVertexData()
		{
			return m_numVertexData;
		}
		inline std::vector<unsigned int>& GetNumIndice()
		{
			return m_numIndices;
		}
		inline std::vector<unsigned int>& GetVertexDataPos()
		{
			return m_vertexDataPos;
		}
		inline std::vector<unsigned int>& GetIndicesPos()
		{
			return m_indicesPos;
		}
		Mesh(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, HeapManager* pHeapManager,
			VertexData* pVertices, unsigned long* pIndices,
			std::vector<unsigned int> vertexDataPos,std::vector<unsigned int> indicesPos,
			wchar_t* pName, std::vector<std::string> matNames);
		Mesh(const Mesh&) = delete;
		~Mesh();

	private:
		ID3D12Device* m_pDevice;

		D3D12_VERTEX_BUFFER_VIEW m_vbV;
		std::vector<D3D12_INDEX_BUFFER_VIEW> m_subIbVs;
		D3D12_INDEX_BUFFER_VIEW m_ibV;

		std::string m_Name;
		std::vector<std::string>m_matNames;

		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_vertexCount;
		unsigned int m_indexCount;
		std::vector<unsigned int> m_vertexDataPos;
		std::vector<unsigned int> m_indicesPos;
		std::vector<unsigned int> m_numVertexData;
		std::vector<unsigned int> m_numIndices;

		std::vector<MaterialInfo> m_matInfos;
		std::vector<PerModel>m_perModels;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_texSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_nullSrvs;
		std::vector<ID3D12Resource*> m_pTexs;
		std::unordered_map<std::string, int> m_texHash;

		DirectX::XMMATRIX m_outlinerScaleMat;
		DirectX::XMMATRIX m_outlinerMat;
		DirectX::XMMATRIX m_scMat;
		DirectX::XMMATRIX m_rtMat;
		DirectX::XMMATRIX m_trMat;
		DirectX::XMMATRIX m_wMat;
		DirectX::XMMATRIX m_invWMat;
		DirectX::XMVECTOR m_angleVec;

		AABB* m_pAABB;
		Sphere* m_pSphere;
		MatBuffer12* m_pMatBuffer;
		std::vector<SubMesh*> m_pSubMeshs;

	};

}