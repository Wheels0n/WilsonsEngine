#include "Shader.h"
#include <D3Dcompiler.h>
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

		m_pShadowVS = nullptr;

		m_pOmniDirShadowVS = nullptr;
		m_pOmniDirShadowGS = nullptr;
		m_pOmniDirShadowPS = nullptr;

		m_pTexVS = nullptr;

		m_pGeometryVS = nullptr;
		m_pGeometryPS = nullptr;
		m_pDeferredPS = nullptr;

		m_pLightCubeVS = nullptr;
		m_pLightCubePS = nullptr;

		m_pOutlinerPS = nullptr;
		m_pBlurPS = nullptr;
		m_pFinPS = nullptr;

		m_pInputLayout = nullptr;
		m_pPosOnlyInputLayout = nullptr;
		m_pTexInputLayout = nullptr;
		m_pDeferredGeoLayout = nullptr;
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

		if (m_pShadowVS != nullptr)
		{
			m_pShadowVS->Release();
			m_pShadowVS = nullptr;
		}

		if (m_pOmniDirShadowVS != nullptr)
		{
			m_pOmniDirShadowVS->Release();
			m_pOmniDirShadowVS = nullptr;
		}

		if (m_pOmniDirShadowGS != nullptr)
		{
			m_pOmniDirShadowGS->Release();
			m_pOmniDirShadowGS = nullptr;
		}

		if (m_pOmniDirShadowPS != nullptr)
		{
			m_pOmniDirShadowPS->Release();
			m_pOmniDirShadowPS = nullptr;
		}

		if (m_pTexVS != nullptr)
		{
			m_pTexVS->Release();
			m_pTexVS = nullptr;
		}

		if (m_pGeometryVS != nullptr)
		{
			m_pGeometryVS->Release();
			m_pGeometryVS = nullptr;
		}
		if (m_pGeometryPS != nullptr)
		{
			m_pGeometryPS->Release();
			m_pGeometryPS = nullptr;
		}
		if (m_pDeferredPS != nullptr)
		{
			m_pDeferredPS->Release();
			m_pDeferredPS = nullptr;
		}

		if (m_pLightCubeVS != nullptr)
		{
			m_pLightCubeVS->Release();
			m_pLightCubeVS = nullptr;
		}

		if (m_pLightCubePS != nullptr)
		{
			m_pLightCubePS->Release();
			m_pLightCubePS = nullptr;
		}

		if (m_pOutlinerPS != nullptr)
		{
			m_pOutlinerPS->Release();
			m_pOutlinerPS = nullptr;
		}

		if (m_pBlurPS != nullptr)
		{
			m_pBlurPS->Release();
			m_pBlurPS = nullptr;
		}

		if (m_pFinPS != nullptr)
		{
			m_pFinPS->Release();
			m_pFinPS = nullptr;
		}

		if (m_pInputLayout != nullptr)
		{
			m_pInputLayout->Release();
			m_pInputLayout = nullptr;
		}

		if (m_pPosOnlyInputLayout != nullptr)
		{
			m_pPosOnlyInputLayout->Release();
			m_pPosOnlyInputLayout = nullptr;
		}

		if (m_pTexInputLayout != nullptr)
		{
			m_pTexInputLayout->Release();
			m_pTexInputLayout = nullptr;
		}

		if (m_pDeferredGeoLayout != nullptr)
		{
			m_pDeferredGeoLayout->Release();
			m_pDeferredGeoLayout = nullptr;
		}

	}

	bool Shader::Init()
	{
		HRESULT hr;
		D3D11_INPUT_ELEMENT_DESC vertexIED[8];
		D3D11_INPUT_ELEMENT_DESC posOnlyIED;
		ID3DBlob* pVSBlob=nullptr;
		ID3DBlob* pGSBlob=nullptr;
		ID3DBlob* pPSBlob=nullptr;
		ID3DBlob* pErrorBlob=nullptr;

		hr = D3DX11CompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVS);


		hr = D3DX11CompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_DEBUG |D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION| D3D10_SHADER_SKIP_OPTIMIZATION, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPS);
	
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
		
		vertexIED[3].SemanticName  = "TANGENT";
		vertexIED[3].SemanticIndex = 0;
		vertexIED[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexIED[3].InputSlot = 0;
		vertexIED[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexIED[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vertexIED[3].InstanceDataStepRate = 0;

		vertexIED[4] = { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,
								D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		vertexIED[5] = { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,
								D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		vertexIED[6] = { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32,
								D3D11_INPUT_PER_INSTANCE_DATA, 1 };
		vertexIED[7] = { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48,
								D3D11_INPUT_PER_INSTANCE_DATA, 1 };

		m_pDevice->CreateInputLayout(vertexIED, sizeof(vertexIED) / sizeof(vertexIED[0]), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout);
		

		hr = D3DX11CompileFromFile(L"SkyBoxVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pSkyBoxVS);

		hr = D3DX11CompileFromFile(L"SkyBoxPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pSkyBoxPS);

		posOnlyIED = vertexIED[0];
		m_pDevice->CreateInputLayout(&posOnlyIED, 1, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pPosOnlyInputLayout);

		hr = D3DX11CompileFromFile(L"ShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pShadowVS);


		hr = D3DX11CompileFromFile(L"OmniDirShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pOmniDirShadowVS);

		hr = D3DX11CompileFromFile(L"OmniDirShadowGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pGSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_pOmniDirShadowGS);


		hr = D3DX11CompileFromFile(L"OmniDirShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pOmniDirShadowPS);

		hr = D3DX11CompileFromFile(L"TexVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pTexVS);
		m_pDevice->CreateInputLayout(vertexIED, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pTexInputLayout);


		hr = D3DX11CompileFromFile(L"GeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pGeometryVS);
		m_pDevice->CreateInputLayout(vertexIED, 4, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),&m_pDeferredGeoLayout);

		hr = D3DX11CompileFromFile(L"GeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pGeometryPS);
	

		hr = D3DX11CompileFromFile(L"DeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0",  D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pDeferredPS);


		hr = D3DX11CompileFromFile(L"LightCubeVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pLightCubeVS);

		hr = D3DX11CompileFromFile(L"LightCubePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pLightCubePS);

		hr = D3DX11CompileFromFile(L"OutlinerPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pOutlinerPS);

		hr = D3DX11CompileFromFile(L"BlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pBlurPS);

		hr = D3DX11CompileFromFile(L"FinPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS  | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pFinPS);


		pPSBlob->Release();
		pPSBlob = nullptr;

		pGSBlob->Release();
		pGSBlob = nullptr;

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