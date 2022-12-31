#ifndef MODEL_H
#define MODEL_H
#include <D3D11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct VertexType
{
	XMFLOAT3 position;
	XMFLOAT2 tex;
	XMFLOAT3 norm;
};

class CModel
{
public:
	CModel(VertexType* pVertices,
	unsigned long* pIndices,
	unsigned int vertexCount,
	unsigned int indexCount);
	CModel(const CModel&);
	~CModel();

	bool Init(ID3D11Device* device);
	void UploadBuffers(ID3D11DeviceContext* context);
	void SetTex(ID3D11ShaderResourceView* tex);

	XMMATRIX* GetWorldMatrix()
	{
		return &m_worldMatrix;
	}
	unsigned int GetIndexCount()
	{
		return m_indexCount;
	}
private:

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11ShaderResourceView* m_pShaderResourceView;

	VertexType* m_pVertices;
	unsigned long* m_pIndices;
	unsigned int m_vertexCount;
	unsigned int m_indexCount;
	XMMATRIX m_worldMatrix;
};
#endif