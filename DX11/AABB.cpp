
#include "Model.h"
#include "AABB.h"

namespace wilson
{
	AABB AABB::GenAABB(const VertexData* pVertexData, UINT cnt)
	{
		DirectX::XMFLOAT3 minAABB(FLT_MAX, FLT_MAX, FLT_MAX);
		DirectX::XMFLOAT3 maxAABB(FLT_MIN, FLT_MIN, FLT_MIN);
		for (UINT i = 0; i < cnt; ++i)
		{
			minAABB.x = min(minAABB.x, pVertexData[i].position.x);
			minAABB.y = min(minAABB.y, pVertexData[i].position.y);
			minAABB.z = min(minAABB.z, pVertexData[i].position.z);

			maxAABB.x = max(maxAABB.x, pVertexData[i].position.x);
			maxAABB.y = max(maxAABB.y, pVertexData[i].position.y);
			maxAABB.z = max(maxAABB.z, pVertexData[i].position.z);
		}

		return AABB(minAABB, maxAABB);
	}
	AABB::AABB(DirectX::XMFLOAT3 minAABB, DirectX::XMFLOAT3 maxAABB)
	{	
		DirectX::XMFLOAT4 center((minAABB.x + maxAABB.x) * 0.5f, (minAABB.y + maxAABB.y) * 0.5f, (minAABB.z + maxAABB.z) * 0.5f, 1.0f);
		m_center = DirectX::XMLoadFloat4(&center);

		DirectX::XMFLOAT3 extents((maxAABB.x-center.x), (maxAABB.y-center.y) , (maxAABB.z-center.z));
		m_extents = DirectX::XMLoadFloat3(&extents);
		UpdateVertices();
	}
	AABB::AABB(DirectX::XMFLOAT4 center, float il, float ij, float ik)
	{	

		m_center = DirectX::XMLoadFloat4(&center);
		DirectX::XMFLOAT3 extents(il, ij, ik);
		m_extents = DirectX::XMLoadFloat3(&extents);
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
		DirectX::XMStoreFloat3(&extents, m_extents);
		m_cubeVertices[0] = { center.x - extents.x, center.y - extents.y, center.z - extents.z }; //좌, 하, 전 
		m_cubeVertices[1] = { center.x - extents.x, center.y - extents.y, center.z + extents.z };//좌 하 후
		m_cubeVertices[2] = { center.x + extents.x, center.y - extents.y, center.z + extents.z };//우 하 후
		m_cubeVertices[3] = { center.x + extents.x, center.y - extents.y, center.z - extents.z };//우 하 전

		m_cubeVertices[4] = { center.x - extents.x, center.y + extents.y, center.z - extents.z };//좌 상 전
		m_cubeVertices[5] = { center.x - extents.x, center.y + extents.y, center.z + extents.z };//좌 상 후
		m_cubeVertices[6] = { center.x + extents.x, center.y + extents.y, center.z + extents.z };//우 상 후
		m_cubeVertices[7] = { center.x + extents.x, center.y + extents.y, center.z - extents.z };//우상 전
	}
	bool AABB::IsOnOrForwardPlane(const Plane& plane) const
	{	
		DirectX::XMFLOAT3 extents;
		DirectX::XMStoreFloat3(&extents, m_extents);
		DirectX::XMFLOAT3 norm;
		DirectX::XMStoreFloat3(&norm, plane.norm);


		const float r = extents.x * std::abs(norm.x) + extents.y * std::abs(norm.y) +
			extents.z * std::abs(norm.z);

		DirectX::XMVECTOR dot = DirectX::XMVector3Dot(plane.norm, m_center);
		float signedDistanceToPlane = dot.m128_f32[0] - plane.d;
		return -r <=signedDistanceToPlane;
	}
	bool AABB::IsOnFrustum(const Plane* pPlanes, const DirectX::XMMATRIX transform) const
	{	

		DirectX::XMVECTOR globalCenterV = DirectX::XMVector4Transform(m_center, transform);
		DirectX::XMFLOAT4 globalCenter;
		DirectX::XMStoreFloat4(&globalCenter, globalCenterV);

		DirectX::XMFLOAT3 extents;
		DirectX::XMStoreFloat3(&extents, m_extents);
		
		
		DirectX::XMVECTOR right = DirectX::XMVectorScale(transform.r[0], extents.x);
		DirectX::XMVECTOR up = DirectX::XMVectorScale(transform.r[1], extents.y);
		DirectX::XMVECTOR forward = DirectX::XMVectorScale(transform.r[2], -extents.z);


		DirectX::XMVECTOR x = DirectX::XMVectorSet(1.0, 0.0f, 0.0f, 0.f);
		DirectX::XMVECTOR y = DirectX::XMVectorSet(0.0, 1.0f, 0.0f, 0.f);
		DirectX::XMVECTOR z = DirectX::XMVectorSet(0.0, 0.0f, 1.0f, 0.f);
		
		float dotRight;
		float dotUp;
		float dotForward;

		DirectX::XMStoreFloat(&dotRight,DirectX::XMVector3Dot(x, right));
		DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(x, up));
		DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(x, forward));

		const float newli = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);

		DirectX::XMStoreFloat(&dotRight, DirectX::XMVector3Dot(y, right));
		DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(y, up));
		DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(y, forward));

		const float newlj = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);

		DirectX::XMStoreFloat(&dotRight, DirectX::XMVector3Dot(z, right));
		DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(z, up));
		DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(z, forward));

		const float newlk = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);


		const AABB globalAABB(globalCenter, newli, newlj, newlk);

		return (
			globalAABB.IsOnOrForwardPlane(pPlanes[0])&&
			globalAABB.IsOnOrForwardPlane(pPlanes[1])&&
			globalAABB.IsOnOrForwardPlane(pPlanes[2])&&
			globalAABB.IsOnOrForwardPlane(pPlanes[3])&&
			globalAABB.IsOnOrForwardPlane(pPlanes[4])&&
			globalAABB.IsOnOrForwardPlane(pPlanes[5]));
	}
}