#pragma once 
#include <DirectXTex.h>
#include "Object12.h"
#include "typedef.h"
namespace wilson {
	class HeapManager;
	class D3D12;
	class Importer12
	{

	public:
		void ClearMesh();
		void ClearObject();
		inline Object12* GetpObject()
		{
			return m_pObject;
		};
		bool LoadTex(LPCWSTR pFileName, ID3D12Device* const pDevice, bool bDiffuse);
		bool LoadObject(const char* pExtension, LPCWSTR pFileName, ID3D12Device* const pDevice);

		Importer12() = delete;
		Importer12(D3D12*);
		~Importer12();
	private:
		void GetCurDir(LPCWSTR pFileName);
		std::streampos GetCnts(LPCWSTR pFileName, const std::streampos pos, std::string& objName);
		wchar_t* GetFileName(LPCWSTR pFileName);
		void GetExtension(char* pDst, const char* pSrc);
		wchar_t* GetMTLPath(LPCWSTR pFileName, wchar_t* pTok);
		bool LoadFbx(LPCWSTR pFileName, ID3D12Device* const pDevice);
		Material LoadFbxMaterial(FbxSurfaceMaterial* const pSurfaceMaterial);
		bool LoadFbxTex(const std::string filePath, FbxSurfaceMaterial* const pSurfaceMaterial, MaterialInfo* const pMatInfo,
			std::vector<std::string>& matNames, ID3D12Device* const pDevice);
		bool LoadMTL(wchar_t* pFileName, ID3D12Device* const pDevice);
		FbxAMatrix GetNodeTransfrom(FbxNode* const pNode);
		bool LoadOBJ(LPCWSTR pFileName, ID3D12Device* const pDevice);
		void LoadSubFbx(FbxMesh* const pMesh, FbxVector4* const pVertices,
			std::vector<UINT>& vertexDataPos, std::vector<UINT>& indicesPos, 
			const std::vector<UINT>& submeshStride, const std::vector<std::string>& matNames, const std::string& str,
			const FbxAMatrix& wMat);
		void LoadSubOBJ(LPCWSTR pFileName, const std::streampos pos, ID3D12Device* const pDevice, const std::string& objName);
		bool TraverseNode(FbxNode* const pNode, ID3D12Device* const pDevice, const std::string& filePath,
			std::unordered_set<FbxSurfaceMaterial*>& obejctMatSet);
	private:
		static ULONG g_nVertexVec;
		static ULONG g_nTexVec;
		static ULONG g_nNormalVec;

		wchar_t* m_pCurDir;
		wchar_t* m_pMtlPath;
		wchar_t* m_pFileName;

		Mesh12* m_pMesh;
		Object12* m_pObject;

		std::vector<Mesh12*> m_pMeshes;
		std::vector<MaterialInfo>m_materialInfos;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pTexSrvs;
		std::vector<ID3D12Resource*> m_pTexs;
		std::unordered_map<std::string, int> m_matHash;
		std::unordered_map<std::string, int> m_texHash;
		std::unordered_map<std::string, eTEX> m_texTypeHash;

		DirectX::XMFLOAT2* m_pTexVecs;
		DirectX::XMFLOAT3* m_pNormalVecs;
		DirectX::XMFLOAT3* m_pTangentVecs;
		DirectX::XMFLOAT3* m_pVertexVecs;

		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_nIndex;
		unsigned int m_nNormalVec;
		unsigned int m_nObject;
		unsigned int m_nTexVec;
		unsigned int m_nVertex;
		unsigned int m_nVertexVec;

		D3D12* m_pD3D12;
		ID3D12Device* m_pDevice;
		ID3D12GraphicsCommandList* m_pCommandList;
		ID3D12GraphicsCommandList* m_pImporterCommandList;
		ID3D12CommandAllocator* m_pImporterCommandAllocator;
		D3D12_GPU_DESCRIPTOR_HANDLE m_Srv;
		HeapManager* m_pHeapManager;

		FbxManager* m_fbxManager;
		FbxIOSettings* m_fbxIOsettings;
		FbxImporter* m_fbxImporter;
	};
}
