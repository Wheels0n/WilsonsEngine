#include "Shader.h"
#include <D3Dcompiler.h>

namespace wilson
{
	Shader::Shader()
	{
		m_pVS = nullptr;
		m_pPS = nullptr;

		m_pPosOnlyVS = nullptr;
		m_pCascadeDirVS = nullptr;
		m_pCascadeDirGS = nullptr;
		m_pCascadeDirPS = nullptr;
		m_pEquirect2CubeGS = nullptr;
		m_pEquirect2CubePS = nullptr;
		m_pDiffuseIrradiancePS = nullptr;
		m_pPrefilterPS = nullptr; 
		m_pBRDFPS = nullptr;

		m_pSkyBoxVS = nullptr;
		m_pSkyBoxPS = nullptr;

		m_pShadowVS = nullptr;
		m_pShadowPS = nullptr;

		m_pOmniDirShadowVS = nullptr;
		m_pOmniDirShadowGS = nullptr;
		m_pOmniDirShadowPS = nullptr;

		m_pTexVS = nullptr;

		m_pGeometryVS = nullptr;
		m_pGeometryPS = nullptr;
		m_pSSAOPS = nullptr;
		m_pSSAOBlurPS = nullptr;
		m_pDeferredPS = nullptr;

		m_pPBRGeometryVS = nullptr;
		m_pPBRGeometryPS = nullptr;
		m_pPBRDeferredPS = nullptr;

		m_pLightCubeVS = nullptr;
		m_pLightCubePS = nullptr;

		m_pAABBVS = nullptr;
		m_pAABBPS = nullptr;

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
		if (m_pShadowPS != nullptr)
		{
			m_pShadowPS->Release();
			m_pShadowPS = nullptr;
		}



		if (m_pPosOnlyVS != nullptr)
		{
			m_pPosOnlyVS->Release();
			m_pPosOnlyVS = nullptr;
		}
		if (m_pEquirect2CubeGS != nullptr)
		{
			m_pEquirect2CubeGS->Release();
			m_pEquirect2CubeGS = nullptr;
		}
		if (m_pEquirect2CubePS != nullptr)
		{
			m_pEquirect2CubePS->Release();
			m_pEquirect2CubePS = nullptr;
		}
		if (m_pDiffuseIrradiancePS != nullptr)
		{
			m_pDiffuseIrradiancePS->Release();
			m_pDiffuseIrradiancePS = nullptr;
		}
		if (m_pPrefilterPS != nullptr)
		{
			m_pPrefilterPS->Release();
			m_pPrefilterPS = nullptr;
		}
		if (m_pBRDFPS != nullptr)
		{
			m_pBRDFPS->Release();
			m_pBRDFPS = nullptr;
		}
		if (m_pCascadeDirVS != nullptr)
		{
			m_pCascadeDirVS->Release();
			m_pCascadeDirVS = nullptr;
		}

		if (m_pCascadeDirGS != nullptr)
		{
			m_pCascadeDirGS->Release();
			m_pCascadeDirGS = nullptr;
		}

		if (m_pCascadeDirPS != nullptr)
		{
			m_pCascadeDirPS->Release();
			m_pCascadeDirPS = nullptr;
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
		if (m_pSSAOPS != nullptr)
		{
			m_pSSAOPS->Release();
			m_pSSAOPS = nullptr;
		}
		if (m_pSSAOBlurPS != nullptr)
		{
			m_pSSAOBlurPS->Release();
			m_pSSAOBlurPS = nullptr;
		}

		if (m_pDeferredPS != nullptr)
		{
			m_pDeferredPS->Release();
			m_pDeferredPS = nullptr;
		}

		if (m_pPBRGeometryVS != nullptr)
		{
			m_pPBRGeometryVS->Release();
			m_pPBRGeometryVS = nullptr;
		}
		if (m_pPBRGeometryPS != nullptr)
		{
			m_pPBRGeometryPS->Release();
			m_pPBRGeometryPS = nullptr;
		}
		if (m_pPBRDeferredPS != nullptr)
		{
			m_pPBRDeferredPS->Release();
			m_pPBRDeferredPS = nullptr;
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
		if (m_pAABBVS != nullptr)
		{
			m_pAABBVS->Release();
			m_pAABBVS = nullptr;
		}
		if (m_pAABBPS != nullptr)
		{
			m_pAABBPS->Release();
			m_pAABBPS = nullptr;
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

	bool Shader::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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
		hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pVS") - 1, "Shader::m_pVS");

		hr = D3DX11CompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_DEBUG |D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION| D3D10_SHADER_SKIP_OPTIMIZATION, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr= pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pPS") - 1, "Shader::m_pPS");

	
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

		hr=pDevice->CreateInputLayout(vertexIED, sizeof(vertexIED) / sizeof(vertexIED[0]), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout);
		if (FAILED(hr))
		{
			return false;
		}
		m_pInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pInputLayout") - 1, "Shader::m_pInputLayout");

		hr = D3DX11CompileFromFile(L"PosOnlyVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pPosOnlyVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pPosOnlyVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pPosOnlyVS") - 1, "Shader::m_pPosOnlyVS");


		hr = D3DX11CompileFromFile(L"CascadeVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pCascadeDirVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pCascadeDirVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pCascadeDirVS") - 1, "Shader::m_pCascadeDirVS");


		
		hr = D3DX11CompileFromFile(L"CascadeGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pGSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_pCascadeDirGS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pCascadeDirGS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pCascadeDirGS") - 1, "Shader::m_pCascadeDirGS");

		hr = D3DX11CompileFromFile(L"CascadePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pCascadeDirPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pCascadeDirPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pCascadeDirPS") - 1, "Shader::m_pCascadeDirPS");





