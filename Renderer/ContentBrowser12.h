#pragma once

#include <d3d12.h>
#include "typedef.h"
#include "TextureManager.h"
#include "../ImGui/imgui.h"
namespace wilson
{
	using namespace Microsoft::WRL;
	using namespace std;
	
	class D3D12;
	class HeapManager;
	class ContentBrowser12
	{
		public:
			void									List();

													ContentBrowser12(shared_ptr<D3D12>);
													ContentBrowser12(const ContentBrowser12&) = delete;
													~ContentBrowser12() = default;

		private:
			void									ParseExtension(char*, const char*);
		private:
			std::filesystem::path					m_pCurDir = "Assets";
			//D3D12
			shared_ptr<TextureWrapper>				m_pDirIconTex;
			shared_ptr<TextureWrapper>				m_pFileIconTex;

	};
}