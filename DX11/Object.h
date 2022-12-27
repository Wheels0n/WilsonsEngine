#ifndef _OBJECT_H
#define _OBJECT_H
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx10math.h>
#include<random>

struct VertexType
{
	D3DXVECTOR3 position;
	D3DXVECTOR2 tex;
	D3DXVECTOR3 norm;
};

struct Light
{   
	D3DXVECTOR4 specular;
	D3DXVECTOR4 ambient;
	D3DXVECTOR4 diffuse;
	D3DXVECTOR3 direction;
	float specPow;
};

struct camBuffer
{
	D3DXVECTOR4 camPos;
};

struct ConstantBufferType
{
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX projection;
};
class CObject
{
public:
	CObject(ID3D11Device* , ID3D11DeviceContext*, D3DXMATRIX*, D3DXMATRIX*);
	CObject(const CObject&);
	~CObject();

	bool Init(D3DXVECTOR3 translation, D3DXVECTOR3 scale);
	void ShutDown();
	void UpdateWorld(float wx, float wy, float wz, float vyaw);
	ID3D11Buffer* getMB()
	{
		return m_pMatrixBuffer;
	};

	ID3D11Buffer* getCB()
	{
		return m_pCamBuffer;
	};	

private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	D3DXMATRIX m_worldMatrix;
	D3DXMATRIX m_projectionMatrix;
	D3DXMATRIX m_viewMatrix;

	D3DXMATRIX m_translationMatrix;
	D3DXMATRIX m_scaleMatrix;
	D3DXMATRIX m_rotationMatrix;
	ID3D11Buffer* m_pMatrixBuffer;
	ID3D11Buffer* m_pCamBuffer;
};

#endif
