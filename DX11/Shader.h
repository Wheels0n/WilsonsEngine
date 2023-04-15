#ifndef SHADER_H
#define SHADER_H

#include<d3d11.h>
#include<D3DX11tex.h>

namespace wilson
{
	class Shader
	{
	public:
		bool Init();
		inline void SetInputLayout()
		{
			m_pContext->IASetInputLayout(m_pInputLayout);
		}
		inline void SetIndexedInputLayout()
		{
			m_pContext->IASetInputLayout(m_pInstancedInputLayout);
		}

		Shader(ID3D11Device* pDevice, ID3D11DeviceContext* context);
		~Shader();

	private:

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pContext;

		ID3D11VertexShader* m_pVertexShader;
		ID3D11PixelShader* m_pPixelShader;
		ID3D11InputLayout* m_pInputLayout;
		ID3D11InputLayout* m_pInstancedInputLayout;
	};
}
#endif
