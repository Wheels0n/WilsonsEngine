#include "Sphere.h"
namespace wilson
{
	using namespace DirectX;
	Sphere::Sphere(const XMVECTOR& center, const FLOAT r)
		: m_localCenter{ center },m_center {center}, m_radius{ r }
	{
	};
	XMVECTOR Sphere::GetCenter()
	{
		return m_center;
	}
	FLOAT Sphere::GetRadius()
	{
		return m_radius;
	}
	void Sphere::UpdateSphere(const XMMATRIX& transform)
	{

		XMVECTOR xV = transform.r[0];
		XMVECTOR yV = transform.r[1];
		XMVECTOR zV = transform.r[2];

		const XMFLOAT3 globalScale = XMFLOAT3{
			XMVector4Length(xV).m128_f32[0],
			XMVector4Length(yV).m128_f32[0],
			XMVector4Length(zV).m128_f32[0],
		};
		float maxScale = max(max(globalScale.x, globalScale.y), globalScale.z);
		maxScale *= 0.5f;
		m_localCenter = XMVector4Transform(m_center, transform);
		m_radius *= maxScale;
	}
	BOOL Sphere::IsOnFrustum(const XMVECTOR* pPlanes)
	{
		return (
			IsOnOrForwardPlane(pPlanes[0]) &&
			IsOnOrForwardPlane(pPlanes[1]) &&
			IsOnOrForwardPlane(pPlanes[2]) &&
			IsOnOrForwardPlane(pPlanes[3]) &&
			IsOnOrForwardPlane(pPlanes[4]) &&
			IsOnOrForwardPlane(pPlanes[5]));
	}
	BOOL Sphere::IsOnOrForwardPlane(const XMVECTOR& plane)
	{
		XMVECTOR dot = XMVector3Dot(plane, m_center);
		float signedDistanceToPlane = dot.m128_f32[0] - plane.m128_f32[3];
		return signedDistanceToPlane > -m_radius;
	}
}
