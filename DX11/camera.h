#ifndef CAM_CAMERA_H
#define CAM_CAMERA_H
#define RAD 6.28319

#include<DirectXMath.h>
#include<d3d11.h>
using namespace DirectX;

struct camBuffer
{
	XMVECTOR camPos;
};

class CCamera
{
public:
	CCamera(int screenWidth, int screenHeight, float ScreenFar, float ScreenNear);
	~CCamera();

	void Init(ID3D11Device* device);
	void Update();
	void SetCamBuffer(ID3D11DeviceContext* context);
	void ResetTranslation();
	void ResetRotation();

	XMVECTOR* GetPosition();
	XMVECTOR* GetTarget();
	XMVECTOR* GetRotation();

	void Rotate(int, int);
	void Translate(XMVECTOR);
	void Zoom(int);

	XMMATRIX* GetViewMatrix();
	XMMATRIX* GetProjectionMatrix();
private:

	XMVECTOR m_vPos;
	XMVECTOR m_vTarget;
	XMVECTOR m_vUp;      //which axis is upward
	XMVECTOR m_vRotation;

	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;// think of the frustum as the lens of our camera, for it controls our view

	ID3D11Buffer* m_pCamBuffer;

	float m_fFOV;
	float m_fScreenRatio;
	float m_fScreenNear;
	float m_fScreenFar;

	float m_trSpeed = 0.1f;
	float m_rtSpeed = 0.0175f;

};

#endif 
