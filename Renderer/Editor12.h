#pragma once 
#include"Editor.h"
namespace wilson {
	class D3D12;
	class ContentBrowser12;
	class Scene12;
	class Viewport12;
	class Settings12;

	class Editor12 : public Editor
	{
	public:
		bool CheckRange(int, int);
		void Draw();
		void Pick();

		Editor12() = default;
		Editor12(D3D12*const);
		~Editor12();
	private:
		ContentBrowser12* m_pContentBrowser;
		D3D12* m_pD3D12;
		Scene12* m_pScene;
		Settings12* m_pSettings;
		Viewport12* m_pViewport;
	};
}
