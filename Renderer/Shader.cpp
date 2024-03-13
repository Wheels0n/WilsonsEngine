#include "Shader.h"
#include <D3Dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
namespace wilson
{
	Shader::Shader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		//Init D3D11Resource
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

			m_pCubeShadowVS = nullptr;
			m_pCubeShadowGS = nullptr;
			m_pCubeShadowPS = nullptr;

			m_pTexVS = nullptr;

			m_pGeometryVS = nullptr;
			m_pGeometryPS = nullptr;
			m_pSSAOPS = nullptr;
			m_pSSAOBlurPS = nullptr;
			m_pDeferredPS = nullptr;

			m_pPBRGeometryVS = nullptr;
			m_pPBRGeometryPS = nullptr;
			m_pPBRDeferredPS = nullptr;

			m_pMatrixTransformVS = nullptr;
			m_pConstantPS = nullptr;

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
		
		{
			HRESULT hr;
			D3D11_INPUT_ELEMENT_DESC vertexIED[8];
			D3D11_INPUT_ELEMENT_DESC posOnlyIED;
			ID3DBlob* pVSBlob = nullptr;
			ID3DBlob* pGSBlob = nullptr;
			ID3DBlob* pPSBlob = nullptr;
			ID3DBlob* pErrorBlob = nullptr;

			hr = D3DCompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVS);
			assert(SUCCEEDED(hr));
			m_pVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pVS") - 1, "Shader::m_pVS");

