#include <DirectXTex.h>
#include "ContentBrowser11.h"

namespace wilson {
	const static std::filesystem::path assetsPath = "Assets";
	const static float _ICON_SZ = 128.0f;
	const static float _PAD = 16.0f;

	ContentBrowser11::~ContentBrowser11()
	{
	}

	ContentBrowser11::ContentBrowser11(ID3D11Device*const pDevice)
	{	

		HRESULT hr;
		DirectX::ScratchImage image;
		hr = DirectX::LoadFromWICFile(L"./Assets/Icons/folderIcon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
		assert(SUCCEEDED(hr));
		hr = CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), m_pDirIconSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pDirIconSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("ContentBrowser11::m_pDirIconSrv") - 1, "ContentBrowser11::m_pDirIconSrv");

		hr = DirectX::LoadFromWICFile(L"./Assets/Icons/fileIcon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
		assert(SUCCEEDED(hr));
		hr = CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), m_pFileIconSrv.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pFileIconSrv->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("ContentBrowser11::m_pFileIconSrv") - 1, "ContentBrowser11::m_pFileIconSrv");
	}


	void ContentBrowser11::List()
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
		unsigned int colCount = static_cast<UINT>(panelWidth / (_ICON_SZ + _PAD));
		if (colCount < 1)
		{
			colCount = 1;
		}
		ImGui::Columns(colCount, 0, false);

		int id = 0;
		for (auto const& item : std::filesystem::directory_iterator(m_pCurDir))
		{
			ImGui::PushID(++id);//이미지가 다 같아서 구분 할 기준이 필요함

			std::string fileName = item.path().filename().string();
			void* pIcon11 = item.is_directory() ? m_pDirIconSrv.Get() : m_pFileIconSrv.Get();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(pIcon11, ImVec2(_ICON_SZ, _ICON_SZ));
			
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

	void ContentBrowser11::GetExtension(char* dst, const char* src)
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