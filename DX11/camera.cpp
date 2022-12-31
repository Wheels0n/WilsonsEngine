#include "camera.h"

CCamera::CCamera(int screenWidth = 1080, int screenHeight = 720, float ScreenFar = 100.0f, float ScreenNear = 0.01f)
{    
	m_fScreenNear = ScreenNear;
	m_fScreenFar = ScreenFar;
	m_fFOV = static_cast<float>(3.1459) / 4.0f;
	m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);

	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
	m_viewMatrix = XMMatrixLookAtLH(m_vPos, m_vLookat, m_vUp);
	
	m_pCamBuffer = nullptr;
}

CCamera::~CCamera()
{
	if (m_pCamBuffer != nullptr)
	{
		m_pCamBuffer->Release();
		m_pCamBuffer = nullptr;
	}
}

void CCamera::ZoomIn()
{   
	XMVECTOR multiplier = XMVectorReplicate(m_fZoomSpeed);
	m_vPos = XMVectorMultiplyAdd(multiplier, m_vLookat, m_vPos);
}

void CCamera::ZoomOut()
{
	XMVECTOR multiplier = XMVectorReplicate(-1 * m_fZoomSpeed);
	m_vPos = XMVectorMultiplyAdd(multiplier, m_vLookat, m_vPos);
}

void CCamera::translateLeft()
{
	XMVECTOR vAdd = { -1* m_fTranslateSpeed, 0.0f, 0.0f, 0.0f };
	m_vPos = XMVectorAdd(m_vPos, vAdd);
}

void CCamera::translateRight()
{   
	XMVECTOR vAdd = { m_fTranslateSpeed, 0.0f, 0.0f, 0.0f };
	m_vPos = XMVectorAdd(m_vPos, vAdd);
}

void CCamera::translateUpward()
{
	XMVECTOR vAdd = { 0.0f, -1* m_fTranslateSpeed, 0.0f, 0.0f };
	m_vPos = XMVectorAdd(m_vPos, vAdd);
}

void CCamera::translateDownWard()
{
	XMVECTOR vAdd = { 0.0f, m_fTranslateSpeed, 0.0f, 0.0f };
	m_vPos = XMVectorAdd(m_vPos, vAdd);
}

void CCamera::RotatePitch()
{
	XMMATRIX rotationMatrix = XMMatrixRotationX(m_fRotateSpeed);
	m_vLookat = XMVector4Transform(m_vLookat, rotationMatrix);
	m_vUp = XMVector4Transform(m_vUp, rotationMatrix);

	m_vLookat = XMVectorAdd(m_vLookat, m_vPos);
}

void CCamera::RotateLeft()
{
	XMMATRIX rotationMatrix = XMMatrixRotationY(m_fRotateSpeed);
	m_vLookat = XMVector4Transform(m_vLookat, rotationMatrix);
	m_vUp = XMVector4Transform(m_vUp, rotationMatrix);
}

void CCamera::RotateRight()
{
	XMMATRIX rotationMatrix = XMMatrixRotationY(-1 * m_fRotateSpeed);
	m_vLookat = XMVector4Transform(m_vLookat, rotationMatrix);
	m_vUp = XMVector4Transform(m_vUp, rotationMatrix);

}

void CCamera::RotateRoll()
{
	XMMATRIX rotationMatrix = XMMatrixRotationZ(m_fRotateSpeed);
	m_vLookat = XMVector4Transform(m_vLookat, rotationMatrix);
	m_vUp = XMVector4Transform(m_vUp, rotationMatrix);

}

XMVECTOR* CCamera::GetPosition()
{
	return &m_vPos;
}

XMVECTOR* CCamera::GetRotation()
{
	return &m_vRotation;
}

XMMATRIX* CCamera::GetViewMatrix()
{
	return &m_viewMatrix;
};
XMMATRIX* CCamera::GetProjectionMatrix()
{
	return &m_projectionMatrix;
}

void CCamera::Init(ID3D11Device* device)
{
	D3D11_BUFFER_DESC camCbd;
	camCbd.Usage = D3D11_USAGE_DYNAMIC;
	camCbd.ByteWidth = sizeof(camBuffer);
	camCbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camCbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camCbd.MiscFlags = 0;
	camCbd.StructureByteStride = 0;
	device->CreateBuffer(&camCbd, nullptr, &m_pCamBuffer);
}

void CCamera::Update()
{   
	//m_vLookat = XMVector4Normalize(m_vLookat);
	//m_vUp = XMVector4Normalize(m_vUp);
	m_viewMatrix = XMMatrixLookAtLH(m_vPos, m_vLookat, m_vUp);
}

void CCamera::SetCamBuffer(ID3D11DeviceContext* context)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	camBuffer* pCamBuffer;

	context->Map(m_pCamBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	pCamBuffer = reinterpret_cast<camBuffer*>(mappedResource.pData);
	pCamBuffer->camPos = m_vPos;
	context->Unmap(m_pCamBuffer, 0);
	context->VSSetConstantBuffers(1, 1, &m_pCamBuffer);

	return;
}
