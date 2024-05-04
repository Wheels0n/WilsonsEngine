#pragma once

#include <d3d12.h>
#include "typedef.h"
#include "../ImGui/imgui.h"
namespace wilson
{	
	class D3D12;
	class HeapManager;
	class ContentBrowser12
	{
	public:
		void List();

		ContentBrowser12() = default;
		ContentBrowser12(D3D12*const);
		ContentBrowser12(const ContentBrowser12&) = delete;
		~ContentBrowser12();

	private:
		void GetExtension(char*, const char*);
	private:
		std::filesystem::path m_pCurDir = "Assets";
		//D3D12
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pDirIconTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pDirIconUploadCb;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pFileIconTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pFileIconUploadCb;
		D3D12_GPU_DESCRIPTOR_HANDLE m_dirIconSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_fileIcon12Srv;
	};
}