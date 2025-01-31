#pragma once 

#include"typedef.h"
namespace wilson {
	using namespace std;

	class D3D12;
	class ContentBrowser12;
	class Scene12;
	class Viewport12;
	class Settings12;

	class Editor12 
	{
		public:
			BOOL							CheckRange(int, int);
			void							Draw();
			void							Pick();

											Editor12(shared_ptr<D3D12>);
											~Editor12() = default;
		private:
			shared_ptr<Settings12>			m_pSettings;
			shared_ptr<Scene12>				m_pScene;
			shared_ptr<Viewport12>			m_pViewport;
			shared_ptr<ContentBrowser12>	m_pContentBrowser;
	};
}
