#ifndef _OBJECT_H
#define _OBJECT_H

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct MatrixBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};
class CMBuffer
{
public:
	CMBuffer(ID3D11Device*, ID3D11DeviceContext*, XMMATRIX*, XMMATRIX*);
	~CMBuffer();

	bool Init();
	void ShutDown();
	void Update();
	void SetWorldMatrix(XMMATRIX*);
	void SetViewMatrix(XMMATRIX*);
	ID3D11Buffer* getMB()
	{
		return m_pMatrixBuffer;
	};

private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_projMat;
	XMMATRIX m_viewMat;

	ID3D11Buffer* m_pMatrixBuffer;
};

#endif