			hr = D3DCompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_DEBUG | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3D10_SHADER_SKIP_OPTIMIZATION, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPS);
			assert(SUCCEEDED(hr));
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

			vertexIED[3].SemanticName = "TANGENT";
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

			hr = pDevice->CreateInputLayout(vertexIED, sizeof(vertexIED) / sizeof(vertexIED[0]), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout);
			assert(SUCCEEDED(hr));
			m_pInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pInputLayout") - 1, "Shader::m_pInputLayout");

			hr = D3DCompileFromFile(L"PosOnlyVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pPosOnlyVS);
			assert(SUCCEEDED(hr));
			m_pPosOnlyVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pPosOnlyVS") - 1, "Shader::m_pPosOnlyVS");


			hr = D3DCompileFromFile(L"CascadeVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pCascadeDirVS);
			assert(SUCCEEDED(hr));
			m_pCascadeDirVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pCascadeDirVS") - 1, "Shader::m_pCascadeDirVS");



			hr = D3DCompileFromFile(L"CascadeGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pGSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_pCascadeDirGS);
			assert(SUCCEEDED(hr));
			m_pCascadeDirGS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pCascadeDirGS") - 1, "Shader::m_pCascadeDirGS");

			hr = D3DCompileFromFile(L"CascadePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pCascadeDirPS);
			assert(SUCCEEDED(hr));
			m_pCascadeDirPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pCascadeDirPS") - 1, "Shader::m_pCascadeDirPS");





			hr = D3DCompileFromFile(L"Equirectangular2CubeMapGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pGSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_pEquirect2CubeGS);
			assert(SUCCEEDED(hr));
			m_pEquirect2CubeGS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pEquirect2CubeGS") - 1, "Shader::m_pEquirect2CubeGS");


			hr = D3DCompileFromFile(L"Equirectangular2CubeMapPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pEquirect2CubePS);
			assert(SUCCEEDED(hr));
			m_pEquirect2CubePS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pEquirect2CubePS") - 1, "Shader::m_pEquirect2CubePS");

			hr = D3DCompileFromFile(L"DiffuseIrradiancePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pDiffuseIrradiancePS);
			assert(SUCCEEDED(hr));
			m_pDiffuseIrradiancePS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pDiffuseIrradiancePS") - 1, "Shader::m_pDiffuseIrradiancePS");

			hr = D3DCompileFromFile(L"PrefilterPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,&pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPrefilterPS);
			assert(SUCCEEDED(hr));
			m_pPrefilterPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pPrefilterPS") - 1, "Shader::m_pPrefilterPS");


			hr = D3DCompileFromFile(L"BRDFPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pBRDFPS);
			assert(SUCCEEDED(hr));
			m_pBRDFPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pBRDFPS") - 1, "Shader::m_pBRDFPS");





			hr = D3DCompileFromFile(L"SkyBoxVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pSkyBoxVS);
			assert(SUCCEEDED(hr));
			m_pSkyBoxVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pSkyBoxVS") - 1, "Shader::m_pSkyBoxVS");

			hr = D3DCompileFromFile(L"SkyBoxPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pSkyBoxPS);
			assert(SUCCEEDED(hr));
			m_pSkyBoxPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pSkyBoxPS") - 1, "Shader::m_pSkyBoxPS");


			posOnlyIED = vertexIED[0];
			hr = pDevice->CreateInputLayout(&posOnlyIED, 1, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pPosOnlyInputLayout);
			assert(SUCCEEDED(hr));
			m_pPosOnlyInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pPosOnlyInputLayout") - 1, "Shader::m_pPosOnlyInputLayout");



			hr = D3DCompileFromFile(L"ShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pShadowVS);
			assert(SUCCEEDED(hr));
			m_pShadowVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pShadowVS") - 1, "Shader::m_pShadowVS");

			hr = D3DCompileFromFile(L"ShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pShadowPS);
			assert(SUCCEEDED(hr));
			m_pShadowPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pShadowPS") - 1, "Shader::m_pShadowPS");



			hr = D3DCompileFromFile(L"CubeShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pCubeShadowVS);
			assert(SUCCEEDED(hr));
			m_pCubeShadowVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pCubeShadowVS") - 1, "Shader::m_pCubeShadowVS");



			hr = D3DCompileFromFile(L"CubeShadowGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pGSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_pCubeShadowGS);
			assert(SUCCEEDED(hr));
			m_pCubeShadowGS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pCubeShadowGS") - 1, "Shader::m_pCubeShadowGS");



			hr = D3DCompileFromFile(L"CubeShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pCubeShadowPS);
			assert(SUCCEEDED(hr));
			m_pCubeShadowPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pCubeShadowPS") - 1, "Shader::m_pCubeShadowPS");



			hr = D3DCompileFromFile(L"TexVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pTexVS);
			assert(SUCCEEDED(hr));
			m_pTexVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pTexVS") - 1, "Shader::m_pTexVS");

			hr = pDevice->CreateInputLayout(vertexIED, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pTexInputLayout);
			assert(SUCCEEDED(hr));
			m_pTexInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pTexInputLayout") - 1, "Shader::m_pTexInputLayout");


			hr = D3DCompileFromFile(L"GeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pGeometryVS);
			assert(SUCCEEDED(hr));
			m_pGeometryVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pGeometryVS") - 1, "Shader::m_pGeometryVS");

			hr = pDevice->CreateInputLayout(vertexIED, 4, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pDeferredGeoLayout);
			assert(SUCCEEDED(hr));
			m_pDeferredGeoLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pDeferredGeoLayout") - 1, "Shader::m_pDeferredGeoLayout");


			hr = D3DCompileFromFile(L"GeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pGeometryPS);
			assert(SUCCEEDED(hr));
			m_pGeometryPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pGeometryPS") - 1, "Shader::m_pGeometryPS");


			hr = D3DCompileFromFile(L"SSAOPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pSSAOPS);
			assert(SUCCEEDED(hr));
			m_pSSAOPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pSSAOPS") - 1, "Shader::m_pSSAOPS");


			hr = D3DCompileFromFile(L"SSAOBlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pSSAOBlurPS);
			assert(SUCCEEDED(hr));
			m_pSSAOBlurPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pSSAOBlurPS") - 1, "Shader::m_pSSAOBlurPS");


			hr = D3DCompileFromFile(L"DeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pDeferredPS);
			assert(SUCCEEDED(hr));
			m_pDeferredPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pDeferredPS") - 1, "Shader::m_pDeferredPS");



			hr = D3DCompileFromFile(L"PBRGeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pPBRGeometryVS);
			assert(SUCCEEDED(hr));
			m_pPBRGeometryVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pPBRGeometryVS") - 1, "Shader::m_pPBRGeometryVS");



			hr = D3DCompileFromFile(L"PBRGeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPBRGeometryPS);
			assert(SUCCEEDED(hr));
			m_pPBRGeometryPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pPBRGeometryPS") - 1, "Shader::m_pPBRGeometryPS");


			hr = D3DCompileFromFile(L"PBRDeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPBRDeferredPS);
			assert(SUCCEEDED(hr));
			m_pPBRDeferredPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pPBRDeferredPS") - 1, "Shader::m_pPBRDeferredPS");



			hr = D3DCompileFromFile(L"MatrixTransformVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pMatrixTransformVS);
			assert(SUCCEEDED(hr));
			m_pMatrixTransformVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pMatrixTransformVS") - 1, "Shader::m_pMatrixTransformVS");




			hr = D3DCompileFromFile(L"ConstantPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pConstantPS);
			assert(SUCCEEDED(hr));
			m_pConstantPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pConstantPS") - 1, "Shader::m_pConstantPS");


			hr = D3DCompileFromFile(L"AABBVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pAABBVS);
			assert(SUCCEEDED(hr));
			m_pAABBVS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pAABBVS") - 1, "Shader::m_pAABBVS");


			hr = D3DCompileFromFile(L"AABBPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pAABBPS);
			assert(SUCCEEDED(hr));
			m_pAABBPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pAABBPS") - 1, "Shader::m_pAABBPS");


			hr = D3DCompileFromFile(L"OutlinerPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pOutlinerPS);
			assert(SUCCEEDED(hr));
			m_pOutlinerPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pOutlinerPS") - 1, "Shader::m_pOutlinerPS");



			hr = D3DCompileFromFile(L"BlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pBlurPS);
			assert(SUCCEEDED(hr));
			m_pBlurPS->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader::m_pBlurPS") - 1, "Shader::m_pBlurPS");



			hr = D3DCompileFromFile(L"FinPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pFinPS);
			assert(SUCCEEDED(hr));
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


		}
	}

	Shader::~Shader()
	{
		//Delete D3D11Resources
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

			if (m_pCubeShadowVS != nullptr)
			{
				m_pCubeShadowVS->Release();
				m_pCubeShadowVS = nullptr;
			}

			if (m_pCubeShadowGS != nullptr)
			{
				m_pCubeShadowGS->Release();
				m_pCubeShadowGS = nullptr;
			}

			if (m_pCubeShadowPS != nullptr)
			{
				m_pCubeShadowPS->Release();
				m_pCubeShadowPS = nullptr;
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



			if (m_pMatrixTransformVS != nullptr)
			{
				m_pMatrixTransformVS->Release();
				m_pMatrixTransformVS = nullptr;
			}

			if (m_pConstantPS != nullptr)
			{
				m_pConstantPS->Release();
				m_pConstantPS = nullptr;
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
		
	}

}