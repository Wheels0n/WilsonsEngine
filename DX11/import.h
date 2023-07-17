#ifndef EDITOR_IMPORT_H
#define EDITOR_IMPORT_H

#include <Windows.h>
#include <D3DX11tex.h>
#include <fbxsdk.h>
#include <fstream>
#include <unordered_set>
#include "ModelGroup.h"
namespace wilson {
	enum ETEX
	{
		Kd,
		Ks,
		Bump,
		d
	};
	class Importer
	{

	public:
		void ClearModel();
		void ClearModelGroup();
		inline ModelGroup* GetModelGroup()
		{
			return m_pModelGroup;
		};
		bool LoadTex(LPCWSTR fileName, ID3D11Device* pDevice, bool isDiffuse);
		bool LoadModel(const char* extension, LPCWSTR fileName, ID3D11Device* pDevice);

		Importer();
		~Importer();
	private:
		std::streampos GetCnts(LPCWSTR fileName, std::streampos pos, std::string& objName);

		bool LoadFbx(LPCWSTR fileName, ID3D11Device* pDevice);
		void LoadSubOBJ(LPCWSTR fileName, std::streampos pos, ID3D11Device* pDevice, std::string& objName);
		bool LoadOBJ(LPCWSTR fileName, ID3D11Device* pDevice);
		bool LoadMTL(wchar_t* fileName, ID3D11Device* pDevice);
		void GetCurDir(LPCWSTR fileName);
		wchar_t* GetFileName(LPCWSTR fileName);
		wchar_t* GetMTLPath(LPCWSTR fileName, wchar_t* tok);
		bool LoadFbxTex(std::string fileName, FbxSurfaceMaterial* pSurfaceMaterial,
			std::unordered_set<std::string>& texSet , std::vector<TextureData>& texData, ID3D11Device* pDevice);
		MaterialInfo LoadFbxMaterial(FbxSurfaceMaterial* pSurfaceMaterial);
	private:
		wchar_t* m_curDir, * m_mtlPath, * m_fileName;
		ModelGroup* m_pModelGroup;
		Model* m_pModel;

		std::vector<Model*> m_pModels;
		std::vector<MaterialInfo>m_Materials;
		std::vector<ID3D11ShaderResourceView*> m_pTexMaps;
		std::unordered_map<std::string, int> m_matHash;
		std::unordered_map<std::string, int> m_texHash;
		std::unordered_map<std::string, ETEX> m_texTypeHash;

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

		ID3D11ShaderResourceView* m_pSRV;

		FbxManager* m_fbxManager;
		FbxIOSettings* m_fbxIOsettings;
		FbxImporter* m_fbxImporter;
	};
}

#endif //EDITOR_IMPORT_H