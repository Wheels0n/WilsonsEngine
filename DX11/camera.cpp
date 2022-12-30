#include "camera.h"

CCamera::CCamera(int screenWidth = 1080, int screenHeight = 720, float ScreenNear = 100.0f, float ScreenFar = 0.01f)
{    
	m_viewMatrix = XMMatrixLookAtLH(m_vPos, m_vLookat, m_vUp);

	m_fScreenNear = ScreenNear;
	m_fScreenFar = ScreenFar;
	m_fFOV = static_cast<float>(3.1459) / 4.0f;
	m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);
	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
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
	XMMATRIX translationMatrix = XMMatrixTranslation(-1*m_fTranslateSpeed,0,0);
	m_vPos = XMVector4Transform(m_vPos, translationMatrix);
}

void CCamera::translateRight()
{
	XMMATRIX translationMatrix = XMMatrixTranslation(m_fTranslateSpeed, 0, 0);
	m_vPos = XMVector4Transform(m_vPos, translationMatrix);
}

void CCamera::translateUpward()
{
	XMMATRIX translationMatrix = XMMatrixTranslation(0, -1 * m_fTranslateSpeed, 0);
	m_vPos = XMVector4Transform(m_vPos, translationMatrix);
}

void CCamera::translateDownWard()
{
	XMMATRIX translationMatrix = XMMatrixTranslation(0, m_fTranslateSpeed, 0);
	m_vPos = XMVector4Transform(m_vPos, translationMatrix);
}

void CCamera::RotatePitch()
{
	XMMATRIX rotationMatrix = XMMatrixRotationX(m_fRotateSpeed);
	m_vLookat = XMVector4Transform(m_vLookat, rotationMatrix);
	m_vUp = XMVector4Transform(m_vUp, rotationMatrix);

	m_vLookat = XMVectorAdd(m_vLookat, m_vPos);
}

void CCamera::RotateYaw()
{
	XMMATRIX rotationMatrix = XMMatrixRotationY(m_fRotateSpeed);
	m_vLookat = XMVector4Transform(m_vLookat, rotationMatrix);
	m_vUp = XMVector4Transform(m_vUp, rotationMatrix);

	m_vLookat = XMVectorAdd(m_vLookat, m_vPos);
}

void CCamera::RotateRoll()
{
	XMMATRIX rotationMatrix = XMMatrixRotationZ(m_fRotateSpeed);
	m_vLookat = XMVector4Transform(m_vLookat, rotationMatrix);
	m_vUp = XMVector4Transform(m_vUp, rotationMatrix);

	m_vLookat = XMVectorAdd(m_vLookat, m_vPos);
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
