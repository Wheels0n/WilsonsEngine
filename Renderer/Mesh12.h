#pragma once 

#include <D3D12.h>

#include <dxgicommon.h>
#include "AABB.h"
#include "Sphere.h"
#include "typedef.h"
namespace wilson {

	using namespace std;
	using namespace DirectX;
	using namespace Microsoft::WRL;

	struct TransformMatrices;
	struct WVPMatrices;
	struct InverseMatrices;

	class HeapManager;
	class MatrixHelper12;
	class SubMesh;
	class Mesh12;
	class TextureWrapper;

	class Meshlet
	{
	public:
		void											AddTris(int t) { m_tris.push_back(t); };
		void											CreateIndices();
		void											CreateAABB(VertexInfo* pVertexData);

		shared_ptr<AABB>								GetAABB() { return m_pAABB; };
		int												GetMeshletIdx() { return m_idx; };
		int												GetNumOfTri() { return m_tris.size(); };
		vector<int>&									GetIndices() { return m_indices; };
		shared_ptr<Mesh12>								GetParentMesh() { return m_pParentMesh.lock(); };
		

		void											SetMeshletIndex(int idx) { m_idx = idx; };
		void											SetParentMesh(shared_ptr<Mesh12> pMesh) { m_pParentMesh = pMesh; };

	private:
		vector<int>										m_tris;
		vector<int>										m_indices;

		int												m_idx;
		shared_ptr<AABB>								m_pAABB;
		weak_ptr<Mesh12>								m_pParentMesh;
	};

	class MeshGroup12;
	class Mesh12 : public enable_shared_from_this<Mesh12>
	{
		private: 
			vector<string>								GetMatNames() const
			{
				return m_matNames;
			}
		public:
			void										SetMatrixDirtyBit(bool dirty);
			bool										operator==(const Mesh12 mesh2)
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
			void										BindMaterial(const unordered_map<string, int>& mathash, const vector<MaterialInfo>& matInfos,
															const unordered_map<string, int>& texhash, const vector<shared_ptr<TextureWrapper>>& texs);

			XMVECTOR*									GetAngle();
			BOOL										GetMatrixDirtyBit();

			UINT										GetTotalIndexCount();
			UINT										GetIndexCount(UINT i);
			UINT										GetMeshLetCount();
			UINT										GetIndexOffset(UINT i);
			UINT										GetNumMaterial();

			PerModel*									GetPerModel(UINT i);
			string&										GetName();

			vector<UINT>&								GetNumIndice();
			vector<UINT>&								GetNumVertexData();
			vector<UINT>&								GetVertexDataPos();
			vector<UINT>&								GetIndicesPos();
			vector<shared_ptr<Meshlet>>&				GetMeshelets();

			shared_ptr<MeshGroup12>						GetParent() { return m_pParent.lock(); };
			D3D12_GPU_DESCRIPTOR_HANDLE*				GetTextureSrv(const UINT matIndex, const eTexType texType);
			shared_ptr<WVPMatrices>						GetWVPMatrices();
			shared_ptr<TransformMatrices>				GetTransformMatrices();
			BOOL* GetHeightOnOff()
			{
				return &m_bHeightOn;
			}
			float* GetHeightScale()
			{
				return &m_heightScale;
			}

			void										PostInitMeshlets();
			void										SetParent(shared_ptr<MeshGroup12>pParent) { m_pParent = pParent; };

			void										UpdateAABB();
			void										UpdateWorldMatrix();
			void										UpdateCompositeMatrix();

			void										UploadVB(ComPtr<ID3D12GraphicsCommandList> pCmdList);
			void										UploadIB(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT i);
			void										UploadTextures(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT i, ePass curPass); 
			void										UploadCompositeMatrix(ComPtr<ID3D12GraphicsCommandList> pCmdList);
			void										UploadGeoPassMatrices(ComPtr<ID3D12GraphicsCommandList> pCmdList);
			void										UpdateParllexMappingParemeters();

														Mesh12(ComPtr<ID3D12Device> pDevice, 
															vector<shared_ptr<Meshlet>> meshlets,
															shared_ptr<VertexInfo[]> pVertices, shared_ptr<ULONG[]> pIndices,
															const vector<UINT> vertexDataPos, const vector<UINT> indicesPos,
															wchar_t* const pName, const vector<string> matNames);
														Mesh12(const Mesh12&) = delete;
														~Mesh12()=default;

		private:
			D3D12_VERTEX_BUFFER_VIEW					m_vbV;
			vector<D3D12_INDEX_BUFFER_VIEW>				m_subIbVs;
			D3D12_INDEX_BUFFER_VIEW						m_ibV;

			string										m_name;
			vector<string>								m_matNames;

			shared_ptr<VertexInfo[]>						m_pVertexData;
			shared_ptr<ULONG[]>							m_pIndices;
			UINT										m_nIndex;
			UINT										m_nVertex;
			vector<UINT>								m_vertexDataPos;
			vector<UINT>								m_indicesPos;
			vector<UINT>								m_nVertexInfo;
			vector<UINT>								m_nIndices;

			weak_ptr<MeshGroup12>						m_pParent;
			vector<shared_ptr<Meshlet>>					m_meshlets;
			vector<MaterialInfo>						m_matInfos;
			vector<PerModel>							m_perModels;
			vector<D3D12_GPU_DESCRIPTOR_HANDLE>			m_nullSrvs;
			vector<D3D12_GPU_DESCRIPTOR_HANDLE>			m_texSrvs;
			unordered_map<string, int>					m_texHash;

			BOOL				m_bMatrixDirty;
			XMVECTOR			m_angleVec;
			
			shared_ptr<TransformMatrices>				m_transfomMatrices;
			shared_ptr<WVPMatrices>						m_wvpMatrices;
			shared_ptr<WVPMatrices>						m_wvptMatrices;
			shared_ptr<WVPMatrices>						m_wvpInvMatrices;
			shared_ptr<CompositeMatrices>				m_compositeMatrices;
			shared_ptr<CompositeMatrices>				m_compositeInvMatrices;

			shared_ptr<AABB>							m_pAABB;

			UINT	m_combindedMatrixKey;
			UINT	m_geoPassMatricesKey;
			UINT	m_heightScaleCbKey;

			BOOL m_bHeightOn;
			float m_heightScale;
	};

}