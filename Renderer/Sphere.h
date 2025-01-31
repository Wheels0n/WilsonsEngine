#pragma once

#include "typedef.h"

namespace wilson
{
	using namespace DirectX;
	struct VertexInfo;
	class Sphere
	{
	public:
		XMVECTOR					GetCenter();
		FLOAT						GetRadius();

		void						UpdateSphere(const XMMATRIX& transform);
		BOOL						IsOnFrustum(const XMVECTOR* pPlanes);

									Sphere(const XMVECTOR& center, const FLOAT r);
									~Sphere() = default;

	private:
		BOOL						IsOnOrForwardPlane(const XMVECTOR& plane);
	private:
		XMVECTOR					m_localCenter;
		XMVECTOR					m_center;
		FLOAT						m_radius;
	};

}
