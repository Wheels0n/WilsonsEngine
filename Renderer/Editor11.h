#pragma once 
#include"Editor.h"
namespace wilson {

	class D3D11;
	class ContentBrowser;
	class Scene;
	class Viewport;
	class Settings;

	class Editor11 : public Editor
	{
	public:
		void Draw();
		void Pick();
		bool CheckRange(int, int);
		
		Editor11() = default;
		Editor11(D3D11*);
		~Editor11();
	private:
		D3D11* m_pD3D11;
		ContentBrowser* m_pContentBrowser;
		Scene* m_pScene;
		Viewport* m_pViewport;
		Settings* m_pSettings;
	};
}
