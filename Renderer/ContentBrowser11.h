#pragma once

#include<d3d11.h>
#include"typedef.h"
#include"../ImGui/imgui.h"
namespace wilson 
{
	class ContentBrowser11
	{
	public:
		void List();

		ContentBrowser11() = default;
		ContentBrowser11(ID3D11Device*const);
		ContentBrowser11(const ContentBrowser11&) = delete;
		virtual ~ContentBrowser11();

	private:
		void GetExtension(char*, const char*);
	private:
		std::filesystem::path m_pCurDir = "Assets";
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pDirIconSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pFileIconSrv;
	};
}