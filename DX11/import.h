#ifndef EDITOR_IMPORT_H
#define EDITOR_IMPORT_H

#include <Windows.h>
#include <D3DX11tex.h>
#include <fstream>
#include <fbxsdk.h>
#include "Model.h"
namespace wilson {
	class Importer
	{

	public:
		void Clear();
		inline Model* GetModel()
		{
			return m_pModel;
		};
		bool LoadTex(Model* model, LPCWSTR fileName, ID3D11Device* device);
		bool LoadModel(const char* extension,LPCWSTR fileName)
		{
			if (!strcmp(extension, "obj"))
			{
				return LoadOBJ(fileName);
			}
			else if (!strcmp(extension, "fbx"))
			{
				return LoadFbx(fileName);
			}
		}

		Importer();
		~Importer();
	private:
		bool LoadFbx(LPCWSTR fileName);
		bool LoadOBJ(LPCWSTR fileName);
		wchar_t* TokenizeCWSTR(LPCWSTR fileName);
	private:
		Model* m_pModel;

		DirectX::XMFLOAT3* m_pVertexCoord;
		DirectX::XMFLOAT3* m_pNormalVector;
		DirectX::XMFLOAT2* m_pTexCoord;
		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_vertexCount;
		unsigned int m_vertexCoordCount;
		unsigned int m_texCoordCount;
		unsigned int m_normalVectorCount;
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