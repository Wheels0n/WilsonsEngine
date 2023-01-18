#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../ImGui/imgui.h"
#include <d3d11.h>
#include "Scene.h"
#include "D3D11.h"
#include "import.h"
class CViewport
{
public:
	CViewport() = default;
	CViewport(const CViewport&)=delete;
	~CViewport() = default;

	void Init(CD3D11*, CScene* );
	void Draw();
private:
	int width;
	int height;
	int left;
	int top;

	CD3D11* m_pCD3D11;
	CScene* m_pCScene;
	CImporter m_CImporter;
	ID3D11ShaderResourceView* m_pSRV;
	ID3D11Device* m_pDevice;
};
#endif // !VIEWPORT_H

