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

		void UpdateAABB(const DirectX::XMMATRIX& transform);
		bool IsOnFrustum(const DirectX::XMVECTOR* pPlanes) const;
		
		AABB() = default;
		AABB(const DirectX::XMFLOAT3 minAABB, const DirectX::XMFLOAT3 maxAABB);
		AABB(const DirectX::XMVECTOR& center, const DirectX::XMVECTOR& extent);
		~AABB();
		
	private:
		bool IsOnOrForwardPlane(const DirectX::XMVECTOR& palne) const;
		void UpdateVertices();
	public:
	private:
		DirectX::XMVECTOR m_localCenter;
		DirectX::XMVECTOR m_center;
		DirectX::XMFLOAT3 m_cubeVertices[8];
		DirectX::XMVECTOR m_extent;
	};
}