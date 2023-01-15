#ifndef EDITOR_H
#define EDITOR_H

#include"Scene.h"
#include"ContentBrowser.h"
#include"Viewport.h"
#include"Fps.h"
class CEditor
{
public:
	void Init(CD3D11*);
	void Draw();
private:
	CContentBrowser m_CContentBrowser;
	CScene m_Scene;
	CViewport m_CViewport;
	CFps m_CFps;
};
#endif // !EDITOR_H
