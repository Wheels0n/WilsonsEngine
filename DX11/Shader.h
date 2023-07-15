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
		inline void SetPosOnlyInputLayout()
		{
			m_pContext->IASetInputLayout(m_pPosOnlyInputLayout);
		}
		inline void SetShader()
		{
			m_pContext->VSSetShader(m_pVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pPS, nullptr, 0);
		}
		inline void SetSkyBoxShader()
		{
			m_pContext->VSSetShader(m_pSkyBoxVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(m_pSkyBoxPS, nullptr, 0);
		}
		inline void SetShadowShader()
		{
			m_pContext->VSSetShader(m_pShadowVS, nullptr, 0);
			m_pContext->GSSetShader(nullptr, nullptr, 0);
			m_pContext->PSSetShader(nullptr, nullptr, 0);
		}
		inline void SetOmniDirShadowShader()
		{
			m_pContext->VSSetShader(m_pOmniDirShadowVS, nullptr, 0);
			m_pContext->GSSetShader(m_pOmniDirShadowGS, nullptr, 0);
			m_pContext->PSSetShader(m_pOmniDirShadowPS, nullptr, 0);
		}

		Shader(ID3D11Device* pDevice, ID3D11DeviceContext* context);
		~Shader();

	private:

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pContext;

		ID3D11VertexShader* m_pVS, *m_pSkyBoxVS, *m_pShadowVS, *m_pOmniDirShadowVS;
		ID3D11GeometryShader* m_pOmniDirShadowGS;
		ID3D11PixelShader* m_pPS, *m_pSkyBoxPS, *m_pOmniDirShadowPS;


		ID3D11InputLayout* m_pInputLayout, *m_pPosOnlyInputLayout;

	};
}
#endif
