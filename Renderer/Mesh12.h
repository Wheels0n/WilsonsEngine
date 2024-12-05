#pragma once 

#include <D3D12.h>

#include <dxgicommon.h>
#include "AABB.h"
#include "Sphere.h"
#include "typedef.h"
namespace wilson {

	class HeapManager;
	class MatrixHandler12;
	class SubMesh;
	class Mesh12;
	class Meshlet
	{
	public:
		void AddTris(int t) { m_tris.push_back(t); };
		void CreateIndices();
		void CreateAABB(VertexData* pVertexData);

		AABB* GetAABB() { return m_pAABB; };
		int GetMeshletIdx() { return m_idx; };
		int GetNumOfTri() { return m_tris.size(); };
		std::vector<int>& GetIndices() { return m_indices; };
		Mesh12* GetParentMesh() { return m_pParentMesh; };
		

		void SetMeshletIndex(int idx) { m_idx = idx; };
		void SetParentMesh(Mesh12* pMesh) { m_pParentMesh = pMesh; };

	private:
		std::vector<int> m_tris;
		std::vector<int> m_indices;

		int m_idx;
		AABB* m_pAABB;
		Mesh12* m_pParentMesh;
	};

	class Mesh12
	{
	private: 
		std::vector<std::string> GetMatNames() const
		{
			return m_matNames;
		}
	public:
		bool operator==(const Mesh12 mesh2)
		{
			if (this->m_matNames.size() != mesh2.m_matNames.size())
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
		void BindMaterial(const std::unordered_map<std::string, int>& mathash, const std::vector<MaterialInfo>& matInfos,
			const std::unordered_map<std::string, int>& texhash, const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& texSrvs);
		inline DirectX::XMVECTOR* GetAngle()
		{
			return &m_angleVec;
		}
		inline UINT GetIndexCount(UINT i)
		{
			return m_meshlets[i]->GetNumOfTri()*3;
		}
		inline UINT GetMeshLetCount()
		{
			return m_meshlets.size();
		}
		inline UINT GetIndexOffset(UINT i)
		{
			return sizeof(UINT) * m_indicesPos[i];
		}
		inline DirectX::XMMATRIX GetInverseWorldMatrix()
		{
			return m_invWMat;
		}
		inline MatrixHandler12* GetMatBuffer() const
		{
			return m_pMatricesCb.get();
		}
		inline std::string GetName() const
		{
			return m_name;
		}
		inline UINT GetNumMaterial() const
		{
			return m_matInfos.size();
		}
		inline std::vector<unsigned int>& GetNumIndice()
		{
			return m_nIndices;
		}
		inline std::vector<unsigned int>& GetNumVertexData()
		{
			return m_nVertexData;
		}
		inline PerModel* GetPerModel(UINT i)
		{
			return &m_perModels[i];
		}
		inline DirectX::XMMATRIX* GetRoatationMatrix()
		{
			return &m_rtMat;
		}
		inline DirectX::XMMATRIX* GetScaleMatrix()
		{
			return &m_scMat;
		}
		inline Sphere* GetSphere() const
		{
			return m_pSphere.get();
		}
		inline std::vector<unsigned int>& GetVertexDataPos()
		{
			return m_vertexDataPos;
		}
		inline std::vector<unsigned int>& GetIndicesPos()
		{
			return m_indicesPos;
		}
		inline std::vector<Meshlet*>& GetMeshelets()
		{
			return m_meshlets;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE* GetTextureSrv(const UINT matIndex, const eTexType texType);
		inline UINT GetTotalIndexCount()
		{
			return m_nIndex;
		}
		inline DirectX::XMMATRIX& GetTransformMatrix(const bool bTransposed)
		{
			return bTransposed ? m_wtMat : m_wMat;
		}
		inline DirectX::XMMATRIX* GetTranslationMatrix()
		{
			return &m_trMat;
		}

		void UpdateWorldMatrix();
		void UpdateAABB();
		void UploadBuffers(ID3D12GraphicsCommandList* pCommandlist, UINT i, ePass curPass);

		Mesh12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandList, HeapManager* const pHeapManager,
			std::vector<Meshlet*> meshlets,
			VertexData* const pVertices, unsigned long* const pIndices, 
			const std::vector<unsigned int> vertexDataPos, const std::vector<unsigned int> indicesPos,
			wchar_t* const pName, const std::vector<std::string> matNames);
		Mesh12(const Mesh12&) = delete;
		~Mesh12();

	private:
		ID3D12Device* m_pDevice;

		D3D12_VERTEX_BUFFER_VIEW m_vbV;
		std::vector<D3D12_INDEX_BUFFER_VIEW> m_subIbVs;
		D3D12_INDEX_BUFFER_VIEW m_ibV;

		std::string m_name;
		std::vector<std::string>m_matNames;

		std::unique_ptr<VertexData> m_pVertexData;
		std::unique_ptr<unsigned long> m_pIndices;
		unsigned int m_nIndex;
		unsigned int m_nVertex;
		std::vector<unsigned int> m_vertexDataPos;
		std::vector<unsigned int> m_indicesPos;
		std::vector<unsigned int> m_nVertexData;
		std::vector<unsigned int> m_nIndices;

		std::vector<Meshlet*> m_meshlets;
		std::vector<MaterialInfo> m_matInfos;
		std::vector<PerModel>m_perModels;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_nullSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_texSrvs;
		std::unordered_map<std::string, int> m_texHash;

		DirectX::XMVECTOR m_angleVec;
		DirectX::XMMATRIX m_invWMat;
		DirectX::XMMATRIX m_rtMat;
		DirectX::XMMATRIX m_scMat;
		DirectX::XMMATRIX m_trMat;
		DirectX::XMMATRIX m_wMat;
		DirectX::XMMATRIX m_wtMat;

		std::unique_ptr<AABB> m_pAABB;
		std::unique_ptr<Sphere> m_pSphere;
		std::unique_ptr<MatrixHandler12> m_pMatricesCb;

	};

}