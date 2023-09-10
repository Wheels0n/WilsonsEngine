#pragma once

#include"Camera.h"

namespace wilson {
	class Frustum
	{
	public:
		void Init(float, Camera*);
		bool IsInFrustum(DirectX::XMVECTOR);
		
		Frustum() = default;
		~Frustum() = default;

	private:
		DirectX::XMVECTOR m_planes[6];
	};
}