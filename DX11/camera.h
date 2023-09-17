#pragma once

#include<DirectXMath.h>
#include<d3d11.h>


namespace wilson {

	class Frustum;

	constexpr float _RAD = 6.28319;

	struct CamBuffer
	{
		DirectX::XMVECTOR camPos;
	};

	class Camera
	{
	public:

		bool Init(ID3D11Device* pDevice);
		void Update();
		bool SetCamPos(ID3D11DeviceContext* pContext);
		void ResetTranslation();
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

		Camera(const UINT screenWidth, const UINT screenHeight, float ScreenFar, float ScreenNear);
		~Camera();
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

		ID3D11Buffer* m_pCamPosBuffer;

		float m_fFOV;
		float m_fScreenRatio;
		float m_fScreenNear;
		float m_fScreenFar;

		float m_trSpeed;
		float m_rtSpeed;

	};

}

