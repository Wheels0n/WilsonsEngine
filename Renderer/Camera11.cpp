
#include "Camera11.h"

namespace wilson {
	Camera11::Camera11(ID3D11Device* const pDevice, const UINT screenWidth, const UINT screenHeight, float screenFar, float screenNear)
	{
		m_nearZ = screenNear;
		m_farZ = screenFar;
		m_fFOV = static_cast<float>(3.1459) / 4.0f;
		m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);
		m_trSpeed = 0.1f;
		m_rtSpeed = 0.0175f;
		UpdateCascadeLevels();


		ResetTranslation();
		ResetRotation();
		m_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		m_worldUp = m_up;

		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_nearZ, m_farZ);
		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_viewMat = DirectX::XMMatrixTranspose(m_viewMat);
		m_projMat = DirectX::XMMatrixTranspose(m_projMat);

		{
			D3D11_BUFFER_DESC cbufferDesc = {};
			cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbufferDesc.ByteWidth = sizeof(DirectX::XMVECTOR);
			cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbufferDesc.MiscFlags = 0;
			cbufferDesc.StructureByteStride = 0;
			HRESULT hr = pDevice->CreateBuffer(&cbufferDesc, nullptr, m_pCamPosCb.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCamPosCb.Get()->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Camera11::m_pCamPosCb") - 1, "Camera11::m_pCamPosCb");

			cbufferDesc.ByteWidth = sizeof(DirectX::XMVECTOR) * _CASCADE_LEVELS;
			hr = pDevice->CreateBuffer(&cbufferDesc, nullptr, m_pCascadeLevelCb.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCascadeLevelCb.Get()->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Camera11::m_pCascadeLevelCb") - 1, "Camera11::m_pCascadeLevelCb");
		}
	}

	Camera11::~Camera11()
	{
		
	}

	void Camera11::ResetTranslation()
	{
		m_pos = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
		m_target = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	void Camera11::UpdateCascadeLevels()
	{
		m_shadowCascadeLevels = std::vector({ m_farZ / 100.0f,  m_farZ / 50.0f,  m_farZ / 25.0f,  m_farZ / 5.0f , m_farZ});
	}


	void Camera11::Rotate(const int dpitch, const int dyaw)
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

	void Camera11::Translate(const DirectX::XMVECTOR dr)
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		DirectX::XMVECTOR displacement = DirectX::XMVector3Transform(dr, rtMat);
		displacement = DirectX::XMVectorScale(displacement, m_trSpeed);

		m_pos = DirectX::XMVectorAdd(m_pos, displacement);
	}

	void Camera11::Update()
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
		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_nearZ, m_farZ);
		m_projMat = DirectX::XMMatrixTranspose(m_projMat);
		UpdateCascadeLevels();
	}

	bool Camera11::UploadCascadeLevels(ID3D11DeviceContext* const  pContext)
	{	
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		DirectX::XMVECTOR* pFarZ;

		hr = pContext->Map(m_pCascadeLevelCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		
		pFarZ = reinterpret_cast<DirectX::XMVECTOR*>(mappedResource.pData);
		for (int i = 0; i < m_shadowCascadeLevels.size(); ++i)
		{
			pFarZ[i]= DirectX::XMVectorSet(0,0,m_shadowCascadeLevels[i],1.0f);
		}
		
		pContext->Unmap(m_pCascadeLevelCb.Get(), 0);
		pContext->PSSetConstantBuffers(1, 1, &m_pCascadeLevelCb);

		return true;
	}

	bool Camera11::UploadCamPos(ID3D11DeviceContext* const pContext)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		DirectX::XMVECTOR* pCamPos;

		hr = pContext->Map(m_pCamPosCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));

		pCamPos = reinterpret_cast<DirectX::XMVECTOR*>(mappedResource.pData);
		*pCamPos= m_pos;
		pContext->Unmap(m_pCamPosCb.Get(), 0);
		pContext->PSSetConstantBuffers(0, 1, &m_pCamPosCb);

		return true;
	}

}