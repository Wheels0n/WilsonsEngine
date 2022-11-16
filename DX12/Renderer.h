#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "D3D.h"

const bool g_bFULL_SCREEN = false;
const bool g_bVSYNC_ENABLED = true;
const float g_fSCREEN_DEPTH = 1000.0f;
const float g_fSCREEN_NEAR = 0.1f;

class CRenderer
{
public:
	CRenderer();
	CRenderer(const CRenderer&);
	~CRenderer();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render();

private:
	CD3D* m_pD3D;
};
#endif