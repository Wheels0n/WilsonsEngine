#include "Camera.h"

namespace wilson {
	Camera::Camera(const UINT screenWidth, const UINT screenHeight, float screenFar, float screenNear)
	{
		m_fScreenNear = screenNear;
		m_fScreenFar = screenFar;
		m_fFOV = static_cast<float>(3.1459) / 4.0f;
		m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);
		m_trSpeed = 0.1f;
		m_rtSpeed = 0.0175f;

		ResetTranslation();
		ResetRotation();
		m_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		m_worldUp = m_up;

		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_pCamPosBuffer = nullptr;
	}

	Camera::~Camera()
	{
		if (m_pCamPosBuffer != nullptr)
		{
			m_pCamPosBuffer->Release();
			m_pCamPosBuffer = nullptr;
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
			pitch += _RAD;
		}
		if (yaw < 0.0f)
		{
			yaw += _RAD;
		}
		pitch = pitch > _RAD ? 0 : pitch;
		yaw = yaw > _RAD ? 0 : yaw;

		m_rotation = DirectX::XMVectorSet(pitch, yaw, 0.0f, 0.0f);
	}

	void Camera::Translate(DirectX::XMVECTOR dv)
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		dv = DirectX::XMVector3Transform(dv, rtMat);
		dv = DirectX::XMVectorScale(dv, m_trSpeed);

		m_pos = DirectX::XMVectorAdd(m_pos, dv);
	}

	bool Camera::Init(ID3D11Device* pDevice)
	{
		D3D11_BUFFER_DESC camCBD = {};
		camCBD.Usage = D3D11_USAGE_DYNAMIC;
		camCBD.ByteWidth = sizeof(CamBuffer);
		camCBD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		camCBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		camCBD.MiscFlags = 0;
		camCBD.StructureByteStride = 0;
		HRESULT hr = pDevice->CreateBuffer(&camCBD, nullptr, &m_pCamPosBuffer);
		if (FAILED(hr))
		{
			return false;
		}
		m_pCamPosBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Camera::m_pCamPosBuffer") - 1, "Camera::m_pCamPosBuffer");
		return true;
	}

	void Camera::Update()
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		m_dir = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rtMat);
		m_dir = DirectX::XMVector3Normalize(m_dir);
		m_target = DirectX::XMVectorAdd(m_dir, m_pos);
		
		
		m_right = DirectX::XMVector3Cross(m_dir, m_worldUp);
		m_right = DirectX::XMVector3Normalize(m_right);

		m_up = DirectX::XMVector3Cross(m_right, m_dir);
		m_up = DirectX::XMVector3Normalize(m_up);

		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
		
	}

	bool Camera::SetCamPos(ID3D11DeviceContext* pContext)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		CamBuffer* pCamBuffer;

		hr = pContext->Map(m_pCamPosBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			return false;
		}

		pCamBuffer = reinterpret_cast<CamBuffer*>(mappedResource.pData);
		pCamBuffer->camPos = m_pos;
		pContext->Unmap(m_pCamPosBuffer, 0);
		pContext->PSSetConstantBuffers(0, 1, &m_pCamPosBuffer);

		return true;
	}

}