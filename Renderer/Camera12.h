#pragma once

#include<DirectXMath.h>
#include<d3d12.h>
#include<vector>

#include "typedef.h"
namespace wilson {

	class Frustum;
	class DescriptorHeapManager;
	class Camera12
	{
	public:

		void Update();
		bool SetCascadeLevels(ID3D12GraphicsCommandList* pCommandlist);
		bool SetCamPos(ID3D12GraphicsCommandList* pCommandlist, bool bGeoPass);
		void ResetTranslation();
		void UpdateCascadeLevels();

		inline void ResetRotation()
		{
			m_rotation = DirectX::XMVectorZero();
		}

		inline DirectX::XMVECTOR* GetPosition()
		{
			return &m_pos;
		}
		inline DirectX::XMVECTOR* GetTarget()
		{
			return &m_target;
		}
		inline DirectX::XMVECTOR GetRight()
		{
			return m_right;
		}
		inline DirectX::XMVECTOR GetUp() const
		{
			return m_up;
		}
		inline DirectX::XMVECTOR GetDir() const
		{
			return m_dir;
		}
		inline DirectX::XMVECTOR* GetRotation()
		{
			return &m_rotation;
		}
		inline DirectX::XMMATRIX* GetViewMatrix()
		{
			return &m_viewMat;
		}
		inline DirectX::XMMATRIX* GetProjectionMatrix()
		{
			return &m_projMat;
		}
		inline std::vector<float>& GetCascadeLevels()
		{
			return m_shadowCascadeLevels;
		}


		inline float* GetTRSpeed()
		{
			return &m_trSpeed;
		}
		inline float* GetNearZ()
		{
			return &m_fScreenNear;
		}
		inline float* GetFarZ()
		{
			return &m_fScreenFar;
		}
		inline float* GetFovY()
		{
			return &m_fFOV;
		}
		inline float GetAspect() const
		{
			return m_fScreenRatio;
		}
		void Rotate(int, int);
		void Translate(DirectX::XMVECTOR);

		Camera12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, DescriptorHeapManager* pDescriptorHeapManager,
			const UINT screenWidth, const UINT screenHeight, float ScreenFar, float ScreenNear);
		~Camera12();
	private:


		DirectX::XMVECTOR m_pos;
		DirectX::XMVECTOR m_dir;
		DirectX::XMVECTOR m_target;
		DirectX::XMVECTOR m_right;
		DirectX::XMVECTOR m_worldUp;
		DirectX::XMVECTOR m_up;      //which axis is upward
		DirectX::XMVECTOR m_rotation;

		DirectX::XMMATRIX m_viewMat;
		DirectX::XMMATRIX m_projMat;// think of the frustum as the lens of our camera, for it controls our view

		ID3D12Resource* m_pCamCb;
		D3D12_GPU_DESCRIPTOR_HANDLE m_camPosCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_cascadeLevelCBV;

		std::vector<float> m_shadowCascadeLevels;
		UINT8* m_pCamPosCbBegin;
		UINT8* m_pCascadeLevelCbBegin;
		float m_fFOV;
		float m_fScreenRatio;
		float m_fScreenNear;
		float m_fScreenFar;

		float m_trSpeed;
		float m_rtSpeed;

	};

}

