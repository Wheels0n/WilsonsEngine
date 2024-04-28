#pragma once 

#include <D3D12.h>

#include <dxgicommon.h>

#include "SubMesh.h"
#include "typedef.h"
namespace wilson {

	class HeapManager;
	class MatBuffer12;
	class SubMesh;
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
			if (this->m_matNames.size()!= mesh2.m_matNames.size())
			{
				return false;
			}
			
			for (int i = 0; i < mesh2.m_matNames.size(); ++i)
			{
				if (m_matNames[i] != mesh2.m_matNames[i])
				{
					return false;
				}
			}
			

			if (this->m_clusterPos.size() != mesh2.m_clusterPos.size())
			{
				return false;
			}
			
			for (int i = 0; i < mesh2.m_clusterPos.size(); ++i)
			{	
				for (int j = 0; j < mesh2.m_clusterPos[0].size(); ++j)
				{
					if (m_clusterPos[i][j] != mesh2.m_clusterPos[i][j])
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
		inline DirectX::XMMATRIX GetInverseWorldMatrix()
		{
			return m_invWMat;
		}
		inline MatBuffer12* GetMatBuffer() const
		{
			return m_pMatricesCb;
		}
		inline std::string GetName() const
		{
			return m_name;
		}
		inline UINT GetNumMaterial() const
		{
			return m_matInfos.size();
		}
		inline DirectX::XMMATRIX* GetOutlinerScaleMatrix()
		{
			return &m_outlinerScaleMat;
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
		inline std::vector<SubMesh*>& GetSubMeshes()
		{
			return m_pSubMeshs;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE* GetTextureSrv(const UINT matIndex, const eTexType texType);
		inline UINT GetTotalIndexCount()
		{
			return m_nIndex;
		}
		inline DirectX::XMMATRIX GetTransformMatrix(const bool bOutliner)
		{
			return bOutliner ? m_outlinerMat : m_wMat;
		}
		inline DirectX::XMMATRIX* GetTranslationMatrix()
		{
			return &m_trMat;
		}
		inline std::vector<unsigned int>& GetVertexDataPos()
		{
			return m_subMeshPos;
		}
		void SetVBandIB(ID3D12GraphicsCommandList* const pCommandList);
		void UpdateWorldMatrix();

		Mesh12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandList, HeapManager* const pHeapManager,
			VertexData* const pVertices, unsigned long* const pIndices, const UINT nVertex,
			const std::vector<unsigned int> vertexDataPos, const std::vector<std::vector<unsigned int>> clusterPos,
			wchar_t* const pName, const std::vector<std::string> matNames);
		Mesh12(const Mesh12&) = delete;
		~Mesh12();

	private:
		ID3D12Device* m_pDevice;

		D3D12_VERTEX_BUFFER_VIEW m_vbV;
		std::vector<std::vector<D3D12_INDEX_BUFFER_VIEW>> m_subIbVs;
		D3D12_INDEX_BUFFER_VIEW m_ibV;

		std::string m_name;
		std::vector<std::string>m_matNames;

		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_nIndex;
		unsigned int m_nVertex;
		std::vector<unsigned int> m_subMeshPos;
		std::vector<std::vector<unsigned int>> m_clusterPos;

		std::vector<MaterialInfo> m_matInfos;
		std::vector<PerModel>m_perModels;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_nullSrvs;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_texSrvs;
		std::vector<SubMesh*> m_pSubMeshs;
		std::vector<ID3D12Resource*> m_pTexs;
		std::unordered_map<std::string, int> m_texHash;

		DirectX::XMVECTOR m_angleVec;
		DirectX::XMMATRIX m_invWMat;
		DirectX::XMMATRIX m_outlinerScaleMat;
		DirectX::XMMATRIX m_outlinerMat;
		DirectX::XMMATRIX m_rtMat;
		DirectX::XMMATRIX m_scMat;
		DirectX::XMMATRIX m_trMat;
		DirectX::XMMATRIX m_wMat;

		MatBuffer12* m_pMatricesCb;
		HeapManager* m_pHeapManager;

	};

}