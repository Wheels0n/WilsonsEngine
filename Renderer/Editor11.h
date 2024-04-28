#pragma once 
#include"Editor.h"
namespace wilson {

	class D3D11;
	class ContentBrowser11;
	class Scene11;
	class Viewport11;
	class Settings11;

	class Editor11 : public Editor
	{
	public:
		bool CheckRange(int, int);
		void Draw();
		void Pick();

		Editor11() = default;
		Editor11(D3D11*const);
		~Editor11();
	private:
		ContentBrowser11* m_pContentBrowser;
		D3D11* m_pD3D11;
		Scene11* m_pScene;
		Settings11* m_pSettings;
		Viewport11* m_pViewport;
	};
}
