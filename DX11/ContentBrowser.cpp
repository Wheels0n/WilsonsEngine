#include "ContentBrowser.h"

const static std::filesystem::path assetsPath = "Assets";

void CContentBrowser::List()
{   
	ImGui::Begin("Content Browser");
    ImGui::Text("%s", m_curDir.string().c_str());

	if (m_curDir != std::filesystem::path(assetsPath))
	{
		if (ImGui::Button("<-"))
		{
			m_curDir = m_curDir.parent_path();
		}
	}

	for (auto const& item : std::filesystem::directory_iterator(m_curDir))
	{   
		std::string fileName = item.path().filename().string();
		if (item.is_directory())
		{
			if (ImGui::Button(fileName.c_str())) //When it's clicked
			{
				m_curDir /= item.path().filename();
			}
		}
		else
		{
			if (ImGui::Button(fileName.c_str()))
			{
				
			}
		}
	}

	ImGui::End();
}
