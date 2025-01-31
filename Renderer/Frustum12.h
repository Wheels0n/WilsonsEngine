#pragma once
#include"Camera12.h"
#include"typedef.h"
namespace wilson {
	using namespace DirectX;
	using namespace std;

	class Camera12;
	class Frustum12
	{
	public:
		XMVECTOR*						GetPlanes();
		UINT							GetNumOfMeshletInFrustum();
		UINT							GetNumOfMeshletInScene();

		void							SetNumOfMeshletInFrustum(const UINT);
		void							SetNumOfMeshletInScene(const UINT);

										Frustum12(Camera12*);
										~Frustum12() = default;

	private:
		XMVECTOR						m_planes[6];

		UINT							m_nMeshletInFrustum;
		UINT							m_nMeshletInScene;
	};
}