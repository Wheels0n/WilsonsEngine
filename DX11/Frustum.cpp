#include "Frustum.h"

namespace wilson {
	void Frustum::Init(Camera* pCam)
	{	
		m_ENTTsInFrustum = 0;
		m_ENTTsInTotal = 0;
		float zFar = *(pCam->GetFarZ());
		float zNear = *(pCam->GetNearZ());
		float ratio = pCam->GetAspect();
		float fovY = *(pCam->GetFovY());

		const float halfVSide = zFar * tanf(fovY * 0.5f);
		const float halfHSide = halfVSide * ratio;
		
		DirectX::XMVECTOR frontV = pCam->GetDir();
		frontV = DirectX::XMVector3Normalize(frontV);

		DirectX::XMVECTOR frontMulNear = DirectX::XMVectorScale(frontV, zNear);
		DirectX::XMVECTOR frontMulFar = DirectX::XMVectorScale(frontV, zFar);

      	DirectX::XMVECTOR posV = *(pCam->GetPosition());
		DirectX::XMVECTOR right = pCam->GetRight();
		DirectX::XMVECTOR up = pCam->GetUp();

		DirectX::XMVECTOR dot;


		//near, far, right, left, top, bottom
		m_planes[0].norm = frontV;
		dot = DirectX::XMVector3Dot(DirectX::XMVectorAdd(posV, frontMulNear), m_planes[0].norm);
		DirectX::XMStoreFloat(&m_planes[0].d, dot);

		frontV = DirectX::XMVectorScale(frontV, -1);
		m_planes[1].norm = frontV;
		dot = DirectX::XMVector3Dot(DirectX::XMVectorAdd(posV, frontMulFar), m_planes[1].norm);
		DirectX::XMStoreFloat(&m_planes[1].d, dot);

		DirectX::XMVECTOR cross = DirectX::XMVectorScale(right, halfHSide);
		cross = DirectX::XMVectorSubtract(frontMulFar, cross);
		cross = DirectX::XMVector3Cross(cross, up);
		m_planes[2].norm = DirectX::XMVector3Normalize(cross);
		dot = DirectX::XMVector3Dot(posV, m_planes[2].norm);
		DirectX::XMStoreFloat(&m_planes[2].d, dot);


		cross = DirectX::XMVectorScale(right, halfHSide);
		cross = DirectX::XMVectorAdd(frontMulFar, cross);
		cross = DirectX::XMVector3Cross(up,cross);
		m_planes[3].norm = DirectX::XMVector3Normalize(cross);
		dot = DirectX::XMVector3Dot(posV, m_planes[3].norm);
		DirectX::XMStoreFloat(&m_planes[3].d, dot);

		cross = DirectX::XMVectorScale(up, halfVSide);
		cross = DirectX::XMVectorSubtract(frontMulFar, cross);
		cross = DirectX::XMVector3Cross(right, cross);
		m_planes[4].norm = DirectX::XMVector3Normalize(cross);
		dot = DirectX::XMVector3Dot(posV, m_planes[4].norm);
		DirectX::XMStoreFloat(&m_planes[4].d, dot);

		cross = DirectX::XMVectorScale(up, halfVSide);
		cross = DirectX::XMVectorAdd(frontMulFar, cross);
		cross = DirectX::XMVector3Cross(cross,right);
		m_planes[5].norm = DirectX::XMVector3Normalize(cross);
		dot = DirectX::XMVector3Dot(posV,m_planes[5].norm);
		DirectX::XMStoreFloat(&m_planes[5].d, dot);
		return;
	}


}
