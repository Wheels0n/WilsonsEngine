#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../ImGui/imgui.h"
#include <d3d11.h>
class CViewport
{
public:
	CViewport()=default;
	CViewport(const CViewport&)=delete;
	~CViewport() = default;

	void Init(ID3D11ShaderResourceView*);
	void Draw();
private:
	ID3D11ShaderResourceView* m_pSRV;
};
#endif // !VIEWPORT_H

