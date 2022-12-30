#ifndef CAM_CAMERA_H
#define CAM_CAMERA_H

#include<DirectXMath.h>

using namespace DirectX;

class CCamera
{
public:
	CCamera(int screenWidth =1080, int screenHeight = 720, float ScreenNear = 100.0f, float ScreenFar = 0.01f);

	void ZoomIn();
	void ZoomOut();
	void translateLeft();
	void translateRight();
	void translateUpward();
	void translateDownWard();
	void RotatePitch();
	void RotateYaw();
	void RotateRoll();

	XMVECTOR* GetPosition();
	XMVECTOR* GetRotation();

	XMMATRIX* GetViewMatrix();
	XMMATRIX* GetProjectionMatrix();
private:

	XMVECTOR m_vPos      = { 0.0f, 0.0f, -1.0f, 0.0f };  
	XMVECTOR m_vLookat   = { 0.0f, 0.0f, 0.0f, 0.0f };
    XMVECTOR m_vUp       = { 0.0f, 1.0f, 0.0f, 0.0f };//which axis is upward
	XMVECTOR m_vRotation = { 0.0f, 0.0f, 0.0f, 0.0f };

	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;// think of the frustum as the lens of our camera, for it controls our view

	float m_fFOV;
	float m_fScreenRatio;
	float m_fScreenNear;
	float m_fScreenFar;

	float m_fTranslateSpeed;
	float m_fRotateSpeed;
	float m_fZoomSpeed;
};

#endif 
