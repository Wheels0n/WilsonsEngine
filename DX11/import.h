#ifndef EDITOR_IMPORT_H
#define EDITOR_IMPORT_H

#include <Windows.h>
#include <DirectXMath.h>
#include <fstream>
#include <vector>
#include <string>


using namespace DirectX;

struct VertexType
{
	XMFLOAT3 position;
	XMFLOAT2 tex;
	XMFLOAT3 norm;
};

class CImporter
{


private:
	std::vector<XMFLOAT3*> m_pVertexCoordinates, m_pNormalVectors;
	std::vector<XMFLOAT2*> m_pTexCoordinates;
	std::vector<VertexType*> m_pVertices;
	std::vector<unsigned long*> m_pIndices;
	std::vector<unsigned int> m_vertexCounts, m_vertexCoordCounts, m_texCoordCounts, m_normalVectorCounts, m_indexCounts;
	unsigned int m_objectCount;
	unsigned int m_texCount;


	char* m_plte;
	char* m_pngData;
public:
	CImporter();
	~CImporter();
	bool LoadOBJ(LPCWSTR fileName);
	bool LoadPNG(LPCWSTR fileName, unsigned int* width, unsigned int* height);
};


#endif //EDITOR_IMPORT_H