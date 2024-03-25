#pragma once

#include <DirectXMath.h>
#include <vector>

#include "Frustum.h"
#include "typedef.h"

namespace wilson
{
	struct VertexData;
	class Sphere
	{
	public:
		inline DirectX::XMFLOAT3 GetCenter()
		{
			return m_center;
		}

		inline float GetRadius()
		{
			return m_radius;
		}

		bool IsOnFrustum(const Plane* pPlanes, const DirectX::XMMATRIX transfrom) const;

		Sphere(DirectX::XMFLOAT3 center, float r) :
			m_center{ center }, m_radius{ r } {};
		~Sphere()=default;

	private:
		bool IsOnOrForwardPlane(const Plane& plane) const;

		DirectX::XMFLOAT3 m_center;
		float m_radius;
	};

}
