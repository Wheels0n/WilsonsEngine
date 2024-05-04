#include "Shader11.h"
#include <D3Dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
namespace wilson
{
	Shader11::Shader11(ID3D11Device* const pDevice, ID3D11DeviceContext* const pContext)
	{
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
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pVs") - 1, "Shader11::m_pVs");

			hr = D3DCompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_DEBUG | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3D10_SHADER_SKIP_OPTIMIZATION, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pPs") - 1, "Shader11::m_pPs");


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

			hr = pDevice->CreateInputLayout(vertexIED, sizeof(vertexIED) / sizeof(vertexIED[0]), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_pInputLayout.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pInputLayout") - 1, "Shader11::m_pInputLayout");

			hr = D3DCompileFromFile(L"PosOnlyVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pPosOnlyVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPosOnlyVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pPosOnlyVs") - 1, "Shader11::m_pPosOnlyVs");


			hr = D3DCompileFromFile(L"CascadeVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pCascadeDirVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCascadeDirVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pCascadeDirVs") - 1, "Shader11::m_pCascadeDirVs");



			hr = D3DCompileFromFile(L"CascadeGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pGSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, m_pCascadeDirGs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCascadeDirGs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pCascadeDirGs") - 1, "Shader11::m_pCascadeDirGs");

			hr = D3DCompileFromFile(L"CascadePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pCascadeDirPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCascadeDirPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pCascadeDirPs") - 1, "Shader11::m_pCascadeDirPs");





			hr = D3DCompileFromFile(L"Equirectangular2CubeMapGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pGSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, m_pEquirect2CubeGs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pEquirect2CubeGs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pEquirect2CubeGs") - 1, "Shader11::m_pEquirect2CubeGs");


			hr = D3DCompileFromFile(L"Equirectangular2CubeMapPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pEquirect2CubePs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pEquirect2CubePs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pEquirect2CubePs") - 1, "Shader11::m_pEquirect2CubePs");

			hr = D3DCompileFromFile(L"DiffuseIrradiancePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pDiffuseIrradiancePs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pDiffuseIrradiancePs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pDiffuseIrradiancePs") - 1, "Shader11::m_pDiffuseIrradiancePs");

			hr = D3DCompileFromFile(L"PrefilterPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,&pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pPrefilterPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPrefilterPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pPrefilterPs") - 1, "Shader11::m_pPrefilterPs");


			hr = D3DCompileFromFile(L"BRDFPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pBrdfPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pBrdfPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pBrdfPs") - 1, "Shader11::m_pBrdfPs");





			hr = D3DCompileFromFile(L"SkyBoxVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pSkyBoxVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pSkyBoxVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pSkyBoxVs") - 1, "Shader11::m_pSkyBoxVs");

			hr = D3DCompileFromFile(L"SkyBoxPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pSkyBoxPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pSkyBoxPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pSkyBoxPs") - 1, "Shader11::m_pSkyBoxPs");


			posOnlyIED = vertexIED[0];
			hr = pDevice->CreateInputLayout(&posOnlyIED, 1, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_pPosOnlyInputLayout.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPosOnlyInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pPosOnlyInputLayout") - 1, "Shader11::m_pPosOnlyInputLayout");



			hr = D3DCompileFromFile(L"ShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pShadowVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pShadowVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pShadowVs") - 1, "Shader11::m_pShadowVs");

			hr = D3DCompileFromFile(L"ShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pShadowPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pShadowPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pShadowPs") - 1, "Shader11::m_pShadowPs");



			hr = D3DCompileFromFile(L"CubeShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pCubeShadowVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCubeShadowVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pCubeShadowVs") - 1, "Shader11::m_pCubeShadowVs");



			hr = D3DCompileFromFile(L"CubeShadowGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pGSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, m_pCubeShadowGs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCubeShadowGs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pCubeShadowGs") - 1, "Shader11::m_pCubeShadowGs");



			hr = D3DCompileFromFile(L"CubeShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pCubeShadowPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pCubeShadowPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pCubeShadowPs") - 1, "Shader11::m_pCubeShadowPs");



			hr = D3DCompileFromFile(L"TexVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pTexVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pTexVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pTexVs") - 1, "Shader11::m_pTexVs");

			hr = pDevice->CreateInputLayout(vertexIED, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_pTexInputLayout.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pTexInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pTexInputLayout") - 1, "Shader11::m_pTexInputLayout");


			hr = D3DCompileFromFile(L"GeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pGeometryVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pGeometryVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pGeometryVs") - 1, "Shader11::m_pGeometryVs");

			hr = pDevice->CreateInputLayout(vertexIED, 4, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_pDeferredGeoLayout.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pDeferredGeoLayout->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pDeferredGeoLayout") - 1, "Shader11::m_pDeferredGeoLayout");


			hr = D3DCompileFromFile(L"GeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pGeometryPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pGeometryPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pGeometryPs") - 1, "Shader11::m_pGeometryPs");


			hr = D3DCompileFromFile(L"SSAOPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pSsaoPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pSsaoPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pSsaoPs") - 1, "Shader11::m_pSsaoPs");


			hr = D3DCompileFromFile(L"SSAOBlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pSsaoBlurPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pSsaoBlurPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pSsaoBlurPs") - 1, "Shader11::m_pSsaoBlurPs");


			hr = D3DCompileFromFile(L"DeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pDeferredPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pDeferredPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pDeferredPs") - 1, "Shader11::m_pDeferredPs");



			hr = D3DCompileFromFile(L"PBRGeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pPbrGeometryVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPbrGeometryVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pPbrGeometryVs") - 1, "Shader11::m_pPbrGeometryVs");



			hr = D3DCompileFromFile(L"PBRGeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pPbrGeometryPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPbrGeometryPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pPbrGeometryPs") - 1, "Shader11::m_pPbrGeometryPs");


			hr = D3DCompileFromFile(L"PBRDeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pPbrDeferredPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPbrDeferredPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pPbrDeferredPs") - 1, "Shader11::m_pPbrDeferredPs");



			hr = D3DCompileFromFile(L"MatrixTransformVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pMatrixTransformVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pMatrixTransformVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pMatrixTransformVs") - 1, "Shader11::m_pMatrixTransformVs");




			hr = D3DCompileFromFile(L"ConstantPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pConstantPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pConstantPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pConstantPs") - 1, "Shader11::m_pConstantPs");


			hr = D3DCompileFromFile(L"AABBVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pAabbVs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pAabbVs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pAabbVs") - 1, "Shader11::m_pAabbVs");


			hr = D3DCompileFromFile(L"AABBPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pAabbPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pAabbPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pAabbPs") - 1, "Shader11::m_pAabbPs");


			hr = D3DCompileFromFile(L"OutlinerPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pOutlinerPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pOutlinerPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pOutlinerPs") - 1, "Shader11::m_pOutlinerPs");



			hr = D3DCompileFromFile(L"BlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pBlurPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pBlurPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pBlurPs") - 1, "Shader11::m_pBlurPs");



			hr = D3DCompileFromFile(L"FinPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pPostProcessPs.GetAddressOf());
			assert(SUCCEEDED(hr));
			m_pPostProcessPs->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader11::m_pPostProcessPs") - 1, "Shader11::m_pPostProcessPs");



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

	Shader11::~Shader11()
	{
		
	}

}