#include "Frustum12.h"

namespace wilson {
	using namespace DirectX;
	Frustum12::Frustum12(Camera12* pCam)
		:m_nMeshletInFrustum(0), m_nMeshletInScene(0)
	{
		XMMATRIX& toNDC = (pCam->GetViewProjectionMatrix());
		
		//col4= z, col3 = Az+B, col2 = y, col1 = x

		m_planes[0] = toNDC.r[2];
		m_planes[1] = XMVectorSubtract(toNDC.r[3], toNDC.r[2]);
		m_planes[2] = XMVectorAdd(toNDC.r[3], toNDC.r[0]);
		m_planes[3] = XMVectorSubtract(toNDC.r[3], toNDC.r[0]);
		m_planes[4] = XMVectorSubtract(toNDC.r[3], toNDC.r[1]);
		m_planes[5] = XMVectorAdd(toNDC.r[3], toNDC.r[1]);

		for (int i = 0; i < 6; ++i)
		{
			m_planes[i]= XMPlaneNormalize(m_planes[i]);
		}
	}
	XMVECTOR* Frustum12::GetPlanes()
	{
		return m_planes;
	}
	UINT Frustum12::GetNumOfMeshletInFrustum()
	{
		return m_nMeshletInFrustum;
	}
	UINT Frustum12::GetNumOfMeshletInScene()
	{
		return m_nMeshletInScene;
	}
	void Frustum12::SetNumOfMeshletInFrustum(const UINT cnt)
	{
		m_nMeshletInFrustum = cnt;
	}
	void Frustum12::SetNumOfMeshletInScene(const UINT cnt)
	{
		m_nMeshletInScene = cnt;
	}
}
