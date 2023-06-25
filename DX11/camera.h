#ifndef CAM_CAMERA_H
#define CAM_CAMERA_H
#define RAD 6.28319

#include<DirectXMath.h>
#include<d3d11.h>

namespace wilson {

	struct CamBuffer
	{
		DirectX::XMVECTOR m_camPos;
	};

	class Camera
	{
	public:

		void Init(ID3D11Device* device);
		void Update();
		void SetCamBuffer(ID3D11DeviceContext* context);
		void ResetTranslation();
		inline void ResetRotation()
		{
			m_rotation = DirectX::XMVectorZero();
		}

		inline void SetENTTsInFrustum(int cnt)
		{
			m_ENTTsInFrustum = cnt;
		}
		inline int GetENTTsInFrustum()
		{
			return m_ENTTsInFrustum;
		}

		inline DirectX::XMVECTOR* GetPosition()
		{
			return &m_pos;
		}
		inline DirectX::XMVECTOR* GetTarget()
		{
			return &m_target;
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

		void Rotate(int, int);
		void Translate(DirectX::XMVECTOR);

		Camera(int screenWidth, int screenHeight, float ScreenFar, float ScreenNear);
		~Camera();
	private:

		DirectX::XMVECTOR m_pos;
		DirectX::XMVECTOR m_target;
		DirectX::XMVECTOR m_up;      //which axis is upward
		DirectX::XMVECTOR m_rotation;

		DirectX::XMMATRIX m_viewMat;
		DirectX::XMMATRIX m_projMat;// think of the frustum as the lens of our camera, for it controls our view

		ID3D11Buffer* m_pCamBuffer;

		int m_ENTTsInFrustum;

		float m_fFOV;
		float m_fScreenRatio;
		float m_fScreenNear;
		float m_fScreenFar;

		float m_trSpeed = 0.1f;
		float m_rtSpeed = 0.0175f;

	};

}
#endif 
