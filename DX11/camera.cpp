#include "camera.h"

CCamera::CCamera(int screenWidth = 1080, int screenHeight = 720, float ScreenFar = 100.0f, float ScreenNear = 0.01f)
{    
	m_fScreenNear = ScreenNear;
	m_fScreenFar = ScreenFar;
	m_fFOV = static_cast<float>(3.1459) / 4.0f;
	m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);

	Reset();
	m_projectionMatrix = XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
	m_viewMatrix = XMMatrixLookAtLH(m_vPos, m_vTarget, m_vUp);
	
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


XMVECTOR* CCamera::GetPosition()
{
	return &m_vPos;
}

XMVECTOR* CCamera::GetTarget()
{
	return &m_vTarget;
}

XMVECTOR* CCamera::GetRotation()
{
	return &m_vRotation;
}

XMMATRIX* CCamera::GetViewMatrix()
{
	return &m_viewMatrix;
}

XMMATRIX* CCamera::GetProjectionMatrix()
{
	return &m_projectionMatrix;
}

void CCamera::Zoom(int)
{
}

void CCamera::Reset()
{
	m_vPos = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	m_vTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f );
	m_vUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f );//which axis is upward
	m_vRotation = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
}

void CCamera::Rotate(int dpitch, int dyaw)
{   
	XMFLOAT4 float4;
	XMStoreFloat4(&float4, m_vRotation);
	float pitch = float4.x + dpitch * m_rtSpeed;
	float yaw = float4.y + dyaw * m_rtSpeed;

	if (pitch < 0.0f)
	{
		pitch += RAD;
	}
	if (yaw < 0.0f)
	{
		yaw += RAD;
	}
	pitch = pitch > RAD ? 0 : pitch;
	yaw = yaw > RAD ?0 : yaw;

	m_vRotation = XMVectorSet(pitch, yaw, 0.0f, 0.0f);
}

void CCamera::Translate(XMVECTOR dv)
{   
	XMMATRIX rt = XMMatrixRotationRollPitchYawFromVector(m_vRotation);
	dv = XMVector3Transform(dv, rt);
	dv = XMVectorScale(dv, m_trSpeed);

	m_vPos = XMVectorAdd(m_vPos, dv);
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
	XMMATRIX rt = XMMatrixRotationRollPitchYawFromVector(m_vRotation);
	m_vTarget = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rt);
	m_vTarget = XMVectorAdd(m_vTarget, m_vPos);
	m_viewMatrix = XMMatrixLookAtLH(m_vPos, m_vTarget, m_vUp);
	//m_vUp = XMVector3Transform(m_vUp, rt); no roll
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
