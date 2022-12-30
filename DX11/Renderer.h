#ifndef _RENDERER_H_
#define _RENDERER_H_

#include<Windows.h>
#include "D3D11.h"

const bool g_bFull_SCREEN = false;
const bool g_bVSYNC_ENABLE = true;

class CRenderer
{
public:
	CRenderer();
	CRenderer(CRenderer&) = delete;
	~CRenderer();

	bool Init(int, int, HWND);
	void Shutdown();
	bool Frame();
	void TranslateRight();
	void TranslateLeft();
	void TranslateDown();
	void TranslateUp();
	void RotateLeft();
	void RotateRight();
	void ZoomIn();
	void ZoomOut();
	
private:
	bool Render();
	CD3D11* m_pD3D11;
};

#endif