#include "Camera12.h"
#include "Frustum12.h"
#include "HeapManager.h"
#include "typedef.h"
namespace wilson {
	using namespace DirectX;
	Camera12::Camera12(const UINT screenWidth, const UINT screenHeight, float screenFar, float screenNear)
		:m_nearZ(screenNear), m_farZ(screenFar), m_fFOV((_Pi_val) / 4.0f), m_fScreenRatio(screenWidth / static_cast<float>(screenHeight)),
		m_translationSpeed(_DEFAULT_CAMERA_TRANSLATION_SPEED), m_rotationSpeed(_DEFAULT_CAMERA_ROTATION_SPEED)
	{
		m_camPosKey			= g_pHeapManager->AllocateCb(sizeof(XMVECTOR));
		m_cascadeLevelKey	= g_pHeapManager->AllocateCb(sizeof(XMVECTOR) * _CASCADE_LEVELS);
		m_vMatrixCbKey		= g_pHeapManager->AllocateCb(sizeof(XMMATRIX));
		m_pMatrixCbKey		= g_pHeapManager->AllocateCb(sizeof(XMMATRIX));

		m_pWVPMatrices				=	make_shared<WVPMatrices>();
		m_pWVPtransposedMatrices	=	make_shared<WVPMatrices>();
		m_pCompositeMatrices		=	make_shared<CompositeMatrices>();

		ResetTranslation();
		ResetRotation();
		UpdateMatrices();
	}

	bool					Camera12::GetDirtyBit()
	{
		return bDirty;
	}
	float					Camera12::GetAspect()
	{
		return m_fScreenRatio;
	}
	float*					Camera12::GetNearZ()
	{
		return &m_nearZ;
	}
	float*					Camera12::GetFarZ()
	{
		return &m_farZ;
	}
	float*					Camera12::GetFovY()
	{
		return &m_fFOV;
	}
	float*					Camera12::GetTranslationSpeed()
	{
		return &m_translationSpeed;
	}
	float*					Camera12::GetRotationSpeed()
	{
		return &m_rotationSpeed;
	}
	XMVECTOR*				Camera12::GetPosition()
	{
		return &m_pos;
	}
	XMVECTOR*				Camera12::GetRotation()
	{
		return &m_rotation;
	}
	XMMATRIX*				Camera12::GetProjectionMatrix()
	{
		return &m_pWVPMatrices->pMat;
	}
	XMMATRIX*				Camera12::GetViewMatrix()
	{
		return &m_pWVPMatrices->vMat;
	}
	XMMATRIX* Camera12::GetProjectionMatrixTransposed()
	{
		return &m_pWVPtransposedMatrices->pMat;
	}
	XMMATRIX* Camera12::GetViewMatrixTransposed()
	{
		return &m_pWVPtransposedMatrices->vMat;
	}
	XMMATRIX&				Camera12::GetViewProjectionMatrix()
	{
		return m_pCompositeMatrices->vpMat;
	}
	vector<float>&			Camera12::GetCascadeLevels()
	{
		return m_shadowCascadeFarZs;
	}
	shared_ptr<Frustum12>	Camera12::GetFrustum()
	{
		return m_pFrustum;
	};

	void Camera12::ResetRotation()
	{
		m_rotation = XMVectorZero();
	}
	void Camera12::ResetTranslation()
	{
		m_pos = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	}
	void Camera12::Rotate(const int dpitch, const int dyaw)
	{
		XMFLOAT4 float4;
		XMStoreFloat4(&float4, m_rotation);
		float pitch = float4.x + dpitch * m_rotationSpeed;
		float yaw = float4.y + dyaw * m_rotationSpeed;

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

		m_rotation = XMVectorSet(pitch, yaw, 0.0f, 0.0f);
		SetDirty(true);
	}

	void Camera12::SetDirty		(const bool isDirty)
	{
		bDirty = isDirty;
	}
	void Camera12::SetFarZ		(const float z)
	{
		m_farZ = z;
	}
	void Camera12::SetNearZ		(const float z)
	{
		m_nearZ = z;
	}
	void Camera12::SetPosition	(const XMVECTOR pos)
	{
		m_pos = pos;
	}
	void Camera12::SetRotation	(const XMVECTOR rot)
	{
		m_rotation = rot;
	}
	void Camera12::SetTranslationSpeed	(const float speed)
	{
		m_translationSpeed = speed;
	}
	void Camera12::SetRotationSpeed(const float speed)
	{
		m_rotationSpeed = speed;
	}

