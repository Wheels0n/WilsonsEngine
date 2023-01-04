#ifndef CAM_CAMERA_H
#define CAM_CAMERA_H

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

	void ZoomIn();
	void ZoomOut();
	void translateLeft();
	void translateRight();
	void translateUpward();
	void translateDownWard();
	void RotatePitch();
	void RotateLeft();
	void RotateRight();
	void RotateRoll();

	void Init(ID3D11Device* device);
	void Update();
	void SetCamBuffer(ID3D11DeviceContext* context);
	XMVECTOR* GetPosition();
	XMVECTOR* GetRotation();

	XMMATRIX* GetViewMatrix();
	XMMATRIX* GetProjectionMatrix();
private:

	XMVECTOR m_vPos      = { 0.0f, 0.0f, -1.0f, 0.0f };  
	XMVECTOR m_vLookat   = { 0.0f, 0.0f, 1.0f, 0.0f };
    XMVECTOR m_vUp       = { 0.0f, 1.0f, 0.0f, 0.0f };//which axis is upward
	XMVECTOR m_vRotation = { 0.0f, 0.0f, 0.0f, 0.0f };

	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;// think of the frustum as the lens of our camera, for it controls our view

	ID3D11Buffer* m_pCamBuffer;

	float m_fFOV;
	float m_fScreenRatio;
	float m_fScreenNear;
	float m_fScreenFar;

	float m_fTranslateSpeed = 0.1f;
	float m_fRotateSpeed = 0.1f;
	float m_fZoomSpeed = 0.1f;


};

#endif 
