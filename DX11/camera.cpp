#include "Camera.h"

namespace wilson {
	Camera::Camera(int screenWidth, int screenHeight, float screenFar, float screenNear)
	{
		m_fScreenNear = screenNear;
		m_fScreenFar = screenFar;
		m_fFOV = static_cast<float>(3.1459) / 4.0f;
		m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);

		ResetTranslation();
		ResetRotation();
		m_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_pCamBuffer = nullptr;
		m_ENTTsInFrustum = 0;

	}

	Camera::~Camera()
	{
		if (m_pCamBuffer != nullptr)
		{
			m_pCamBuffer->Release();
			m_pCamBuffer = nullptr;
		}
	}

	void Camera::ResetTranslation()
	{
		m_pos = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
		m_target = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}


	void Camera::Rotate(int dpitch, int dyaw)
	{
		DirectX::XMFLOAT4 float4;
		DirectX::XMStoreFloat4(&float4, m_rotation);
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
		yaw = yaw > RAD ? 0 : yaw;

		m_rotation = DirectX::XMVectorSet(pitch, yaw, 0.0f, 0.0f);
	}

	void Camera::Translate(DirectX::XMVECTOR dv)
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		dv = DirectX::XMVector3Transform(dv, rtMat);
		dv = DirectX::XMVectorScale(dv, m_trSpeed);

		m_pos = DirectX::XMVectorAdd(m_pos, dv);
	}

	void Camera::Init(ID3D11Device* pDevice)
	{
		D3D11_BUFFER_DESC camCBD;
		camCBD.Usage = D3D11_USAGE_DYNAMIC;
		camCBD.ByteWidth = sizeof(CamBuffer);
		camCBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		camCBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		camCBD.MiscFlags = 0;
		camCBD.StructureByteStride = 0;
		pDevice->CreateBuffer(&camCBD, nullptr, &m_pCamBuffer);
	}

	void Camera::Update()
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		m_target = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rtMat);
		m_target = DirectX::XMVectorAdd(m_target, m_pos);
		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		//m_vUp = XMVector3Transform(m_vUp, rt); no roll

		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
	}

	void Camera::SetCamBuffer(ID3D11DeviceContext* pContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		CamBuffer* pCamBuffer;

		pContext->Map(m_pCamBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		pCamBuffer = reinterpret_cast<CamBuffer*>(mappedResource.pData);
		pCamBuffer->m_camPos = m_pos;
		pContext->Unmap(m_pCamBuffer, 0);
		pContext->PSSetConstantBuffers(2, 1, &m_pCamBuffer);

		return;
	}
}