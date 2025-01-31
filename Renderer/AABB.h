#pragma once

#include "typedef.h"
namespace wilson
{	
	using namespace DirectX;
	struct VertexInfo;
	class AABB
	{
	public:
		XMVECTOR				GetCenter();
		XMVECTOR				GetExtent();
		XMFLOAT3*				GetBound() ;

		void					UpdateAABB (const XMMATRIX& transform);
		BOOL					IsOnFrustum(const XMVECTOR* pPlanes);
		
								AABB(const XMFLOAT3  minAABB, const XMFLOAT3  maxAABB);
								AABB(const XMVECTOR& center,  const XMVECTOR& extent);
								~AABB() = default;
		
	private:
		BOOL					IsOnOrForwardPlane(const XMVECTOR& plane);
		void					UpdateBound();
	private:
		XMVECTOR				m_localCenter;
		XMVECTOR				m_center;
		XMVECTOR				m_extent;

		XMFLOAT3				m_bound[_CUBE_VERTICES];
	};
}