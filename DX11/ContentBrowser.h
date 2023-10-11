#pragma once

#include<D3DX11tex.h>
#include<filesystem>

#include"../ImGui/imgui.h"

namespace wilson 
{
	class ContentBrowser
	{
	public:
		void List();

		ContentBrowser() = default;
		ContentBrowser(ID3D11Device*);
		ContentBrowser(const ContentBrowser&) = delete;
		~ContentBrowser();

	private:
		void GetExtension(char*, const char*);
	private:
		std::filesystem::path m_curDir = "Assets";
		ID3D11ShaderResourceView* m_pDirIcon;
		ID3D11ShaderResourceView* m_pFileIcon;
	};
}