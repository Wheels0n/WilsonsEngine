#pragma once

#include "Frustum11.h"
#include "typedef.h"
namespace wilson
{	
	struct VertexData;
	class AABB
	{
	public:
		inline DirectX::XMVECTOR GetCenter()const
		{
			return m_center;
		}
		inline DirectX::XMVECTOR GetExtent()const
		{
			return m_extent;
		}
		inline DirectX::XMFLOAT3* GetVertices()
		{
			return m_cubeVertices;
		}

		bool IsOnFrustum(const Plane* pPlanes, const DirectX::XMMATRIX transfrom) const;
		AABB MultiplyWorldMatrix(const DirectX::XMMATRIX transform);

		AABB() = default;
		AABB(const DirectX::XMFLOAT3 minAABB, const DirectX::XMFLOAT3 maxAABB);
		AABB(const DirectX::XMFLOAT4 center, const float il, const float ij, const float ik);
		~AABB();
		
	private:
		bool IsOnOrForwardPlane(const Plane& palne) const;
		void UpdateVertices();
	public:
	private:
		DirectX::XMVECTOR m_center;
		DirectX::XMFLOAT3 m_cubeVertices[8];
		DirectX::XMVECTOR m_extent;
	};
}