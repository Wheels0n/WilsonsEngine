#include "Frustum.h"

namespace wilson {
	void Frustum::Construct(float screenDepth, Camera* pCam)
	{
		DirectX::XMMATRIX projMat = *(pCam->GetProjectionMatrix());
		DirectX::XMFLOAT4X4 projMat4;
		XMStoreFloat4x4(&projMat4, projMat);

		float zNear = -projMat4._43 / projMat4._33; //(-n*f/(far-near)) / (far/(far-near)) ;
		float r = screenDepth / (screenDepth - zNear);

		//새 z 정규화 
		projMat4._33 = r;
		projMat4._43 = -r * zNear;
		projMat = XMLoadFloat4x4(&projMat4);

		DirectX::XMMATRIX toNDC = XMMatrixMultiply(*(pCam->GetViewMatrix()), projMat);

		DirectX::XMFLOAT4X4 toNDC4;
		XMStoreFloat4x4(&toNDC4, toNDC);

		//col4= z, col3 = Az+B, col2 = y, col1 = x 평면방정식 참고
		float x = (float)(toNDC4._14 + toNDC4._13);
		float y = (float)(toNDC4._24 + toNDC4._23);
		float z = (float)(toNDC4._34 + toNDC4._33);
		float w = (float)(toNDC4._44 + toNDC4._43);
		m_planes[0] = DirectX::XMVectorSet(x, y, z, w);
		m_planes[0] = DirectX::XMPlaneNormalize(m_planes[0]);

		x = (float)(toNDC4._14 - toNDC4._13);
		y = (float)(toNDC4._24 - toNDC4._23);
		z = (float)(toNDC4._34 - toNDC4._33);
		w = (float)(toNDC4._44 - toNDC4._43);
		m_planes[1] = DirectX::XMVectorSet(x, y, z, w);
		m_planes[1] = DirectX::XMPlaneNormalize(m_planes[1]);

		x = (float)(toNDC4._14 + toNDC4._11);
		y = (float)(toNDC4._24 + toNDC4._21);
		z = (float)(toNDC4._34 + toNDC4._31);
		w = (float)(toNDC4._44 + toNDC4._41);
		m_planes[2] = DirectX::XMVectorSet(x, y, z, w);
		m_planes[2] = DirectX::XMPlaneNormalize(m_planes[2]);

		x = (float)(toNDC4._14 - toNDC4._11);
		y = (float)(toNDC4._24 - toNDC4._21);
		z = (float)(toNDC4._34 - toNDC4._31);
		w = (float)(toNDC4._44 - toNDC4._41);
		m_planes[3] = DirectX::XMVectorSet(x, y, z, w);
		m_planes[3] = DirectX::XMPlaneNormalize(m_planes[3]);

		x = (float)(toNDC4._14 + toNDC4._12);
		y = (float)(toNDC4._24 + toNDC4._22);
		z = (float)(toNDC4._34 + toNDC4._32);
		w = (float)(toNDC4._44 + toNDC4._42);
		m_planes[4] = DirectX::XMVectorSet(x, y, z, w);
		m_planes[4] = DirectX::XMPlaneNormalize(m_planes[4]);

		x = (float)(toNDC4._14 - toNDC4._12);
		y = (float)(toNDC4._24 - toNDC4._22);
		z = (float)(toNDC4._34 - toNDC4._32);
		w = (float)(toNDC4._44 - toNDC4._42);
		m_planes[5] = DirectX::XMVectorSet(x, y, z, w);
		m_planes[5] = DirectX::XMPlaneNormalize(m_planes[5]);

		return;
	}

	bool Frustum::IsInFrustum(DirectX::XMVECTOR pos)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (DirectX::XMVectorGetX(
				DirectX::XMPlaneDotCoord(m_planes[i], pos)) < -1.0f)
			{
				return false;
			}
		}
		return true;
	}
}
