#include "Shader.h"

CShader::CShader(ID3D11Device* device, ID3D11DeviceContext* context)
{   
	m_pDevice = device;
	m_pContext = context;
    m_pVertexShader = nullptr;
    m_pPixelShader = nullptr;
    m_pInputLayout = nullptr;
}

CShader::~CShader()
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

bool CShader::Init()
{   
	HRESULT hr;
    D3D11_INPUT_ELEMENT_DESC vertexDesc[3];
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

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].SemanticIndex = 0;
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].InputSlot = 0;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexDesc[0].InstanceDataStepRate = 0;

	vertexDesc[1].SemanticName = "TEXTURE";
	vertexDesc[1].SemanticIndex = 0;
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].InputSlot = 0;
	vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexDesc[1].InstanceDataStepRate = 0;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].SemanticIndex = 0;
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].InputSlot = 0;
	vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexDesc[2].InstanceDataStepRate = 0;
	m_pDevice->CreateInputLayout(vertexDesc, sizeof(vertexDesc) / sizeof(vertexDesc[0]), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &m_pInputLayout);
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