		hr = D3DX11CompileFromFile(L"Equirectangular2CubeMapGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pGSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_pEquirect2CubeGS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pEquirect2CubeGS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pEquirect2CubeGS") - 1, "Shader::m_pEquirect2CubeGS");


		hr = D3DX11CompileFromFile(L"Equirectangular2CubeMapPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr= pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pEquirect2CubePS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pEquirect2CubePS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pEquirect2CubePS") - 1, "Shader::m_pEquirect2CubePS");

		hr = D3DX11CompileFromFile(L"DiffuseIrradiancePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pDiffuseIrradiancePS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDiffuseIrradiancePS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pDiffuseIrradiancePS") - 1, "Shader::m_pDiffuseIrradiancePS");

		hr = D3DX11CompileFromFile(L"PrefilterPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr =pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPrefilterPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pPrefilterPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pPrefilterPS") - 1, "Shader::m_pPrefilterPS");


		hr = D3DX11CompileFromFile(L"BRDFPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pBRDFPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pBRDFPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pBRDFPS") - 1, "Shader::m_pBRDFPS");





		hr = D3DX11CompileFromFile(L"SkyBoxVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pSkyBoxVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pSkyBoxVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pSkyBoxVS") - 1, "Shader::m_pSkyBoxVS");

		hr = D3DX11CompileFromFile(L"SkyBoxPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pSkyBoxPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pSkyBoxPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pSkyBoxPS") - 1, "Shader::m_pSkyBoxPS");


		posOnlyIED = vertexIED[0];
		hr=pDevice->CreateInputLayout(&posOnlyIED, 1, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pPosOnlyInputLayout);
		if (FAILED(hr))
		{
			return false;
		}
		m_pPosOnlyInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pPosOnlyInputLayout") - 1, "Shader::m_pPosOnlyInputLayout");



		hr = D3DX11CompileFromFile(L"ShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pShadowVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pShadowVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pShadowVS") - 1, "Shader::m_pShadowVS");

		hr = D3DX11CompileFromFile(L"ShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pShadowPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pShadowPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pShadowPS") - 1, "Shader::m_pShadowPS");



		hr = D3DX11CompileFromFile(L"OmniDirShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pOmniDirShadowVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pOmniDirShadowVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pOmniDirShadowVS") - 1, "Shader::m_pOmniDirShadowVS");



		hr = D3DX11CompileFromFile(L"OmniDirShadowGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pGSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_pOmniDirShadowGS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pOmniDirShadowGS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pOmniDirShadowGS") - 1, "Shader::m_pOmniDirShadowGS");



		hr = D3DX11CompileFromFile(L"OmniDirShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pOmniDirShadowPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pOmniDirShadowPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pOmniDirShadowPS") - 1, "Shader::m_pOmniDirShadowPS");



		hr = D3DX11CompileFromFile(L"TexVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pTexVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pTexVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pTexVS") - 1, "Shader::m_pTexVS");

		hr=pDevice->CreateInputLayout(vertexIED, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pTexInputLayout);
		if (FAILED(hr))
		{
			return false;
		}
		m_pTexInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pTexInputLayout") - 1, "Shader::m_pTexInputLayout");


		hr = D3DX11CompileFromFile(L"GeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pGeometryVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pGeometryVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pGeometryVS") - 1, "Shader::m_pGeometryVS");

		hr=pDevice->CreateInputLayout(vertexIED, 4, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),&m_pDeferredGeoLayout);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDeferredGeoLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pDeferredGeoLayout") - 1, "Shader::m_pDeferredGeoLayout");


		hr = D3DX11CompileFromFile(L"GeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pGeometryPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pGeometryPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pGeometryPS") - 1, "Shader::m_pGeometryPS");

		
		hr=D3DX11CompileFromFile(L"SSAOPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pSSAOPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pSSAOPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pSSAOPS") - 1, "Shader::m_pSSAOPS");


		hr = D3DX11CompileFromFile(L"SSAOBlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pSSAOBlurPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pSSAOBlurPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pSSAOBlurPS") - 1, "Shader::m_pSSAOBlurPS");


		hr = D3DX11CompileFromFile(L"DeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0",  D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pDeferredPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pDeferredPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pDeferredPS") - 1, "Shader::m_pDeferredPS");



		hr = D3DX11CompileFromFile(L"PBRGeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pPBRGeometryVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pPBRGeometryVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pPBRGeometryVS") - 1, "Shader::m_pPBRGeometryVS");



		hr = D3DX11CompileFromFile(L"PBRGeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPBRGeometryPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pPBRGeometryPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pPBRGeometryPS") - 1, "Shader::m_pPBRGeometryPS");


		hr = D3DX11CompileFromFile(L"PBRDeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPBRDeferredPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pPBRDeferredPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pPBRDeferredPS") - 1, "Shader::m_pPBRDeferredPS");



		hr = D3DX11CompileFromFile(L"LightCubeVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pLightCubeVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pLightCubeVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pLightCubeVS") - 1, "Shader::m_pLightCubeVS");




		hr = D3DX11CompileFromFile(L"LightCubePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pLightCubePS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pLightCubePS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pLightCubePS") - 1, "Shader::m_pLightCubePS");


		hr = D3DX11CompileFromFile(L"AABBVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pVSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pAABBVS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pAABBVS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pAABBVS") - 1, "Shader::m_pAABBVS");


		hr = D3DX11CompileFromFile(L"AABBPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pAABBPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pAABBPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pAABBPS") - 1, "Shader::m_pAABBPS");


		hr = D3DX11CompileFromFile(L"OutlinerPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pOutlinerPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pOutlinerPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pOutlinerPS") - 1, "Shader::m_pOutlinerPS");



		hr = D3DX11CompileFromFile(L"BlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pBlurPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pBlurPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pBlurPS") - 1, "Shader::m_pBlurPS");



		hr = D3DX11CompileFromFile(L"FinPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS  | D3DCOMPILE_DEBUG, 0, nullptr, &pPSBlob, &pErrorBlob, nullptr);
		if (FAILED(hr))
		{	
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			return false;
		}
		hr=pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pFinPS);
		if (FAILED(hr))
		{
			return false;
		}
		m_pFinPS->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Shader::m_pFinPS") - 1, "Shader::m_pFinPS");



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