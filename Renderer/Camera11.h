#pragma once
#include<d3d11.h>
#include "typedef.h"
namespace wilson {
	
	class Frustum11;

	class Camera11
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
		inline float* GetFarZ()
		{
			return &m_farZ;
		}
		inline float* GetFovY()
		{
			return &m_fFOV;
		}
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

		void Update();
		void UpdateCascadeLevels();
		bool UploadCamPos(ID3D11DeviceContext* const pContext);
		bool UploadCascadeLevels(ID3D11DeviceContext* const  pContext);
		inline void ResetRotation()
		{
			m_rotation = DirectX::XMVectorZero();
		}
		void ResetTranslation();
		void Rotate(const int, const int);
		void Translate(const DirectX::XMVECTOR);

		Camera11(ID3D11Device* const pDevice, const UINT screenWidth, const UINT screenHeight, const float ScreenFar, const float ScreenNear);
		~Camera11();
	private:
		

		DirectX::XMVECTOR m_dir;
		DirectX::XMVECTOR m_pos;
		DirectX::XMVECTOR m_right;
		DirectX::XMVECTOR m_rotation;
		DirectX::XMVECTOR m_target;
		DirectX::XMVECTOR m_up;      //which axis is upward
		DirectX::XMVECTOR m_worldUp;

		DirectX::XMMATRIX m_projMat;// think of the frustum as the lens of our camera, for it controls our view
		DirectX::XMMATRIX m_viewMat;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pCamPosCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pCascadeLevelCb;

		std::vector<float> m_shadowCascadeLevels;
		float m_farZ;
		float m_fFOV;
		float m_fScreenRatio;
		float m_nearZ;
		float m_rtSpeed;
		float m_trSpeed;

	};

}

