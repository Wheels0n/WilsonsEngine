#pragma once 
#include"Editor.h"
#include"typedef.h"
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
		~Editor12() override;
	private:
		D3D12* m_pD3D12;
		std::unique_ptr<Settings12> m_pSettings;
		std::unique_ptr<Scene12> m_pScene;
		std::unique_ptr<Viewport12> m_pViewport;
		std::unique_ptr<ContentBrowser12> m_pContentBrowser;
	};
}
