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
		~Editor11() override;
	private:
		D3D11* m_pD3D11;
		Settings11* m_pSettings;
		Viewport11* m_pViewport;
		Scene11* m_pScene;
		ContentBrowser11* m_pContentBrowser;
	};
}
