#include "CubeMap.h"
#include <vector>
#include<filesystem>
bool Cubemap::Init(ID3D11Device* pDevice)
{	
	std::vector<DirectX::ScratchImage> scratch(6);
	HRESULT hr;
	int cnt = 0;
	for (auto const& dir_endtry : std::filesystem::directory_iterator("./Assets/Textures/skybox"))
	{
		hr = DirectX::LoadFromWICFile(L"./Assets/Textures/skybox/bottom.jpg", DirectX::WIC_FLAGS_NONE, nullptr, scratch[cnt++]);
		if (FAILED(hr))
		{
			return false;
		}

	}
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = scratch[0].GetMetadata().width;
	texDesc.Height = scratch[0].GetMetadata().height;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	SRVDesc.Format = texDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRVDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SRVDesc.TextureCube.MostDetailedMip = 0;

	D3D11_SUBRESOURCE_DATA data[6];
	for (int i = 0; i < 6; ++i)
	{
		data[i].pSysMem = scratch[i].GetPixels();
		data[i].SysMemPitch = scratch[i].GetImage(0, 0, 0)->rowPitch;
		data[i].SysMemSlicePitch = 0;
	}

	hr = pDevice->CreateTexture2D(&texDesc, data, &m_pTexture);
	if (FAILED(hr))
	{
		return false;
	}

	hr = pDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pSRV);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}
