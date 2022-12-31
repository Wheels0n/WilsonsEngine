#ifndef SHADER_H
#define SHADER_H

#include<d3d11.h>
#include<D3DX11tex.h>

class CShader
{
public:
	CShader(ID3D11Device* device, ID3D11DeviceContext* context);
	~CShader();
	bool Init();
private:

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;

	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11InputLayout* m_pInputLayout;
};

#endif
