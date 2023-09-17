#pragma once

#include <DirectXMath.h>
#include <vector>

#include "Frustum.h"
namespace wilson
{	
	struct VertexData;
	class AABB
	{
	public:
		static AABB GenAABB(const VertexData* pVertexData, UINT cnt);
		bool IsOnFrustum(const Plane* pPlanes, const DirectX::XMMATRIX transfrom) const;

		inline DirectX::XMVECTOR GetCenter()const
		{
			return m_center;
		}
		inline DirectX::XMVECTOR GetExtents()const
		{
			return m_extents;
		}
		inline DirectX::XMFLOAT3* GetVertices()
		{
			return m_cubeVertices;
		}

		AABB() = default;
		AABB(DirectX::XMFLOAT3 minAABB, DirectX::XMFLOAT3 maxAABB);
		AABB(DirectX::XMFLOAT4 center, float il, float ij, float ik);
		~AABB();
		
	private:
		bool IsOnOrForwardPlane(const Plane& palne) const;
		void UpdateVertices();
	public:
	private:
		DirectX::XMFLOAT3 m_cubeVertices[8];
		DirectX::XMVECTOR m_center;
		DirectX::XMVECTOR m_extents;
	};
}