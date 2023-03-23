#ifndef _OBJECT_H
#define _OBJECT_H

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct MatrixBuffer
{
	XMMATRIX m_worldMat;
	XMMATRIX m_viewMat;
	XMMATRIX m_projMat;
};
class MatBuffer
{
public:
	bool Init();
	void ShutDown();
	void Update();
	
	inline void SetViewMatrix(XMMATRIX* viewMatrix)
	{
		m_viewMat = *viewMatrix;
	}
	inline void SetWorldMatrix(XMMATRIX* worldMatrix)
	{
		m_worldMat = *worldMatrix;
	}
	inline ID3D11Buffer* getMB()
	{
		return m_pMatBuffer;
	};

	MatBuffer(ID3D11Device*, ID3D11DeviceContext*, XMMATRIX*, XMMATRIX*);
	~MatBuffer()=delete;

private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	XMMATRIX m_worldMat;
	XMMATRIX m_projMat;
	XMMATRIX m_viewMat;

	ID3D11Buffer* m_pMatBuffer;
};

#endif
