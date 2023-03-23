#include "Frustum.h"

void CFrustum::Construct(float screenDepth, Camera* pCam)
{   
	XMMATRIX projectionMat = *(pCam->GetProjectionMatrix());
	XMFLOAT4X4 projectionMat4;
	XMStoreFloat4x4(&projectionMat4, projectionMat);

	float zNear = -projectionMat4._43 / projectionMat4._33; //(-n*f/(far-near)) / (far/(far-near)) ;
	float r = screenDepth / (screenDepth - zNear);

	//새 z 정규화 
	projectionMat4._33 = r;
	projectionMat4._43 = -r * zNear;
	projectionMat = XMLoadFloat4x4(&projectionMat4);

	XMMATRIX toNDC = XMMatrixMultiply(*(pCam->GetViewMatrix()), projectionMat);

	XMFLOAT4X4 toNDC4;
	XMStoreFloat4x4(&toNDC4, toNDC);
	
	//col4= z, col3 = Az+B, col2 = y, col1 = x 평면방정식 참고
	float x = (float)(toNDC4._14 + toNDC4._13); 
	float y = (float)(toNDC4._24 + toNDC4._23);
	float z = (float)(toNDC4._34 + toNDC4._33);
	float w = (float)(toNDC4._44 + toNDC4._43);
	m_plane[0] = XMVectorSet(x, y, z, w);
	m_plane[0] = XMPlaneNormalize(m_plane[0]);

	x = (float)(toNDC4._14 - toNDC4._13);
	y = (float)(toNDC4._24 - toNDC4._23);
	z = (float)(toNDC4._34 - toNDC4._33);
	w = (float)(toNDC4._44 - toNDC4._43);
	m_plane[1] = XMVectorSet(x, y, z, w);
	m_plane[1] = XMPlaneNormalize(m_plane[1]);

	x = (float)(toNDC4._14 + toNDC4._11);
	y = (float)(toNDC4._24 + toNDC4._21);
	z = (float)(toNDC4._34 + toNDC4._31);
	w = (float)(toNDC4._44 + toNDC4._41);
	m_plane[2] = XMVectorSet(x, y, z, w);
	m_plane[2] = XMPlaneNormalize(m_plane[2]);

	x = (float)(toNDC4._14 - toNDC4._11);
	y = (float)(toNDC4._24 - toNDC4._21);
	z = (float)(toNDC4._34 - toNDC4._31);
	w = (float)(toNDC4._44 - toNDC4._41);
	m_plane[3] = XMVectorSet(x, y, z, w);
	m_plane[3] = XMPlaneNormalize(m_plane[3]);

	x = (float)(toNDC4._14 + toNDC4._12);
	y = (float)(toNDC4._24 + toNDC4._22);
	z = (float)(toNDC4._34 + toNDC4._32);
	w = (float)(toNDC4._44 + toNDC4._42);
	m_plane[4] = XMVectorSet(x, y, z, w);
	m_plane[4] = XMPlaneNormalize(m_plane[4]);

	x = (float)(toNDC4._14 - toNDC4._12);
	y = (float)(toNDC4._24 - toNDC4._22);
	z = (float)(toNDC4._34 - toNDC4._32);
	w = (float)(toNDC4._44 - toNDC4._42);
	m_plane[5] = XMVectorSet(x, y, z, w);
	m_plane[5] = XMPlaneNormalize(m_plane[5]);

	return;
}

bool CFrustum::IsInFrustum(XMVECTOR pos)
{   
	for (int i = 0; i < 6; ++i)
	{
		if (XMVectorGetX(XMPlaneDotCoord(m_plane[i], pos)) < -1.0f)
		{
			return false;
		}
	}
	return true;
}

