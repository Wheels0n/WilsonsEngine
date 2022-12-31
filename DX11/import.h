#ifndef EDITOR_IMPORT_H
#define EDITOR_IMPORT_H

#include <Windows.h>
#include <D3D11.h>
#include <D3DX11tex.h>
#include <DirectXMath.h>
#include <fstream>
#include <string>
#include "Model.h"

using namespace DirectX;

class CImporter
{


private:
	CModel* m_pCModel;

	XMFLOAT3* m_pVertexCoord; 
	XMFLOAT3* m_pNormalVector;
	XMFLOAT2* m_pTexCoord;
	VertexType* m_pVertices;
	unsigned long* m_pIndices;
	unsigned int m_vertexCount;
	unsigned int m_vertexCoordCount;
	unsigned int m_texCoordCount;
	unsigned int m_normalVectorCount;
	unsigned int m_indexCount;
	unsigned int m_objectCount;
	unsigned int m_texCount;

	ID3D11ShaderResourceView* m_pShaderResourceView;

	char* m_plte;
	char* m_pngData;
public:
	CImporter();
	~CImporter();
	void Clear();
	CModel* GetModel()
	{
		return m_pCModel;
	};
	bool LoadOBJ(LPCWSTR fileName);
	bool LoadPNG(LPCWSTR fileName, unsigned int* width, unsigned int* height);
	bool LoadTex(CModel* model, LPCWSTR fileName, ID3D11Device* device);
};


#endif //EDITOR_IMPORT_H