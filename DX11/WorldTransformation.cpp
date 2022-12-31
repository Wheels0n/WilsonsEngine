#include "WorldTransformation.h"

CWMTransformation::CWMTransformation(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_pDevice = device;
	m_pContext = context;
}

void CWMTransformation::Scale(XMMATRIX& worldMatrix, float dx, float dy, float dz)
{   
	XMMATRIX scaleMatrix = XMMatrixScaling(dx, dy, dz);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
}

void CWMTransformation::Rotate(XMMATRIX& worldMatrix, float dx, float dy, float dz)
{
	XMMATRIX rotateMatrix = XMMatrixRotationRollPitchYaw(dx, dy, dz);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotateMatrix);
}

void CWMTransformation::Translate(XMMATRIX& worldMatrix, float dx, float dy, float dz)
{
	XMMATRIX translateMatrix = XMMatrixTranslation(dx, dy, dz);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);
}
