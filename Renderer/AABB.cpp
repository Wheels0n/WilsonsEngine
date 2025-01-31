
#include "AABB.h"

namespace wilson
{
	using namespace DirectX;
	AABB::AABB(const XMFLOAT3 minAABB, const XMFLOAT3 maxAABB)
	{	
		XMFLOAT4 center((minAABB.x + maxAABB.x) * 0.5f, (minAABB.y + maxAABB.y) * 0.5f, (minAABB.z + maxAABB.z) * 0.5f, 1.0f);
		m_center = XMLoadFloat4(&center);
		m_localCenter = m_center;

		XMFLOAT3 extents((maxAABB.x-center.x), (maxAABB.y-center.y) , (maxAABB.z-center.z));
		m_extent = XMLoadFloat3(&extents);
		UpdateBound();
	}
	AABB::AABB(const XMVECTOR& center, const XMVECTOR& extent)
		:m_center(center), m_extent(extent)
	{	
		UpdateBound();
	}
	XMVECTOR AABB::GetCenter()
	{
		return m_center;
	}
	XMVECTOR AABB::GetExtent()
	{
		return m_extent;
	}
	XMFLOAT3* AABB::GetBound()
	{
		return m_bound;
	}
	void AABB::UpdateAABB(const XMMATRIX& transform)
	{
		XMVECTOR centerV = XMVector4Transform(m_localCenter, transform);

		XMFLOAT3 extents;
		XMStoreFloat3(&extents, m_extent);

		XMVECTOR right = XMVectorScale(transform.r[0], extents.x);
		XMVECTOR up = XMVectorScale(transform.r[1], extents.y);
		XMVECTOR forward = XMVectorScale(transform.r[2], -extents.z);


		XMVECTOR unit = XMVectorSet(1.0, 1.0f, 1.0f, 0.f);

		XMVECTOR dotRight   = XMVectorMultiply(unit, right);
		XMVECTOR dotUp      = XMVectorMultiply(unit, up);
		XMVECTOR dotForward = XMVectorMultiply(unit, forward);
		XMVECTOR extent     = XMVectorAdd(dotRight, DirectX::XMVectorAdd(dotUp, dotForward));

		m_center = centerV;
		m_extent = extent;
		UpdateBound();

	}
	void AABB::UpdateBound()
	{
		XMFLOAT3 center;
		XMStoreFloat3(&center, m_center);

		XMFLOAT3 extents;
		XMStoreFloat3(&extents, m_extent);
		m_bound[0] = { center.x - extents.x, center.y - extents.y, center.z - extents.z }; //좌, 하, 전 
		m_bound[1] = { center.x - extents.x, center.y - extents.y, center.z + extents.z };//좌 하 후
		m_bound[2] = { center.x + extents.x, center.y - extents.y, center.z + extents.z };//우 하 후
		m_bound[3] = { center.x + extents.x, center.y - extents.y, center.z - extents.z };//우 하 전

		m_bound[4] = { center.x - extents.x, center.y + extents.y, center.z - extents.z };//좌 상 전
		m_bound[5] = { center.x - extents.x, center.y + extents.y, center.z + extents.z };//좌 상 후
		m_bound[6] = { center.x + extents.x, center.y + extents.y, center.z + extents.z };//우 상 후
		m_bound[7] = { center.x + extents.x, center.y + extents.y, center.z - extents.z };//우상 전
	}
	BOOL AABB::IsOnFrustum(const XMVECTOR* pPlanes)
	{
		return (
			IsOnOrForwardPlane(pPlanes[0]) &&
			IsOnOrForwardPlane(pPlanes[1]) &&
			IsOnOrForwardPlane(pPlanes[2]) &&
			IsOnOrForwardPlane(pPlanes[3]) &&
			IsOnOrForwardPlane(pPlanes[4]) &&
			IsOnOrForwardPlane(pPlanes[5]));
	}
	BOOL AABB::IsOnOrForwardPlane(const XMVECTOR& plane)
	{	

		XMVECTOR norm = XMVectorAbs(plane);
		XMVECTOR normExtentDot = XMVector3Dot(m_extent, norm);
		const FLOAT r = normExtentDot.m128_f32[0];

		XMVECTOR centerPlaneDot = XMVector3Dot(plane, m_center);
		FLOAT signedDistanceToPlane =centerPlaneDot.m128_f32[0] - plane.m128_f32[3];
		return r <=abs(signedDistanceToPlane);
	}
}