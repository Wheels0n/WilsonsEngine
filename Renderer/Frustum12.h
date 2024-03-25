#pragma once
#include"Camera12.h"
#include"typedef.h"
namespace wilson {
	class Frustum12
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
		inline DirectX::XMVECTOR* GetPlanes()
		{
			return m_planes;
		}


		Frustum12() = default;
		Frustum12(Camera12*);
		~Frustum12() = default;

	private:
		UINT m_ENTTsInFrustum;
		UINT m_ENTTsInTotal;
		DirectX::XMVECTOR m_planes[6];
		Frustum* m_pFrustum;
	};
}