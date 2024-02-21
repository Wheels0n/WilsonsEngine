#pragma once

#include <DirectXTex.h>
#include<filesystem>
#include<d3d11.h>
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
		virtual ~ContentBrowser();

	private:
		void GetExtension(char*, const char*);
	private:
		std::filesystem::path m_curDir = "Assets";
		ID3D11ShaderResourceView* m_pDirIconSRV;
		ID3D11ShaderResourceView* m_pFileIconSRV;
	};
}