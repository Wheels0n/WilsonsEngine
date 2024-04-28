#pragma once
#include"Camera12.h"
#include"typedef.h"
namespace wilson {
	class Frustum12
	{
	public:
		inline DirectX::XMVECTOR* GetPlanes()
		{
			return m_planes;
		}
		inline void SetSubMeshesInFrustum(const UINT cnt)
		{
			m_subMeshesInFrustum = cnt;
		}
		inline void SetSubMeshesInScene(const UINT cnt)
		{
			m_subMeshesInScene = cnt;
		}
		inline UINT GetSubMeshesInFrustum() const
		{
			return m_subMeshesInFrustum;
		}
		inline UINT GetSubMeshesInScene() const
		{
			return m_subMeshesInScene;
		}


		Frustum12() = default;
		Frustum12(Camera12* const);
		~Frustum12() = default;

	private:
		Frustum12* m_pFrustum;
		DirectX::XMVECTOR m_planes[6];

		UINT m_subMeshesInFrustum;
		UINT m_subMeshesInScene;
	};
}