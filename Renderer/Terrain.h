#ifndef TERRAIN_H
#define TERRAIN_H

#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;

class Terrain
{
private:
	struct GridType
	{
		XMFLOAT3 pos;
		XMFLOAT4 col;
	};
public:
	Terrain();
	Terrain(const Terrain&) = default;
	~Terrain();

	bool Init(ID3D11Device*, int, int);
	void UploadBuffers(ID3D11DeviceContext*);

	int GetIndexCount()
	{
		return m_indexCount;
	}
private:
	int m_terrainWidth;
	int m_terrainHeight;
	int m_vertexCount;
	int m_indexCount;
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

};

#endif // !TERRAIN_H

