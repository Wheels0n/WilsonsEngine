#ifndef FRUSTUM_H
#define FRUSTUM_H
#include"camera.h"

class CFrustum
{
public:
	CFrustum() = default;
	~CFrustum() = default;
	void Construct(float, Camera*);
	bool IsInFrustum(XMVECTOR);
private:
	XMVECTOR m_plane[6];
};

#endif // !FRUSTUM_H
