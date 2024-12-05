#include "Mesh11.h"
#include "AABB.h"

namespace wilson
{
	AABB::AABB(const DirectX::XMFLOAT3 minAABB, const DirectX::XMFLOAT3 maxAABB)
	{	
		DirectX::XMFLOAT4 center((minAABB.x + maxAABB.x) * 0.5f, (minAABB.y + maxAABB.y) * 0.5f, (minAABB.z + maxAABB.z) * 0.5f, 1.0f);
		m_center = DirectX::XMLoadFloat4(&center);
		m_localCenter = m_center;

		DirectX::XMFLOAT3 extents((maxAABB.x-center.x), (maxAABB.y-center.y) , (maxAABB.z-center.z));
		m_extent = DirectX::XMLoadFloat3(&extents);
		UpdateVertices();
	}
	AABB::AABB(const DirectX::XMVECTOR& center, const DirectX::XMVECTOR& extent)
	{	

		m_center = center;
		m_extent = extent;
		UpdateVertices();

	}
	AABB::~AABB()
	{

	}
	void AABB::UpdateVertices()
	{
		DirectX::XMFLOAT3 center;
		DirectX::XMStoreFloat3(&center, m_center);

		DirectX::XMFLOAT3 extents;
		DirectX::XMStoreFloat3(&extents, m_extent);
		m_cubeVertices[0] = { center.x - extents.x, center.y - extents.y, center.z - extents.z }; //좌, 하, 전 
		m_cubeVertices[1] = { center.x - extents.x, center.y - extents.y, center.z + extents.z };//좌 하 후
		m_cubeVertices[2] = { center.x + extents.x, center.y - extents.y, center.z + extents.z };//우 하 후
		m_cubeVertices[3] = { center.x + extents.x, center.y - extents.y, center.z - extents.z };//우 하 전

		m_cubeVertices[4] = { center.x - extents.x, center.y + extents.y, center.z - extents.z };//좌 상 전
		m_cubeVertices[5] = { center.x - extents.x, center.y + extents.y, center.z + extents.z };//좌 상 후
		m_cubeVertices[6] = { center.x + extents.x, center.y + extents.y, center.z + extents.z };//우 상 후
		m_cubeVertices[7] = { center.x + extents.x, center.y + extents.y, center.z - extents.z };//우상 전
	}
	bool AABB::IsOnOrForwardPlane(const DirectX::XMVECTOR& plane) const
	{	

		DirectX::XMVECTOR norm = DirectX::XMVectorAbs(plane);
		DirectX::XMVECTOR normExtentDot = DirectX::XMVector3Dot(m_extent, norm);
		const float r = normExtentDot.m128_f32[0];

		DirectX::XMVECTOR centerPlaneDot = DirectX::XMVector3Dot(plane, m_center);
		float signedDistanceToPlane =centerPlaneDot.m128_f32[0] - plane.m128_f32[3];
		return r <=abs(signedDistanceToPlane);
	}
	void AABB::UpdateAABB(const DirectX::XMMATRIX& transform)
	{
		DirectX::XMVECTOR centerV = DirectX::XMVector4Transform(m_localCenter, transform);

		DirectX::XMFLOAT3 extents;
		DirectX::XMStoreFloat3(&extents, m_extent);

		DirectX::XMVECTOR right = DirectX::XMVectorScale(transform.r[0], extents.x);
		DirectX::XMVECTOR up = DirectX::XMVectorScale(transform.r[1], extents.y);
		DirectX::XMVECTOR forward = DirectX::XMVectorScale(transform.r[2], -extents.z);


		DirectX::XMVECTOR unit = DirectX::XMVectorSet(1.0, 1.0f, 1.0f, 0.f);

		DirectX::XMVECTOR dotRight = DirectX::XMVectorMultiply(unit, right);
		DirectX::XMVECTOR dotUp = DirectX::XMVectorMultiply(unit, up);
		DirectX::XMVECTOR dotForward = DirectX::XMVectorMultiply(unit, forward);
		DirectX::XMVECTOR extent = DirectX::XMVectorAdd(dotRight, DirectX::XMVectorAdd(dotUp, dotForward));

		m_center = centerV;
		m_extent = extent;
		UpdateVertices();

	}
	bool AABB::IsOnFrustum(const DirectX::XMVECTOR* pPlanes) const
	{

		return (
			IsOnOrForwardPlane(pPlanes[0])&&
			IsOnOrForwardPlane(pPlanes[1])&&
			IsOnOrForwardPlane(pPlanes[2])&&
			IsOnOrForwardPlane(pPlanes[3])&&
			IsOnOrForwardPlane(pPlanes[4])&&
			IsOnOrForwardPlane(pPlanes[5]));
	}
}