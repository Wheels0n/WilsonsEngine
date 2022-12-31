#ifndef LIGHT_H
#define LIGHT_H

#include <dxgi.h>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct Light
{
	XMVECTOR specular;
	XMVECTOR ambient;
	XMVECTOR diffuse;
	XMFLOAT3 direction;
	float specPow;
};

class CLight
{
public:
	CLight(ID3D11Device* device, ID3D11DeviceContext* context);
	~CLight();
	void Update();
	bool Init();
private:

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	ID3D11Buffer* m_pLightBuffer;

	XMVECTOR m_vSpecular = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR m_vAmbient = { 0.1f,  0.1f, 0.1f, 0.1f };
	XMVECTOR m_vDiffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 m_vDirection = {0.0f, 0.0f, 1.0f};
	float    m_fSpecPow = 32.0f;
};

#endif 
