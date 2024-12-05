#pragma once
#include<d3d12.h>
#include "typedef.h"
namespace wilson {

	class Frustum12;
	class HeapManager;
	class Camera12
	{
	public:
		inline float GetAspect() const
		{
			return m_fScreenRatio;
		}
		inline std::vector<float>& GetCascadeLevels()
		{
			return m_shadowCascadeLevels;
		}
		inline DirectX::XMVECTOR GetDir() const
		{
			return m_dir;
		}
		inline bool GetDirtyBit() const
		{
			return bDirty;
		}
		inline float* GetFarZ()
		{
			return &m_farZ;
		}
		inline float* GetFovY()
		{
			return &m_fFOV;
		}
		inline Frustum12* GetFrustum() const
		{
			return m_pFrustum;
		};
		inline float* GetNearZ()
		{
			return &m_nearZ;
		}
		inline DirectX::XMVECTOR* GetPosition()
		{
			return &m_pos;
		}
		inline DirectX::XMMATRIX* GetProjectionMatrix()
		{
			return &m_projMat;
		}
		inline DirectX::XMVECTOR GetRight()
		{
			return m_right;
		}
		inline DirectX::XMVECTOR* GetRotation()
		{
			return &m_rotation;
		}
		inline DirectX::XMVECTOR* GetTarget()
		{
			return &m_target;
		}
		inline float* GetTrSpeed()
		{
			return &m_trSpeed;
		}
		inline DirectX::XMVECTOR GetUp() const
		{
			return m_up;
		}
		inline DirectX::XMMATRIX* GetViewMatrix()
		{
			return &m_viewMat;
		}
		inline DirectX::XMMATRIX& GetViewProjectionMatrix()
		{
			return m_vpMat;
		}

		void Update();
		void UpdateCascadeLevels();
		bool UploadCamPos(ID3D12GraphicsCommandList* const pCommandlist, const bool bGeoPass);
		bool UploadCascadeLevels(ID3D12GraphicsCommandList* const pCommandlist);
		inline void ResetRotation()
		{
			m_rotation = DirectX::XMVectorZero();
		}
		void ResetTranslation();
		void Rotate(const int, const int);
		inline void SetDirty(const bool isDirty)
		{
			bDirty = isDirty;
		}
		inline void SetFarZ(const float z)
		{
			m_farZ = z;
		}
		inline void SetNearZ(const float z)
		{
			m_nearZ = z;
		}
		inline void SetPosition(const DirectX::XMVECTOR pos)
		{
			m_pos = pos;
		}
		inline void SetRotation(const DirectX::XMVECTOR rot)
		{
			m_rotation = rot;
		}
		inline void SetTrSpeed(const float speed)
		{
			m_trSpeed = speed;
		}
		void Translate(const DirectX::XMVECTOR);

		Camera12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandlist, HeapManager* const pHeapManager,
			const UINT screenWidth, const UINT screenHeight, float ScreenFar, float ScreenNear);
		~Camera12();
	private:
		Frustum12* m_pFrustum;

		DirectX::XMVECTOR m_dir;
		DirectX::XMVECTOR m_pos;
		DirectX::XMVECTOR m_right;
		DirectX::XMVECTOR m_rotation;
		DirectX::XMVECTOR m_target;
		DirectX::XMVECTOR m_up;      //which axis is upward
		DirectX::XMVECTOR m_worldUp;

		DirectX::XMMATRIX m_projMat;// think of the frustum as the lens of our camera, for it controls our view
		DirectX::XMMATRIX m_viewMat;
		DirectX::XMMATRIX m_vpMat;

		D3D12_GPU_DESCRIPTOR_HANDLE m_camPosCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_cascadeLevelCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_frustumInfoCbv;

		UINT8* m_pCamPosCbBegin;
		UINT8* m_pCascadeLevelCbBegin;

		std::vector<float> m_shadowCascadeLevels;
		float m_farZ;
		float m_fFOV;
		float m_fScreenRatio;
		float m_nearZ;
		float m_rtSpeed;
		float m_trSpeed;

		bool bDirty;
	};

}

