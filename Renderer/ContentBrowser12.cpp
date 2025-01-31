#include "CommandListWrapperPool.h"
#include "D3D12.h"
#include "HeapManager.h"
#include "ContentBrowser12.h"

namespace wilson 
{
	using namespace DirectX;
	
	const static filesystem::path assetsPath = "Assets";
	const static float _ICON_SZ = 128.0f;
	const static float _PAD = 16.0f;

	ContentBrowser12::ContentBrowser12(shared_ptr<D3D12> pD3D12)
	{
		//D3D12

		shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();
		shared_ptr<HeapManager> pHeapManager = pD3D12->GetHeapManager();

		HRESULT hr;
		//Gen DirIcon
		{
			//Gen IconTexFromFile 
			m_pDirIconTex = g_pTextureManager->LoadImageWICFile(L"./Assets/Icons/folderIcon.png", pCmdList);	
			SET_PRIVATE_DATA(m_pDirIconTex->tex);
		}

		//Gen FileIcon
		{
			//Gen IconTexFromFile 
			m_pFileIconTex = g_pTextureManager->LoadImageWICFile(L"./Assets/Icons/fileIcon.png", pCmdList);
			SET_PRIVATE_DATA(m_pFileIconTex->tex);
		}
		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);
	}

	void ContentBrowser12::List()
	{
		ImGui::Begin("Content Browser");
		ImGui::Text("%s", m_pCurDir.string().c_str());

		if (m_pCurDir != std::filesystem::path(assetsPath))
		{
			if (ImGui::Button("<-"))
			{
				m_pCurDir = m_pCurDir.parent_path();
			}
		}

		float panelWidth = ImGui::GetContentRegionAvail().x;
		unsigned int colCount = E_TO_UINT(panelWidth / (_ICON_SZ + _PAD));
		if (colCount < 1)
		{
			colCount = 1;
		}
		ImGui::Columns(colCount, 0, false);

		int id = 0;
		for (auto const& item : filesystem::directory_iterator(m_pCurDir))
		{
			ImGui::PushID(++id);//이미지가 다 같아서 구분 할 기준이 필요함

			string fileName = item.path().filename().string();
			UINT64 icon12 = item.is_directory() ? m_pDirIconTex->srv.ptr : m_pFileIconTex->srv.ptr;
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)icon12, ImVec2(_ICON_SZ, _ICON_SZ));

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				char ext[4];
				const wchar_t* fileName_t = item.path().c_str(); //value_type	character type used by the native encoding of the filesystem: char on POSIX, wchar_t on Windows
				ParseExtension(ext, fileName.c_str());

				ImGui::SetDragDropPayload(ext, fileName_t, (wcslen(fileName_t) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::Text("%s", fileName.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (item.is_directory())
				{
					m_pCurDir /= item.path().filename();
				}
			}
			ImGui::TextWrapped(fileName.c_str());
			ImGui::NextColumn();

			ImGui::PopID();
		}
		ImGui::Columns(1);

		ImGui::End();
	}

	void ContentBrowser12::ParseExtension(char* dst, const char* src)
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