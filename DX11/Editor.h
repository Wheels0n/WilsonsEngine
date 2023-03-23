#ifndef EDITOR_H
#define EDITOR_H

#include"Scene.h"
#include"ContentBrowser.h"
#include"Viewport.h"
#include"Settings.h"
class CEditor
{
public:
	void Init(CD3D11*);
	void Draw();
	void Pick();
	bool CheckRange(int, int);
private:
	CD3D11* m_pCD3D11;
	ContentBrowser m_CContentBrowser;
	CScene m_Scene;
	CViewport m_CViewport;
	CSettings m_CSettings;
};
#endif // !EDITOR_H
