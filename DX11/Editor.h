#ifndef EDITOR_H
#define EDITOR_H

#include"Scene.h"
#include"ContentBrowser.h"
#include"Viewport.h"
#include"Settings.h"

namespace wilson {
	class Editor
	{
	public:
		void Init(CD3D11*);
		void Draw();
		void Pick();
		bool CheckRange(int, int);
	private:
		CD3D11* m_pD3D11;
		ContentBrowser m_contentBrowser;
		CScene m_scene;
		Viewport m_viewport;
		Settings m_settings;
	};
}
#endif // !EDITOR_H
