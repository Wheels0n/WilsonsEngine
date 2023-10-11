#pragma once 

#include"Scene.h"
#include"ContentBrowser.h"
#include"Viewport.h"
#include"Settings.h"

namespace wilson {
	class Editor
	{
	public:
		void Draw();
		void Pick();
		bool CheckRange(int, int);
		
		Editor() = default;
		Editor(D3D11*);
		~Editor();
	private:
		D3D11* m_pD3D11;
		ContentBrowser* m_pContentBrowser;
		Scene* m_pScene;
		Viewport* m_pViewport;
		Settings* m_pSettings;
	};
}
