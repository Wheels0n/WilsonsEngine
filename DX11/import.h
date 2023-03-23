#ifndef EDITOR_IMPORT_H
#define EDITOR_IMPORT_H

#include <Windows.h>
#include <D3DX11tex.h>
#include <fstream>
#include "Model.h"

namespace wilson {
	class Importer
	{
	private:
		Model* m_pModel;

		DirectX::XMFLOAT3* m_pVertexCoord;
		DirectX::XMFLOAT3* m_pNormalVector;
		DirectX::XMFLOAT2* m_pTexCoord;
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

	public:
		void Clear();
		inline Model* GetModel()
		{
			return m_pModel;
		};
		bool LoadOBJ(LPCWSTR fileName);
		bool LoadTex(Model* model, LPCWSTR fileName, ID3D11Device* device);

		Importer();
		~Importer();
	};
}

#endif //EDITOR_IMPORT_H