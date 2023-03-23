#ifndef EDITOR_IMPORT_H
#define EDITOR_IMPORT_H

#include <Windows.h>
#include <D3DX11tex.h>
#include <fstream>
#include "Model.h"

using namespace DirectX;

class CImporter
{


private:
	Model* m_pCModel;

	XMFLOAT3* m_pVertexCoord; 
	XMFLOAT3* m_pNormalVector;
	XMFLOAT2* m_pTexCoord;
	VertexData* m_pVertices;
	unsigned long* m_pIndices;
	unsigned int m_vertexCount;
	unsigned int m_vertexCoordCount;
	unsigned int m_texCoordCount;
	unsigned int m_normalVectorCount;
	unsigned int m_indexCount;
	unsigned int m_objectCount;
	unsigned int m_texCount;

	ID3D11ShaderResourceView* m_pSRV;

	char* m_plte;
	char* m_pngData;
public:
	CImporter();
	~CImporter();
	void Clear();
	Model* GetModel()
	{
		return m_pCModel;
	};
	bool LoadOBJ(LPCWSTR fileName);
	bool LoadPNG(LPCWSTR fileName, unsigned int* width, unsigned int* height);
	bool LoadTex(Model* model, LPCWSTR fileName, ID3D11Device* device);
};


#endif //EDITOR_IMPORT_H