#pragma	once
#include <DirectXTex.h>
#include "ContentBrowser12.h"
#include "D3D12.h"
#include "HeapManager.h"
namespace wilson {
	const static std::filesystem::path assetsPath = "Assets";
	const static float _ICON_SZ = 128.0f;
	const static float _PAD = 16.0f;

	ContentBrowser12::~ContentBrowser12()
	{

		if (m_pDirIconTex != nullptr)
		{
			m_pDirIconTex->Release();
			m_pDirIconTex = nullptr;
		}

		if (m_pFileIconTex != nullptr)
		{
			m_pFileIconTex->Release();
			m_pFileIconTex = nullptr;
		}

		if (m_pDirIconUploadCb != nullptr)
		{
			m_pDirIconUploadCb->Release();
			m_pDirIconUploadCb = nullptr;
		}

		if (m_pFileIconUploadCB != nullptr)
		{
			m_pFileIconUploadCB->Release();
			m_pFileIconUploadCB = nullptr;
		}

	}

	ContentBrowser12::ContentBrowser12(D3D12*const pD3D12)
	{
		//D3D12
		m_pDirIconTex = nullptr;
		m_pFileIconTex = nullptr;
		m_pDirIconUploadCb = nullptr;
		m_pFileIconUploadCB = nullptr;

		ID3D12Device* pDevice = pD3D12->GetDevice();
		ID3D12GraphicsCommandList* pCommandlist = pD3D12->GetCommandList();
		HeapManager* pHeapManager = pD3D12->GetHeapManager();

		HRESULT hr;
		//Gen DirIcon
		{
			//Gen IconTexFromFile 
			DirectX::ScratchImage image;
			hr = DirectX::LoadFromWICFile(L"./Assets/Icons/folderIcon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
			assert(SUCCEEDED(hr));
			UINT8* pData = image.GetPixels();
			size_t rowPitch;
			size_t slidePitch;
			DirectX::TexMetadata metadata = image.GetMetadata();
			ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slidePitch);

			D3D12_RESOURCE_DESC	texDesc = {};
			texDesc.Width = image.GetMetadata().width;
			texDesc.Height = image.GetMetadata().height;
			texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			texDesc.Alignment = 0;
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Format = image.GetMetadata().format;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;

			pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
				&m_pDirIconTex, pDevice);
			m_pDirIconTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ContentBrowser12::m_pDirIconTex") - 1, "ContentBrowser12::m_pDirIconTex");
			
			pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, m_pDirIconTex, &m_pDirIconUploadCb, pCommandlist);

			//Gen SRV
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			m_dirIcon12SRV = pHeapManager->GetSrv(srvDesc, m_pDirIconTex, pDevice);
			
		}


		//Gen FileIcon
		{
			//Gen IconTexFromFile 
			DirectX::ScratchImage image;
			hr = DirectX::LoadFromWICFile(L"./Assets/Icons/fileIcon.png", DirectX::WIC_FLAGS_NONE, nullptr, image);
			assert(SUCCEEDED(hr));
			UINT8* pData = image.GetPixels();
			
			size_t rowPitch;
			size_t slidePitch;
			DirectX::TexMetadata metadata = image.GetMetadata();
			ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slidePitch);
			

			D3D12_RESOURCE_DESC	texDesc = {};
			texDesc.Width = image.GetMetadata().width;
			texDesc.Height = image.GetMetadata().height;
			texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			texDesc.Alignment = 0;
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Format = image.GetMetadata().format;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;

			pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
				&m_pFileIconTex, pDevice);
			m_pFileIconTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("ContentBrowser12::m_pFileIconTex") - 1, "ContentBrowser12::m_pFileIconTex");

			pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, m_pFileIconTex, &m_pFileIconUploadCB, pCommandlist);

			//Gen SRV
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			m_fileIcon12SRV = pHeapManager->GetSrv(srvDesc, m_pFileIconTex, pDevice);
		}


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
			UINT64 icon12 = item.is_directory() ? m_dirIcon12SRV.ptr : m_fileIcon12SRV.ptr;
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)icon12, ImVec2(_ICON_SZ, _ICON_SZ));

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

	void ContentBrowser12::GetExtension(char* dst, const char* src)
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