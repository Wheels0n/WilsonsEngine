#pragma once 
#include <DirectXTex.h>
#include <Windows.h>
#include <fbxsdk.h>
#include <fstream>
#include <unordered_set>

#include "ModelGroup12.h"
#include "typedef.h"
namespace wilson {
	
	class DescriptorHeapManager;
	class D3D12;
	class Importer12
	{

	public:
		void ClearModel();
		void ClearModelGroup();
		inline ModelGroup12* GetModelGroup()
		{
			return m_pModelGroup;
		};
		bool LoadTex(LPCWSTR fileName, ID3D12Device* pDevice, bool isDiffuse);
		bool LoadModel(const char* extension, LPCWSTR fileName, ID3D12Device* pDevice);

		Importer12() = delete;
		Importer12(D3D12*);
		~Importer12();
	private:
		void GetExtension(char* dst, const char* src);
		std::streampos GetCnts(LPCWSTR fileName, std::streampos pos, std::string& objName);

		void LoadSubFbx(FbxMesh* pMesh, FbxVector4* pVertices,
			std::vector<UINT>& vertexDataPos, std::vector<UINT>& indicesPos, std::vector<UINT>& submeshStride, std::vector<std::string>& matNames, std::string& str,
			FbxAMatrix& wMat);
		bool LoadFbx(LPCWSTR fileName, ID3D12Device* pDevice);
		bool TraverseNode(FbxNode* pNode, ID3D12Device* pDevice, std::string& filePath, std::unordered_set<FbxSurfaceMaterial*>& groupMatSet);
		void LoadSubOBJ(LPCWSTR fileName, std::streampos pos, ID3D12Device* pDevice, std::string& objName);
		bool LoadOBJ(LPCWSTR fileName, ID3D12Device* pDevice);
		bool LoadMTL(wchar_t* fileName, ID3D12Device* pDevice);
		FbxAMatrix GetNodeTransfrom(FbxNode* pNode);
		void GetCurDir(LPCWSTR fileName);
		wchar_t* GetFileName(LPCWSTR fileName);
		wchar_t* GetMTLPath(LPCWSTR fileName, wchar_t* tok);
		bool LoadFbxTex(std::string filePath, FbxSurfaceMaterial* pSurfaceMaterial, MaterialInfo* pMatInfo, std::vector<std::string>& matNames, ID3D12Device* pDevice);
		Material LoadFbxMaterial(FbxSurfaceMaterial* pSurfaceMaterial);
	private:
		static ULONG g_vertexVecCount;
		static ULONG g_texVecCount;
		static ULONG g_normalVecCount;

		wchar_t* m_curDir;
		wchar_t* m_mtlPath;
		wchar_t* m_fileName;

		ModelGroup12* m_pModelGroup;
		Model12* m_pModel;

		std::vector<Model12*> m_pModels;
		std::vector<MaterialInfo>m_MaterialInfoV;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_pTexSrvs;
		std::vector<ID3D12Resource*> m_pTexs;
		std::unordered_map<std::string, int> m_matHash;
		std::unordered_map<std::string, int> m_texHash;
		std::unordered_map<std::string, eTEX> m_texTypeHash;

		DirectX::XMFLOAT3* m_pVertexVecs;
		DirectX::XMFLOAT3* m_pNormalVecs;
		DirectX::XMFLOAT2* m_pTexVecs;
		DirectX::XMFLOAT3* m_pTangentVecs;

		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_vertexCount;
		unsigned int m_vertexVecCount;
		unsigned int m_texVecCount;
		unsigned int m_normalVecCount;
		unsigned int m_indexCount;
		unsigned int m_objectCount;

		D3D12* m_pD3D12;
		ID3D12Device* m_pDevice;
		ID3D12GraphicsCommandList* m_pCommandList;
		ID3D12GraphicsCommandList* m_pImporterCommandList;
		ID3D12CommandAllocator* m_pImporterCommandAllocator;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SRV;
		DescriptorHeapManager* m_pDescriptorHeapManager;

		FbxManager* m_fbxManager;
		FbxIOSettings* m_fbxIOsettings;
		FbxImporter* m_fbxImporter;
	};
}
