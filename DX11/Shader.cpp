#include "Shader.h"

namespace wilson
{
	Shader::Shader(ID3D11Device* pDevice, ID3D11DeviceContext* context)
	{
		m_pDevice = pDevice;
		m_pContext = context;
		m_pVS = nullptr;
		m_pPS = nullptr;
		m_pSkyBoxVS = nullptr;
		m_pSkyBoxPS = nullptr;
		m_pInputLayout = nullptr;
		m_pSkyBoxInputLayout = nullptr;
	}

	Shader::~Shader()
	{
		if (m_pVS != nullptr)
		{
			m_pVS->Release();
			m_pVS = nullptr;
		}

		if (m_pPS != nullptr)
		{
			m_pPS->Release();
			m_pPS = nullptr;
		}

		if (m_pSkyBoxVS != nullptr)
		{
			m_pSkyBoxVS->Release();
			m_pSkyBoxVS = nullptr;
		}

		if (m_pSkyBoxPS != nullptr)
		{
			m_pSkyBoxPS->Release();
			m_pSkyBoxPS = nullptr;
		}

		if (m_pInputLayout != nullptr)
		{
			m_pInputLayout->Release();
			m_pInputLayout = nullptr;
		}

		if (m_pSkyBoxInputLayout != nullptr)
		{
			m_pSkyBoxInputLayout->Release();
			m_pSkyBoxInputLayout = nullptr;
		}
	}

	bool Shader::Init()
	{
		HRESULT hr;
		D3D11_INPUT_ELEMENT_DESC vertexIED[7];
		D3D11_INPUT_ELEMENT_DESC skyBoxIED;
		ID3DBlob* pVSBlob;
		ID3DBlob* pPSBlob;
		ID3DBlob* pErrorBlob;

		hr = D3DX11CompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVS);


		hr = D3DX11CompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPS);
		
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
	
		vertexIED[3] = { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,
								D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		vertexIED[4] = { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,
								D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		vertexIED[5] = { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32,
								D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		vertexIED[6] = { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48,
								D3D11_INPUT_PER_INSTANCE_DATA, 1 };

		m_pDevice->CreateInputLayout(vertexIED, sizeof(vertexIED) / sizeof(vertexIED[0]), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout);


		hr = D3DX11CompileFromFile(L"SkyBoxVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pSkyBoxVS);

		hr = D3DX11CompileFromFile(L"SkyBoxPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pSkyBoxPS);

		skyBoxIED = vertexIED[0];

		m_pDevice->CreateInputLayout(&skyBoxIED, 1, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pSkyBoxInputLayout);

		pPSBlob->Release();
		pPSBlob = nullptr;

		pVSBlob->Release();
		pVSBlob = nullptr;

		if (pErrorBlob != nullptr)
		{
			pErrorBlob->Release();
			pErrorBlob = nullptr;
		}

		return true;
	}
}