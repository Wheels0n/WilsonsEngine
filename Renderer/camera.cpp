
#include "Camera.h"

namespace wilson {
	Camera::Camera(ID3D11Device* pDevice, const UINT screenWidth, const UINT screenHeight, float screenFar, float screenNear)
	{
		m_fScreenNear = screenNear;
		m_fScreenFar = screenFar;
		m_fFOV = static_cast<float>(3.1459) / 4.0f;
		m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);
		m_trSpeed = 0.1f;
		m_rtSpeed = 0.0175f;
		UpdateCascadeLevels();


		ResetTranslation();
		ResetRotation();
		m_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		m_worldUp = m_up;

		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_viewMat = DirectX::XMMatrixTranspose(m_viewMat);
		m_projMat = DirectX::XMMatrixTranspose(m_projMat);


		m_pCamPosBuffer = nullptr;
		m_pCascadeLevelBuffer = nullptr;


		{
			D3D11_BUFFER_DESC cbufferDesc = {};
			cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbufferDesc.ByteWidth = sizeof(CamBuffer);
			cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbufferDesc.MiscFlags = 0;
			cbufferDesc.StructureByteStride = 0;
			HRESULT hr = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pCamPosBuffer);
			if (FAILED(hr))
			{
				OutputDebugStringA("Camera::m_pCamPosBuffer::CreateBufferFailed");
			}
			m_pCamPosBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Camera::m_pCamPosBuffer") - 1, "Camera::m_pCamPosBuffer");

			cbufferDesc.ByteWidth = sizeof(DirectX::XMVECTOR) * 5;
			hr = pDevice->CreateBuffer(&cbufferDesc, nullptr, &m_pCascadeLevelBuffer);
			if (FAILED(hr))
			{
				OutputDebugStringA("Camera::m_pCascadeLevelBuffer::CreateBufferFailed");
			}
			m_pCascadeLevelBuffer->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Camera::m_pCascadeLevelBuffer") - 1, "Camera::m_pCascadeLevelBuffer");
		}
	}

	Camera::~Camera()
	{
		if (m_pCamPosBuffer != nullptr)
		{
			m_pCamPosBuffer->Release();
			m_pCamPosBuffer = nullptr;
		}
		if (m_pCascadeLevelBuffer != nullptr)
		{
			m_pCascadeLevelBuffer->Release();
			m_pCascadeLevelBuffer = nullptr;
		}
		
	}

	void Camera::ResetTranslation()
	{
		m_pos = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
		m_target = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	void Camera::UpdateCascadeLevels()
	{
		m_shadowCascadeLevels = std::vector({ m_fScreenFar / 100.0f,  m_fScreenFar / 50.0f,  m_fScreenFar / 25.0f,  m_fScreenFar / 5.0f , m_fScreenFar});
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

	void Camera::Update()
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		m_dir = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rtMat);
		m_dir = DirectX::XMVector3Normalize(m_dir);
		m_target = DirectX::XMVectorAdd(m_dir, m_pos);
		
		
		m_right = DirectX::XMVector3Cross(m_dir, m_worldUp);
		m_right = DirectX::XMVector3Normalize(m_right);

		m_up = DirectX::XMVector3Cross(m_right, m_dir);
		m_up = DirectX::XMVector3Normalize(m_up);

		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_viewMat = DirectX::XMMatrixTranspose(m_viewMat);
		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_fScreenNear, m_fScreenFar);
		m_projMat = DirectX::XMMatrixTranspose(m_projMat);
		UpdateCascadeLevels();
	}

	bool Camera::SetCascadeLevels(ID3D11DeviceContext* pContext)
	{	
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		DirectX::XMVECTOR* pFarZ;

		hr = pContext->Map(m_pCascadeLevelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{	
			OutputDebugStringA("m_pCascadeLevelBuffer::MapFailed");
			return false;
		}
		
		pFarZ = reinterpret_cast<DirectX::XMVECTOR*>(mappedResource.pData);
		for (int i = 0; i < m_shadowCascadeLevels.size(); ++i)
		{
			pFarZ[i]= DirectX::XMVectorSet(0,0,m_shadowCascadeLevels[i],1.0f);
		}
		
		pContext->Unmap(m_pCascadeLevelBuffer, 0);
		pContext->PSSetConstantBuffers(1, 1, &m_pCascadeLevelBuffer);

		return true;
	}

	bool Camera::SetCamPos(ID3D11DeviceContext* pContext)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		CamBuffer* pCamBuffer;

		hr = pContext->Map(m_pCamPosBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{	
			OutputDebugStringA("m_pCamPosBuffer::MapFailed");
			return false;
		}

		pCamBuffer = reinterpret_cast<CamBuffer*>(mappedResource.pData);
		pCamBuffer->camPos = m_pos;
		pContext->Unmap(m_pCamPosBuffer, 0);
		pContext->PSSetConstantBuffers(0, 1, &m_pCamPosBuffer);

		return true;
	}

}