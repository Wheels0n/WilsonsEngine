#pragma once 
#include <DirectXTex.h>
#include "Object12.h"
#include "typedef.h"
namespace wilson {

	using namespace Microsoft::WRL;
	using namespace std;
	using namespace DirectX;

	class HeapManager;
	class D3D12;
	class Importer12
	{

		public:
			void											ClearMesh();
			void											ClearObject();
			shared_ptr<MeshGroup12>							GetMesh()
			{
				return m_pObject;
			};
			bool											LoadObject(const char* pExtension, const wchar_t* pPath, ComPtr<ID3D12Device> pDevice);

															Importer12();
															~Importer12();
		private:
			void											GetCurDir(const wchar_t* pAbsPath);
			void											GetFileName(vector<wchar_t>& pFileName);
			void											GetExtension(char* pDst, const char* pSrc);
			FbxAMatrix										GetNodeTransfrom(FbxNode* const pNode);

			bool											LoadFbx(const wchar_t* pAbsPath);
			Material										LoadFbxMaterial(FbxSurfaceMaterial* const pSurfaceMaterial);
			bool											LoadFbxTex(const string filePath, FbxSurfaceMaterial* const pSurfaceMaterial, MaterialInfo* const pMatInfo,
																vector<string>& matNames);
			void											LoadSubFbx(FbxMesh* const pMesh, FbxVector4* const pVertices,
																vector<UINT>& vertexDataPos, vector<UINT>& indicesPos, 
																const vector<UINT>& submeshStride, const vector<string>& matNames, const string& str,
																const FbxAMatrix& wMat);
			bool											TraverseNode(FbxNode* const pNode, const string& filePath,
																unordered_set<FbxSurfaceMaterial*>& obejctMatSet);
		private:
			static ULONG									g_nVertexVec;
			static ULONG									g_nTexVec;
			static ULONG									g_nNormalVec;

			shared_ptr<wchar_t[]>							m_pCurDir;
			wstring											m_pFileName;

			shared_ptr<Mesh12>								m_pMesh;
			shared_ptr<MeshGroup12>							m_pObject;

			vector<shared_ptr<Mesh12>>						m_pMeshes;
			vector<MaterialInfo>							m_materialInfos;
			vector<shared_ptr<TextureWrapper>>				m_pTexs;
			unordered_map<string, int>						m_matHash;
			unordered_map<string, int>						m_texHash;

			shared_ptr<XMFLOAT2[]>							m_pTexVecs;
			shared_ptr<XMFLOAT3[]>							m_pNormalVecs;
			shared_ptr<XMFLOAT3[]>							m_pTangentVecs;
			shared_ptr<XMFLOAT3[]>							m_pVertexVecs;

			shared_ptr<VertexInfo[]>						m_pVertexData;
			shared_ptr<ULONG[]>								m_pIndices;
			UINT											m_nIndex;
			UINT											m_nNormalVec;
			UINT											m_nObject;
			UINT											m_nTexVec;
			UINT											m_nVertex;
			UINT											m_nVertexVec;

			D3D12_GPU_DESCRIPTOR_HANDLE						m_Srv;

			FbxManager*										m_fbxManager;
			FbxIOSettings*									m_fbxIOsettings;
			FbxImporter*									m_fbxImporter;
	};
}
