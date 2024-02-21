#pragma once
#include"Camera.h"
#include"typedef.h"
namespace wilson {
	
	class Frustum
	{
	public:
		inline void SetENTTsInFrustum(UINT cnt)
		{
			m_ENTTsInFrustum = cnt;
		}
		inline void SetENTTsInTotal(UINT cnt)
		{
			m_ENTTsInTotal = cnt;
		}
		inline int GetENTTsInFrustum() const
		{
			return m_ENTTsInFrustum;
		}
		inline int GetENTTsInTotal() const
		{
			return m_ENTTsInTotal;
		}
		inline Plane* GetPlanes()
		{
			return m_planes;
		}

		
		Frustum() = default;
		Frustum(Camera*);
		~Frustum() = default;

	private:
		UINT m_ENTTsInFrustum;
		UINT m_ENTTsInTotal;
		Plane m_planes[6];
		Frustum* m_pFrustum;
	};
}