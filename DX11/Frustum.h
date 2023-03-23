#ifndef FRUSTUM_H
#define FRUSTUM_H
#include"Camera.h"

namespace wilson {
	class Frustum
	{
	public:
		void Construct(float, Camera*);
		bool IsInFrustum(DirectX::XMVECTOR);
		
		Frustum() = default;
		~Frustum() = default;

	private:
		DirectX::XMVECTOR m_planes[6];
	};
}
#endif // !FRUSTUM_H
