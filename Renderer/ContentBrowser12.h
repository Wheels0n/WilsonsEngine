#pragma once

#include<filesystem>
#include<d3d12.h>
#include"../ImGui/imgui.h"
namespace wilson
{	
	class D3D12;
	class DescriptorHeapManager;
	class ContentBrowser12
	{
	public:
		void List();

		ContentBrowser12() = default;
		ContentBrowser12(D3D12*);
		ContentBrowser12(const ContentBrowser12&) = delete;
		~ContentBrowser12();

	private:
		void GetExtension(char*, const char*);
	private:
		std::filesystem::path m_curDir = "Assets";
		//D3D12
		ID3D12Resource* m_pDirIcon12Tex;
		ID3D12Resource* m_pFileIcon12Tex;
		ID3D12Resource* m_pDirIcon12UploadCB;
		ID3D12Resource* m_pFileIcon12UploadCB;
		D3D12_GPU_DESCRIPTOR_HANDLE m_dirIcon12SRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_fileIcon12SRV;
	};
}