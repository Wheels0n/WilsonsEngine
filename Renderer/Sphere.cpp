#include "Sphere.h"

bool wilson::Sphere::IsOnFrustum(const Plane* pPlanes, const DirectX::XMMATRIX transfrom) const
{	
	
	DirectX::XMFLOAT3 x;
	DirectX::XMStoreFloat3(&x, transfrom.r[0]);
	DirectX::XMFLOAT3 y;
	DirectX::XMStoreFloat3(&y, transfrom.r[1]);
	DirectX::XMFLOAT3 z;
	DirectX::XMStoreFloat3(&z, transfrom.r[2]);
	DirectX::XMVECTOR xV = DirectX::XMLoadFloat3(&x);
	DirectX::XMVECTOR yV = DirectX::XMLoadFloat3(&y);
	DirectX::XMVECTOR zV = DirectX::XMLoadFloat3(&z);

	const DirectX::XMFLOAT3 globalScale = DirectX::XMFLOAT3{
		DirectX::XMVector4Length(xV).m128_f32[0],
		DirectX::XMVector4Length(yV).m128_f32[0],
		DirectX::XMVector4Length(zV).m128_f32[0],
	};
	const float maxScale = max(max(globalScale.x, globalScale.y), globalScale.z);

	DirectX::XMFLOAT4 center4 = DirectX::XMFLOAT4{ m_center.x, m_center.y, m_center.z, 1.0f };
	const DirectX::XMVECTOR globalCenterV =
		DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&center4), DirectX::XMMatrixTranspose(transfrom));
	DirectX::XMFLOAT3 center3;
	DirectX::XMStoreFloat3(&center3, globalCenterV);

	Sphere globalSphere(center3, m_radius * maxScale * 0.5f);

	return globalSphere.IsOnOrForwardPlane(pPlanes[0])&&
		globalSphere.IsOnOrForwardPlane(pPlanes[1])&&
		globalSphere.IsOnOrForwardPlane(pPlanes[2])&&
		globalSphere.IsOnOrForwardPlane(pPlanes[3])&&
		globalSphere.IsOnOrForwardPlane(pPlanes[4])&&
		globalSphere.IsOnOrForwardPlane(pPlanes[5]);
}

bool wilson::Sphere::IsOnOrForwardPlane(const Plane& plane) const
{	
	DirectX::XMVECTOR centerV = DirectX::XMLoadFloat3(&m_center);
	DirectX::XMVECTOR dot = DirectX::XMVector3Dot(plane.norm, centerV);
	float signedDistanceToPlane = dot.m128_f32[0] - plane.d;
	return signedDistanceToPlane>-m_radius;
}
