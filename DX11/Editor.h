#ifndef EDITOR_H
#define EDITOR_H

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
	CFps m_CFps;
	CViewport m_CViewport;
};
#endif // !EDITOR_H
