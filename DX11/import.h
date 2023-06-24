#ifndef EDITOR_IMPORT_H
#define EDITOR_IMPORT_H

#include <Windows.h>
#include <D3DX11tex.h>
#include <fbxsdk.h>
#include <fstream>
#include <unordered_set>
#include "Model.h"
namespace wilson {

	class Importer
	{

	public:
		void Clear();
		inline std::vector<Model*>& GetModel()
		{
			return m_pModelGroup;
		};
		bool LoadTex(Model* model, LPCWSTR fileName, ID3D11Device* pDevice);
		bool LoadModel(const char* extension, LPCWSTR fileName, ID3D11Device* pDevice);

		Importer();
		~Importer();
	private:
		std::streampos GetCnts(LPCWSTR fileName, std::streampos pos, std::string& objName);

		bool LoadFbx(LPCWSTR fileName, ID3D11Device* pDevice);
		void LoadSubOBJ(LPCWSTR fileName, std::streampos pos, ID3D11Device* pDevice, std::string& objName);
		bool LoadOBJ(LPCWSTR fileName, ID3D11Device* pDevice);
		bool LoadMTL(wchar_t* fileName, char* matName, Model* pModel, ID3D11Device* pDevice);
		void GetCurDir(LPCWSTR fileName);
		wchar_t* GetModelName(LPCWSTR fileName);
		wchar_t* GetMTLPath(LPCWSTR fileName, wchar_t* tok);
		bool LoadFbxTex(std::string fileName, FbxSurfaceMaterial* pSurfaceMaterial,
			std::unordered_set<std::string>& texSet , std::vector<TextureData>& texData, ID3D11Device* pDevice);
		Material LoadFbxMaterial(FbxSurfaceMaterial* pSurfaceMaterial);
	private:
		wchar_t* m_curDir, * m_mtlPath, * m_fileName;
		Model* m_pModel;
		std::vector<Model*> m_pModelGroup;
		
		DirectX::XMFLOAT3* m_pVertexVecs;
		DirectX::XMFLOAT3* m_pNormalVecs;
		DirectX::XMFLOAT2* m_pTexVecs;
		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_vertexCount;
		unsigned int m_vertexVecCount;
		unsigned int m_texVecCount;
		unsigned int m_normalVecCount;
		unsigned int m_indexCount;
		unsigned int m_objectCount;
		unsigned int m_texCount;

		ID3D11ShaderResourceView* m_pSRV;

		FbxManager* m_fbxManager;
		FbxIOSettings* m_fbxIOsettings;
		FbxImporter* m_fbxImporter;
	};
}

#endif //EDITOR_IMPORT_H