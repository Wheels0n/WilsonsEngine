#pragma once
#include <DirectXTex.h>
#include <d3d11.h>

class Cubemap
{

public: 
	bool Init(ID3D11Device* pDevice);
	void ShutDown();

private:
	ID3D11RenderTargetView* m_cubeMapRTVs[6];
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pSRV;
};