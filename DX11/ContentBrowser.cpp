#include "ContentBrowser.h"

namespace wilson {
	const static std::filesystem::path assetsPath = "Assets";
	const static float iconSz = 128.0f;
	const static float padding = 16.0f;

	void ContentBrowser::Init(ID3D11Device* pDevice)
	{
		D3DX11CreateShaderResourceViewFromFileW(pDevice, L"./Assets/Icons/folderIcon.png", nullptr, nullptr, &m_pDirIcon, nullptr);
		D3DX11CreateShaderResourceViewFromFileW(pDevice, L"./Assets/Icons/fileIcon.png", nullptr, nullptr, &m_pFileIcon, nullptr);
	}

	void ContentBrowser::List()
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

		float panelWidth = ImGui::GetContentRegionAvail().x;
		unsigned int colCount = (int)(panelWidth / (iconSz + padding));
		if (colCount < 1)
		{
			colCount = 1;
		}
		ImGui::Columns(colCount, 0, false);

		int id = 0;
		for (auto const& item : std::filesystem::directory_iterator(m_curDir))
		{
			ImGui::PushID(++id);//이미지가 다 같아서 구분 할 기준이 필요함

			std::string fileName = item.path().filename().string();
			void* icon = item.is_directory() ? m_pDirIcon : m_pFileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(icon, ImVec2(iconSz, iconSz));

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				char ext[4];
				const wchar_t* fileName_t = item.path().c_str(); //value_type	character type used by the native encoding of the filesystem: char on POSIX, wchar_t on Windows
				GetExtension(ext, fileName.c_str());

				ImGui::SetDragDropPayload(ext, fileName_t, (wcslen(fileName_t) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::Text("%s", fileName.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (item.is_directory())
				{
					m_curDir /= item.path().filename();
				}
			}
			ImGui::TextWrapped(fileName.c_str());
			ImGui::NextColumn();

			ImGui::PopID();
		}
		ImGui::Columns(1);

		ImGui::End();
	}

	void ContentBrowser::GetExtension(char* dst, const char* src)
	{
		int i = 0;
		while (true)
		{
			++i;
			if (src[i] == '.')
			{
				++i;
				break;
			}
		}

		for (int j = 0; j < 3; ++j, ++i)
		{
			dst[j] = src[i];
		}
		dst[3] = '\0';
		return;
	}
}