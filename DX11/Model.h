#ifndef MODEL_H
#define MODEL_H
#include <D3D11.h>
#include <DirectXMath.h>
#include <string>

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
	unsigned int indexCount,
	wchar_t* pName);
	CModel(const CModel&);
	~CModel();

	bool Init(ID3D11Device* device);
	void UploadBuffers(ID3D11DeviceContext* context);
	void SetTex(ID3D11ShaderResourceView* tex);

	XMMATRIX* GetTranslationMatrix();
	XMMATRIX* GetScaleMatrix();
	XMMATRIX* GetRoatationMatrix();
	XMMATRIX GetTransformMatrix();
	XMVECTOR* GetAngle();

	unsigned int GetIndexCount();

	LPCWSTR GetName();

private:

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11ShaderResourceView* m_pShaderResourceView;
    
	wchar_t* m_pName;

	VertexType* m_pVertices;
	unsigned long* m_pIndices;
	unsigned int m_vertexCount;
	unsigned int m_indexCount;

	XMMATRIX m_scaleMatrix;
	XMMATRIX m_rotationMatrix;
	XMMATRIX m_translationMatrix;

	XMVECTOR m_angleVector;
};
#endif