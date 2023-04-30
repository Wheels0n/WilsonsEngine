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
		inline void SetSkyBoxInputLayout()
		{
			m_pContext->IASetInputLayout(m_pSkyBoxInputLayout);
		}
		inline void SetShader()
		{
			m_pContext->VSSetShader(m_pVS, nullptr, 0);
			m_pContext->PSSetShader(m_pPS, nullptr, 0);
		}
		inline void SetSkyBoxShader()
		{
			m_pContext->VSSetShader(m_pSkyBoxVS, nullptr, 0);
			m_pContext->PSSetShader(m_pSkyBoxPS, nullptr, 0);
		}
		

		Shader(ID3D11Device* pDevice, ID3D11DeviceContext* context);
		~Shader();

	private:

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pContext;

		ID3D11VertexShader* m_pVS, *m_pSkyBoxVS;
		ID3D11PixelShader* m_pPS, *m_pSkyBoxPS;


		ID3D11InputLayout* m_pInputLayout, *m_pSkyBoxInputLayout;

	};
}
#endif
