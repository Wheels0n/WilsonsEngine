#include "Shader.h"

namespace wilson
{
	Shader::Shader(ID3D11Device* pDevice, ID3D11DeviceContext* context)
	{
		m_pDevice = pDevice;
		m_pContext = context;
		m_pVertexShader = nullptr;
		m_pPixelShader = nullptr;
		m_pInputLayout = nullptr;
	}

	Shader::~Shader()
	{
		if (m_pVertexShader != nullptr)
		{
			m_pVertexShader->Release();
			m_pVertexShader = nullptr;
		}

		if (m_pPixelShader != nullptr)
		{
			m_pPixelShader->Release();
			m_pPixelShader = nullptr;
		}

		if (m_pInputLayout != nullptr)
		{
			m_pInputLayout->Release();
			m_pInputLayout = nullptr;
		}
	}

	bool Shader::Init()
	{
		HRESULT hr;
		D3D11_INPUT_ELEMENT_DESC vertexIED[3];
		ID3DBlob* pVsBlob;
		ID3DBlob* pPsBlob;
		ID3DBlob* pErrorBlob;

		hr = D3DX11CompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVsBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &m_pVertexShader);
		m_pContext->VSSetShader(m_pVertexShader, nullptr, 0);


		hr = D3DX11CompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPsBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &m_pPixelShader);
		m_pContext->PSSetShader(m_pPixelShader, nullptr, 0);

		vertexIED[0].SemanticName = "POSITION";
		vertexIED[0].SemanticIndex = 0;
		vertexIED[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexIED[0].InputSlot = 0;
		vertexIED[0].AlignedByteOffset = 0;
		vertexIED[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vertexIED[0].InstanceDataStepRate = 0;

		vertexIED[1].SemanticName = "TEXTURE";
		vertexIED[1].SemanticIndex = 0;
		vertexIED[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexIED[1].InputSlot = 0;
		vertexIED[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexIED[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vertexIED[1].InstanceDataStepRate = 0;

		vertexIED[2].SemanticName = "NORMAL";
		vertexIED[2].SemanticIndex = 0;
		vertexIED[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexIED[2].InputSlot = 0;
		vertexIED[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexIED[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vertexIED[2].InstanceDataStepRate = 0;
		m_pDevice->CreateInputLayout(vertexIED, sizeof(vertexIED) / sizeof(vertexIED[0]), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &m_pInputLayout);
		m_pContext->IASetInputLayout(m_pInputLayout);


		pPsBlob->Release();
		pPsBlob = nullptr;

		pVsBlob->Release();
		pVsBlob = nullptr;

		if (pErrorBlob != nullptr)
		{
			pErrorBlob->Release();
			pErrorBlob = nullptr;
		}

		return true;
	}
}