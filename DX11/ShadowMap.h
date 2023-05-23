#pragma once
#include <d3d11.h>

class ShadowMap
{
public:
	bool Init(ID3D11Device* pDevice, int width, int height);

	void BindDSV(ID3D11DeviceContext* pContext);
	inline ID3D11ShaderResourceView** GetSRV()
	{
		return &m_SRV;
	}
	inline ID3D11SamplerState** GetShadowSampler()
	{
		return &m_pSamplerState;
	}
	inline D3D11_VIEWPORT* GetViewport()
	{
		return &m_viewport;
	}
	ShadowMap() = default;
	~ShadowMap();
private:
	void SetNullRenderTarget(ID3D11DeviceContext* pContext);
private:
	ID3D11Texture2D* m_depthMap;
	ID3D11ShaderResourceView* m_SRV;
	ID3D11DepthStencilView* m_DSV;
	ID3D11SamplerState* m_pSamplerState;
	D3D11_VIEWPORT m_viewport;
};