#pragma once
#include<d3d12.h>
#include "typedef.h"

namespace wilson {
	using namespace std;
	using namespace DirectX;
	using namespace Microsoft::WRL;
	struct WVPMatrices;
	struct CompositeMatrices;

	class Frustum12;
	class Camera12 : public enable_shared_from_this<Camera12>
	{
	public:
		bool							GetDirtyBit();
		float							GetAspect();
		float*							GetNearZ();
		float*							GetFarZ();
		float*							GetFovY();
		float*							GetTranslationSpeed();
		float*							GetRotationSpeed();
		XMVECTOR*						GetPosition();
		XMVECTOR*						GetRotation();
		XMMATRIX*						GetProjectionMatrix();
		XMMATRIX*						GetViewMatrix();
		XMMATRIX*						GetProjectionMatrixTransposed();
		XMMATRIX*						GetViewMatrixTransposed();
		XMMATRIX&						GetViewProjectionMatrix();
		vector<float>&					GetCascadeLevels();
		shared_ptr<Frustum12>			GetFrustum();

		void							UpdateMatrices();
		void							UpdateCascadeFarZs();

		bool							UploadCamPos(ComPtr<ID3D12GraphicsCommandList> pCmdList, const bool bGeoPass);

		void							ResetRotation();
		void							ResetTranslation();
		void							Rotate(const int, const int);

		void							SetDirty	(const bool isDirty);
		void							SetFarZ	(const float z);
		void							SetNearZ	(const float z);
		void							SetPosition(const XMVECTOR pos);
		void							SetRotation(const XMVECTOR rot);
		void							SetTranslationSpeed	(const float speed);
		void							SetRotationSpeed(const float speed);
		void							Translate(const XMVECTOR);

		void							SetLightingPass(ComPtr<ID3D12GraphicsCommandList>);

										Camera12(const UINT screenWidth, const UINT screenHeight, float ScreenFar, float ScreenNear);
										~Camera12() = default;
	private:
		shared_ptr<Frustum12>			m_pFrustum;
		shared_ptr<WVPMatrices>			m_pWVPMatrices;
		shared_ptr<WVPMatrices>			m_pWVPtransposedMatrices;
		shared_ptr<CompositeMatrices>	m_pCompositeMatrices;

		XMVECTOR						m_pos;
		XMVECTOR						m_rotation;

		UINT							m_camPosKey;
		UINT							m_cascadeLevelKey;
		UINT							m_vMatrixCbKey;
		UINT							m_pMatrixCbKey;

		vector<float>					m_shadowCascadeFarZs;
		float							m_nearZ;
		float							m_farZ;
		float							m_fFOV;
		float							m_fScreenRatio;

		float							m_rotationSpeed;
		float							m_translationSpeed;

		bool							bDirty;
	};

}

