#pragma once 
#include <DirectXTex.h>
#include "typedef.h"
#include "Object11.h"
namespace wilson {
	class Importer11
	{

	public:
		void ClearMesh();
		void ClearObject();
		inline Object11* GetpObject()
		{
			return m_pObject;
		};
		bool LoadObject(const char* pExtension, LPCWSTR pFileName, ID3D11Device*const pDevice);
		bool LoadTex(LPCWSTR pFileName, ID3D11Device* const pDevice, bool bDiffuse);

		Importer11()=delete;
		Importer11(ID3D11Device* const pDevice);
		~Importer11();
	private:
		std::streampos GetCnts(LPCWSTR pFileName, std::streampos pos, std::string& objName);
		void GetCurDir(LPCWSTR);
		void GetExtension(char* pDst, const char* pSrc);
		wchar_t* GetFileName(LPCWSTR pFileName);
		wchar_t* GetMTLPath(LPCWSTR pFileName, wchar_t* pTok);
		FbxAMatrix GetNodeTransfrom(FbxNode*const pNode);
		bool LoadFbx(LPCWSTR pFileName, ID3D11Device*const pDevice);
		Material LoadFbxMaterial(FbxSurfaceMaterial* pSurfaceMaterial);
		bool LoadFbxTex(const std::string filePath, FbxSurfaceMaterial*const pSurfaceMaterial,
			MaterialInfo*const pMatInfo, std::vector<std::string>& matNames, ID3D11Device*const pDevice);
		bool LoadMTL(wchar_t* pFileName, ID3D11Device*const pDevice);
		bool LoadOBJ(LPCWSTR pFileName, ID3D11Device*const pDevice);
		void LoadSubFbx(FbxMesh*const pMesh, FbxVector4* pVertices,
			std::vector<UINT>& vertexDataPos, std::vector<UINT>& indicesPos, 
			const std::vector<UINT>& submeshStride, const std::vector<std::string>& matNames, const std::string& str,
			const FbxAMatrix& wMat);
		void LoadSubOBJ(LPCWSTR pFileName, std::streampos pos, ID3D11Device* const pDevice, const std::string& objName);
		bool TraverseNode(FbxNode* const pNode, ID3D11Device* const pDevice, const std::string& filePath, std::unordered_set<FbxSurfaceMaterial*>& objectMatSet);

	private:
		static ULONG g_nNormalVec;
		static ULONG g_nTexVec;
		static ULONG g_nVertexVec;

		wchar_t* m_pCurDir;
		wchar_t* m_pFileName;
		wchar_t* m_pMtlPath;

		Object11* m_pObject;
		Mesh11* m_pMesh;

		std::vector<Mesh11*> m_pMeshes;
		std::vector<MaterialInfo>m_materialInfos;
		std::vector<ID3D11ShaderResourceView*> m_pTexSrvs;
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

		ID3D11Device* m_pDevice;
		ID3D11ShaderResourceView* m_pSrv;

		FbxManager* m_fbxManager;
		FbxImporter* m_fbxImporter;
		FbxIOSettings* m_fbxIOsettings;
	};
}
