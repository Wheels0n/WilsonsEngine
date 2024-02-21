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
		void Draw();
		void Pick();
		bool CheckRange(int, int);

		Editor12() = default;
		Editor12(D3D12*);
		~Editor12();
	private:
		D3D12* m_pD3D12;
		ContentBrowser12* m_pContentBrowser12;
		Scene12* m_pScene12;
		Viewport12* m_pViewport12;
		Settings12* m_pSettings12;
	};
}
