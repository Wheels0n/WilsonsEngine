#pragma once
#include"Camera11.h"
#include"typedef.h"
namespace wilson {
	
	class Frustum11
	{
	public:
		inline Plane* GetPlanes()
		{
			return m_planes;
		}
		inline UINT GetSubMeshesInFrustum() const
		{
			return m_subMeshesInFrustum;
		}
		inline UINT GetSubMeshesInScene() const
		{
			return m_subMeshesInScene;
		}

		inline void SetSubMeshesInFrustum(const UINT cnt)
		{
			m_subMeshesInFrustum = cnt;
		}
		inline void SetSubMeshesInScene(const UINT cnt)
		{
			m_subMeshesInScene = cnt;
		}
		
		Frustum11() = default;
		Frustum11(Camera11* const);
		~Frustum11() = default;

	private:
		Frustum11* m_pFrustum;
		Plane m_planes[6];
		UINT m_subMeshesInFrustum;
		UINT m_subMeshesInScene;
	};
}