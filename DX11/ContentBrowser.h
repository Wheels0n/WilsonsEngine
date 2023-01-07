#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include"../ImGui/imgui.h"
#include<filesystem>

class CContentBrowser
{
public:
	CContentBrowser() = default;
	CContentBrowser(const CContentBrowser&) = delete;
	~CContentBrowser() = default;

	void List();
private:
	std::filesystem::path m_curDir = "Assets";
};
#endif // !CONTENTBROWSER_H