	void Camera12::Translate(const XMVECTOR dv)
	{
		XMMATRIX rtMat = XMMatrixRotationRollPitchYawFromVector(m_rotation);
		XMVECTOR displacemenst = XMVector3Transform(dv, rtMat);
		displacemenst =XMVectorScale(displacemenst, m_translationSpeed);
		m_pos = XMVectorAdd(m_pos, displacemenst);

		SetDirty(true);
	}
	void Camera12::SetLightingPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		UploadCamPos(pCmdList, false);
		
		g_pHeapManager->UploadGraphicsCb(m_cascadeLevelKey, E_TO_UINT(ePbrLightRP::psCasCadeLevels), pCmdList);
		g_pHeapManager->UploadGraphicsCb(m_vMatrixCbKey, E_TO_UINT(ePbrLightRP::psViewMat), pCmdList);
		g_pHeapManager->UploadGraphicsCb(m_pMatrixCbKey, E_TO_UINT(ePbrLightRP::psProjMat), pCmdList);
	}
	void Camera12::UpdateMatrices()
	{
		XMMATRIX rtMat = XMMatrixRotationRollPitchYawFromVector(m_rotation);
		XMVECTOR dir = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rtMat);
		dir = XMVector3Normalize(dir);
		XMVECTOR target = XMVectorAdd(dir, m_pos);

		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		XMVECTOR right = XMVector3Cross(dir, up);
		right = XMVector3Normalize(right);

		up = XMVector3Cross(right, dir);
		up = XMVector3Normalize(up);

		m_pWVPMatrices->vMat =					XMMatrixLookAtLH(m_pos, target, up);
		m_pWVPMatrices->pMat =					XMMatrixPerspectiveFovLH(m_fFOV, m_fScreenRatio, m_nearZ, m_farZ);
		m_pCompositeMatrices->vpMat =			XMMatrixMultiplyTranspose(m_pWVPMatrices->vMat, m_pWVPMatrices->pMat);
		m_pWVPtransposedMatrices->vMat =		XMMatrixTranspose(m_pWVPMatrices->vMat);
		m_pWVPtransposedMatrices->pMat =		XMMatrixTranspose(m_pWVPMatrices->pMat);
		UpdateCascadeFarZs();

		g_pHeapManager->CopyDataToCb(m_camPosKey, sizeof(XMVECTOR), &m_pos);
		g_pHeapManager->CopyDataToCb(m_vMatrixCbKey, sizeof(XMMATRIX), &m_pWVPtransposedMatrices->vMat);
		g_pHeapManager->CopyDataToCb(m_pMatrixCbKey, sizeof(XMMATRIX), &m_pWVPtransposedMatrices->pMat);

		m_pFrustum = make_shared<Frustum12>(this);
	}
	void Camera12::UpdateCascadeFarZs()
	{
		vector<XMVECTOR> farZs(_CASCADE_LEVELS);
		m_shadowCascadeFarZs = vector<float>(_CASCADE_LEVELS);
		for (int i = 0; i < _CASCADE_LEVELS; ++i)
		{
			m_shadowCascadeFarZs[i] = m_farZ / _CASACADE_BOUND_FACTORS[i];
			farZs[i] = XMVectorSet(0, 0, m_shadowCascadeFarZs[i], 1.0f);
		}
		g_pHeapManager->CopyDataToCb(m_cascadeLevelKey, sizeof(XMVECTOR) * _CASCADE_LEVELS, &farZs[0]);
		
	}
	bool Camera12::UploadCamPos(ComPtr<ID3D12GraphicsCommandList> pCmdList, const bool bGeoPass)
	{
		UINT rootParmIdx = bGeoPass ? E_TO_UINT(ePbrGeoRP::psCamPos)
			: E_TO_UINT(ePbrLightRP::psCamPos);
		g_pHeapManager->UploadGraphicsCb(m_camPosKey, rootParmIdx, pCmdList);
		return true;

	}
}