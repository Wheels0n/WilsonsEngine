#ifndef _RENDERER_H_
#define _RENDERER_H_

#include<Windows.h>
#include "D3D11.h"

const bool g_bFull_SCREEN = false;
const bool g_bVSYNC_ENABLE = true;
const float g_fSCREEN_DEPTH = 1000.0f;
const float g_fSCREEN_NEAR = 0.1f;

class CRenderer
{
public:
	CRenderer();
	CRenderer(CRenderer&) = delete;
	~CRenderer();

	bool Init(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render();
	CD3D11* m_pD3D11;
};

#endif