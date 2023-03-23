#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include"../ImGui/imgui.h"
#include<D3DX11tex.h>
#include<filesystem>

namespace wilson 
{
	class ContentBrowser
	{
	public:
		void Init(ID3D11Device*);
		void List();

		ContentBrowser() = default;
		ContentBrowser(const ContentBrowser&) = delete;
		~ContentBrowser() = default;

	private:
		void GetExtension(char*, const char*);
	private:
		std::filesystem::path m_curDir = "Assets";
		ID3D11ShaderResourceView* m_pDirIcon;
		ID3D11ShaderResourceView* m_pFileIcon;
	};
}
#endif // !CONTENTBROWSER_H