#include "Camera12.h"
#include "HeapManager.h"
#include "typedef.h"
namespace wilson {

	Camera12::Camera12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, HeapManager* pHeapManager,
		const UINT screenWidth, const UINT screenHeight, float screenFar, float screenNear)
	{
		m_nearZ = screenNear;
		m_farZ = screenFar;
		//radian 
		m_fFOV = static_cast<float>(3.1459) / 4.0f;
		m_fScreenRatio = screenWidth / static_cast<float>(screenHeight);
		m_trSpeed = 0.1f;
		m_rtSpeed = 0.0175f;
		UpdateCascadeLevels();


		ResetTranslation();
		ResetRotation();
		m_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		m_worldUp = m_up;

		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_nearZ, m_farZ);
		m_viewMat = DirectX::XMMatrixLookAtLH(m_pos, m_target, m_up);
		m_vpMat = DirectX::XMMatrixMultiplyTranspose(m_viewMat, m_projMat);
		m_viewMat = DirectX::XMMatrixTranspose(m_viewMat);
		m_projMat = DirectX::XMMatrixTranspose(m_projMat);

		m_pCamPosCbBegin = nullptr;
		m_pCascadeLevelCbBegin = nullptr; 
		
		UINT cbSize = sizeof(DirectX::XMVECTOR);
		m_pCamPosCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_camPosCBV = pHeapManager->GetCBV(cbSize, pDevice);
		
		
		cbSize = sizeof(DirectX::XMVECTOR) * _CASCADE_LEVELS;
		m_pCascadeLevelCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_cascadeLevelCBV = pHeapManager->GetCBV(cbSize, pDevice);
		
	
	}

	void Camera12::ResetTranslation()
	{
		m_pos = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
		m_target = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	}

	void Camera12::UpdateCascadeLevels()
	{
		m_shadowCascadeLevels = std::vector({ m_farZ / 100.0f,  m_farZ / 50.0f,  m_farZ / 25.0f,  m_farZ / 5.0f , m_farZ });
	}


	void Camera12::Rotate(int dpitch, int dyaw)
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

	void Camera12::Translate(DirectX::XMVECTOR dv)
	{
		DirectX::XMMATRIX rtMat = DirectX::XMMatrixRotationRollPitchYawFromVector(m_rotation);
		dv = DirectX::XMVector3Transform(dv, rtMat);
		dv = DirectX::XMVectorScale(dv, m_trSpeed);

		m_pos = DirectX::XMVectorAdd(m_pos, dv);
	}

	void Camera12::Update()
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
		m_projMat = DirectX::XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_nearZ, m_farZ);
		m_vpMat =   DirectX::XMMatrixMultiplyTranspose(m_viewMat, m_projMat);
		m_viewMat = DirectX::XMMatrixTranspose(m_viewMat);
		m_projMat = DirectX::XMMatrixTranspose(m_projMat);
		UpdateCascadeLevels();
	}

	bool Camera12::UploadCascadeLevels(ID3D12GraphicsCommandList* pCommandlist)
	{
		std::vector<DirectX::XMVECTOR> FarZs(_CASCADE_LEVELS);
		for (int i = 0; i < _CASCADE_LEVELS; ++i)
		{
			FarZs[i] = DirectX::XMVectorSet(0, 0, m_shadowCascadeLevels[i], 1.0f);
		}

		memcpy(m_pCascadeLevelCbBegin, &FarZs[0], sizeof(DirectX::XMVECTOR) * _CASCADE_LEVELS);
		pCommandlist->SetGraphicsRootDescriptorTable(ePbrLightRP::ePbrLight_ePsCasCadeLevels, m_cascadeLevelCBV);
		return true;
	}


	bool Camera12::UploadCamPos(ID3D12GraphicsCommandList* pCommandlist, bool bGeoPass)
	{
		
		memcpy(m_pCamPosCbBegin, &m_pos, sizeof(DirectX::XMVECTOR));
		pCommandlist->SetGraphicsRootDescriptorTable(bGeoPass?ePbrGeoRP::ePbrGeo_ePsCamPos: ePbrLightRP::ePbrLight_ePsCamPos,
			m_camPosCBV);
		return true;

	}

}