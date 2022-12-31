#ifndef WORLDTRANSFORMATION_H
#define WORLDTRANSFORMATION_H

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class CWMTransformation
{
public:
	CWMTransformation(ID3D11Device* device, ID3D11DeviceContext* context);
	~CWMTransformation() = default;

	void Scale(XMMATRIX& worldMatrix, float dx, float dy, float dz);
	void Rotate(XMMATRIX& worldMatrix, float dx, float dy, float dz);
	void Translate(XMMATRIX& worldMatrix, float dx, float dy, float dz);
private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
};

#endif
