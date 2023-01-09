#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include"../ImGui/imgui.h"
#include<D3DX11tex.h>
#include<filesystem>

class CContentBrowser
{
public:
	CContentBrowser() = default;
	CContentBrowser(const CContentBrowser&) = delete;
	~CContentBrowser() = default;

	void Init(ID3D11Device*);
	void List();
private:
	void getExtension(char*, const char*);

	std::filesystem::path m_curDir = "Assets";
	ID3D11ShaderResourceView* m_pDirIcon;
	ID3D11ShaderResourceView* m_pFileIcon;
};
#endif // !CONTENTBROWSER_H