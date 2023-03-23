#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../ImGui/imgui.h"
#include <d3d11.h>
#include "Scene.h"
#include "D3D11.h"
#include "import.h"

static const char* texFormats[] =
{
	"bmp", "dds", "png"
};

class CViewport
{
public:
	CViewport() = default;
	CViewport(const CViewport&)=delete;
	~CViewport() = default;

	void Init(CD3D11*, CScene* );
	void Draw();
	void Resize();
	bool CheckRange(int, int);
	float GetNDCX(int);
	float GetNDCY(int);

	int  GetWidth() const
	{
		return m_width;
	};

	int GetHeight() const
	{
		return m_height;
	};

	int GetX() const
	{
		return m_left;
	};

	int GetY() const
	{  
		return m_top;
	};
private:
	bool m_IsFocused;

	int m_width;
	int m_height;
	int m_left;
	int m_top;

	CD3D11* m_pD3D11;
	Camera* m_pCCam;
	CScene* m_pCScene;
	Importer m_CImporter;
	IDXGISwapChain* m_pSwapChain;
	ID3D11ShaderResourceView* m_pSRV;
	ID3D11Device* m_pDevice;
};
#endif // !VIEWPORT_H

