
#include <d3dcompiler.h>
#include "Shader12.h"
#include "typedef.h"
#pragma comment(lib, "d3dcompiler.lib")
namespace wilson
{
	Shader12::Shader12(ID3D12Device* pDevice)
	{
	
		//Init D3D12Resource
		{

			m_pForwardVS = nullptr;
			m_pForwardPS = nullptr;

			m_pPosOnly12VS = nullptr;
			m_pCascadeDir12VS = nullptr;
			m_pCascadeDir12GS = nullptr;
			m_pCascadeDir12PS = nullptr;
			m_pEquirect2Cube12GS = nullptr;
			m_pEquirect2Cube12PS = nullptr;
			m_pDiffuseIrradiance12PS = nullptr;
			m_pPrefilter12PS = nullptr;
			m_pBRDF12PS = nullptr;

			m_pSkyBox12VS = nullptr;
			m_pSkyBox12PS = nullptr;

			m_pShadow12VS = nullptr;
			m_pShadow12PS = nullptr;

			m_pOmniDirShadow12VS = nullptr;
			m_pOmniDirShadow12GS = nullptr;
			m_pOmniDirShadow12PS = nullptr;

			m_pTex12VS = nullptr;

			m_pGeometry12VS = nullptr;
			m_pGeometry12PS = nullptr;
			m_pSSAO12VS = nullptr;
			m_pSSAO12PS = nullptr;
			m_pSSAOBlur12PS = nullptr;
			m_pDeferred12PS = nullptr;

			m_pPBRGeometry12VS = nullptr;
			m_pPBRGeometryNormal12VS = nullptr;
			m_pPBRGeometry12PS = nullptr;
			m_pPBRGeometryEmissive12PS = nullptr;
			m_pPBRGeometryNormal12PS = nullptr;
			m_pPBRGeometryNormalHeight12PS = nullptr;
			m_pPBRGeometryNormalHeightEmissive12PS = nullptr;
			m_pPBRDeferred12PS = nullptr;

			m_pMatrixTransformVS = nullptr;
			m_pConstantPS = nullptr;

			m_pAABB12VS = nullptr;
			m_pAABB12PS = nullptr;

			m_pOutlinerTest12PS = nullptr;
			m_pBlur12PS = nullptr;
			m_pFin12PS = nullptr;

			m_pGenMipCS = nullptr;
			m_pSSAOCS = nullptr;
			m_pSSAOBlurCS = nullptr;

			m_pZpassRootSignature = nullptr;
			m_pCasacadePassRootSignature = nullptr;
			m_pSpotShadowRootSignature = nullptr;
			m_pCubeShadowRootSignature = nullptr;
			m_pSkyboxRootSignature = nullptr;
			m_pPBRGeoRootSignature = nullptr;
			m_pSSAORootSignature = nullptr;
			m_pSSAOBlurRootSignature = nullptr;
			m_pPBRLightRootSignature = nullptr;
			m_pOutlinerTestRootSignature = nullptr;
			m_pFinalRootSignature = nullptr;
			m_pPrefilterRootSignature = nullptr;
			m_pBrdfRootSignature = nullptr;
			m_pDiffuseIrradianceRootSignature = nullptr;
			m_pEquirect2cubeRootSignature = nullptr;
			m_pBlurRootSignature = nullptr;
			m_pAABBRootSignature = nullptr;
			m_pCubeRootsignature = nullptr;
			m_pGeoRootSignature = nullptr;
			m_pLightRootSignature = nullptr;
			m_pGenMipMapRootsignature = nullptr;
		}

		//Compile .hlsl
		{
			HRESULT hr;
			ID3DBlob* pErrorBlob;
			hr = D3DCompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &m_pForwardVS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &m_pForwardPS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PosOnlyVS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &m_pPosOnly12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CascadeVS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &m_pCascadeDir12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CascadeGS.hlsl", nullptr, nullptr, "main", "gs_5_0", 0, 0, &m_pCascadeDir12GS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CascadePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pCascadeDir12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"Equirectangular2CubeMapGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3DCOMPILE_DEBUG, 0, &m_pEquirect2Cube12GS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"Equirectangular2CubeMapPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pEquirect2Cube12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"DiffuseIrradiancePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pDiffuseIrradiance12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PrefilterPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pPrefilter12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"BRDFPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pBRDF12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SkyBoxVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pSkyBox12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SkyBoxPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pSkyBox12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"ShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pShadow12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"ShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pShadow12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"OmniDirShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pOmniDirShadow12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"OmniDirShadowGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3DCOMPILE_DEBUG, 0, &m_pOmniDirShadow12GS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"OmniDirShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pOmniDirShadow12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));


			hr = D3DCompileFromFile(L"TexVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pTex12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));


			hr = D3DCompileFromFile(L"GeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pGeometry12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"GeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pGeometry12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pSSAO12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pSSAO12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOCS.hlsl", nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG, 0, &m_pSSAOCS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOBlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pSSAOBlur12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOBlurCS.hlsl", nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG, 0, &m_pSSAOBlurCS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"DeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pDeferred12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryVS.hlsli", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pPBRGeometry12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryVS_Normal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pPBRGeometryNormal12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryPS.hlsli", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pPBRGeometry12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));
		
			hr = D3DCompileFromFile(L"PBRGeometryPS_Emissive.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pPBRGeometryEmissive12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryPS_Normal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pPBRGeometryNormal12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryPS_Nomral_Height.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pPBRGeometryNormalHeight12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));
			
			hr = D3DCompileFromFile(L"PBRGeometryPS_Nomral_Height_Emissive.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pPBRGeometryNormalHeightEmissive12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));
			
			hr = D3DCompileFromFile(L"PBRDeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pPBRDeferred12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));


			hr = D3DCompileFromFile(L"MatrixTransformVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pMatrixTransformVS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"ConstantPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pConstantPS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"AABBVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, &m_pAABB12VS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"AABBPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pAABB12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));



			hr = D3DCompileFromFile(L"OutlinerPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pOutlinerTest12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"BlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pBlur12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));


			hr = D3DCompileFromFile(L"FinPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, &m_pFin12PS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"GenMipCS.hlsl", nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG, 0, &m_pGenMipCS, &pErrorBlob);
			assert(SUCCEEDED(hr));

			if (pErrorBlob != nullptr)
			{
				pErrorBlob->Release();
				pErrorBlob = nullptr;
			}


		}

		//Gen INPUT_LAYOUT_DESC
		{
			//Gen InputLayout
			{
				
				m_defaultElementDescs[0].SemanticName = "POSITION";
				m_defaultElementDescs[0].SemanticIndex = 0;
				m_defaultElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				m_defaultElementDescs[0].InputSlot = 0;
				m_defaultElementDescs[0].AlignedByteOffset = 0;
				m_defaultElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_defaultElementDescs[0].InstanceDataStepRate = 0;

				m_defaultElementDescs[1].SemanticName = "TEXTURE";
				m_defaultElementDescs[1].SemanticIndex = 0;
				m_defaultElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
				m_defaultElementDescs[1].InputSlot = 0;
				m_defaultElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				m_defaultElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_defaultElementDescs[1].InstanceDataStepRate = 0;

				m_defaultElementDescs[2].SemanticName = "NORMAL";
				m_defaultElementDescs[2].SemanticIndex = 0;
				m_defaultElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				m_defaultElementDescs[2].InputSlot = 0;
				m_defaultElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				m_defaultElementDescs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_defaultElementDescs[2].InstanceDataStepRate = 0;

				m_defaultElementDescs[3].SemanticName = "TANGENT";
				m_defaultElementDescs[3].SemanticIndex = 0;
				m_defaultElementDescs[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				m_defaultElementDescs[3].InputSlot = 0;
				m_defaultElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				m_defaultElementDescs[3].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_defaultElementDescs[3].InstanceDataStepRate = 0;

				m_defaultElementDescs[4] = { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,
										D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
				m_defaultElementDescs[5] = { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16,
										D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
				m_defaultElementDescs[6] = { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32,
										D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
				m_defaultElementDescs[7] = { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48,
										D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

				m_inputLayout.pInputElementDescs = m_defaultElementDescs;
				m_inputLayout.NumElements = sizeof(m_defaultElementDescs) / sizeof(D3D12_INPUT_ELEMENT_DESC);

			}

			//Gen PosOnlyInputLayout
			{
				m_posOnlyElementDesc.SemanticName = "POSITION";
				m_posOnlyElementDesc.SemanticIndex = 0;
				m_posOnlyElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				m_posOnlyElementDesc.InputSlot = 0;
				m_posOnlyElementDesc.AlignedByteOffset = 0;
				m_posOnlyElementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_posOnlyElementDesc.InstanceDataStepRate = 0;

				m_posOnlyInputLayout.pInputElementDescs = &m_posOnlyElementDesc;
				m_posOnlyInputLayout.NumElements = sizeof(m_posOnlyElementDesc) / sizeof(D3D12_INPUT_ELEMENT_DESC);

			}

			//Gen TexInputLayout
			{
				m_texElementDescs[0].SemanticName = "POSITION";
				m_texElementDescs[0].SemanticIndex = 0;
				m_texElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				m_texElementDescs[0].InputSlot = 0;
				m_texElementDescs[0].AlignedByteOffset = 0;
				m_texElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_texElementDescs[0].InstanceDataStepRate = 0;

				m_texElementDescs[1].SemanticName = "TEXTURE";
				m_texElementDescs[1].SemanticIndex = 0;
				m_texElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
				m_texElementDescs[1].InputSlot = 0;
				m_texElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				m_texElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_texElementDescs[1].InstanceDataStepRate = 0;

				m_texInputLayout.pInputElementDescs = m_texElementDescs;
				m_texInputLayout.NumElements = sizeof(m_texElementDescs) / sizeof(D3D12_INPUT_ELEMENT_DESC);

			}

			//Gen DeferredGeoInputLayout
			{
				m_deferredGeoElementDescs[0].SemanticName = "POSITION";
				m_deferredGeoElementDescs[0].SemanticIndex = 0;
				m_deferredGeoElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				m_deferredGeoElementDescs[0].InputSlot = 0;
				m_deferredGeoElementDescs[0].AlignedByteOffset = 0;
				m_deferredGeoElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_deferredGeoElementDescs[0].InstanceDataStepRate = 0;

				m_deferredGeoElementDescs[1].SemanticName = "TEXTURE";
				m_deferredGeoElementDescs[1].SemanticIndex = 0;
				m_deferredGeoElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
				m_deferredGeoElementDescs[1].InputSlot = 0;
				m_deferredGeoElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				m_deferredGeoElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_deferredGeoElementDescs[1].InstanceDataStepRate = 0;

				m_deferredGeoElementDescs[2].SemanticName = "NORMAL";
				m_deferredGeoElementDescs[2].SemanticIndex = 0;
				m_deferredGeoElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				m_deferredGeoElementDescs[2].InputSlot = 0;
				m_deferredGeoElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				m_deferredGeoElementDescs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_deferredGeoElementDescs[2].InstanceDataStepRate = 0;

				m_deferredGeoElementDescs[3].SemanticName = "TANGENT";
				m_deferredGeoElementDescs[3].SemanticIndex = 0;
				m_deferredGeoElementDescs[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				m_deferredGeoElementDescs[3].InputSlot = 0;
				m_deferredGeoElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
				m_deferredGeoElementDescs[3].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				m_deferredGeoElementDescs[3].InstanceDataStepRate = 0;

				m_deferredGeoLayout.pInputElementDescs = m_deferredGeoElementDescs;
				m_deferredGeoLayout.NumElements = sizeof(m_deferredGeoElementDescs) / sizeof(D3D12_INPUT_ELEMENT_DESC);

			}
		}

		//Gen RootSignature
		{
			HRESULT hr;
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

			if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}
			//Zpass
			D3D12_DESCRIPTOR_RANGE1 zPassRanges[1] = {};
			zPassRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			zPassRanges[0].NumDescriptors = 1;
			zPassRanges[0].BaseShaderRegister = 0;
			zPassRanges[0].RegisterSpace = 0;
			zPassRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER1 zPassRootParameter[1] = {};

			zPassRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			zPassRootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
			zPassRootParameter[0].DescriptorTable.pDescriptorRanges = &zPassRanges[0];
			zPassRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC zPassRootSignatureDesc;
			zPassRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			zPassRootSignatureDesc.Desc_1_1.NumParameters = 1;
			zPassRootSignatureDesc.Desc_1_1.pParameters = zPassRootParameter;
			zPassRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			zPassRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			zPassRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			ID3DBlob* signature;
			ID3DBlob* error;
			hr = D3D12SerializeVersionedRootSignature(&zPassRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pZpassRootSignature));
			assert(SUCCEEDED(hr));
			m_pZpassRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pZpassRootSignature") - 1, "Shader12::m_pZpassRootSignature");


			//CascadeShadow Pass 
			D3D12_DESCRIPTOR_RANGE1 cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eCnt] = {};

			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eVsMat].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eVsMat].NumDescriptors = 1;
			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eVsMat].BaseShaderRegister = 0;
			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eVsMat].RegisterSpace = 0;
			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eVsMat].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eGsMat].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eGsMat].NumDescriptors = 1;
			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eGsMat].BaseShaderRegister = 0;
			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eGsMat].RegisterSpace = 0;
			cascadedShadowRanges[eCascadeShadowRP::eCascadeShadow_eGsMat].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER1 cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eCnt] = {};
			
			cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eVsMat].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eVsMat].DescriptorTable.NumDescriptorRanges = 1;
			cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eVsMat].DescriptorTable.pDescriptorRanges = &cascadedShadowRanges[eCascadeShadow_eVsMat];
			cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eVsMat].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eGsMat].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eGsMat].DescriptorTable.NumDescriptorRanges = 1;
			cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eGsMat].DescriptorTable.pDescriptorRanges = &cascadedShadowRanges[eCascadeShadow_eGsMat];
			cascadedShadowRootParameter[eCascadeShadowRP::eCascadeShadow_eGsMat].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC cascadedShadowRootSignatureDesc;
			cascadedShadowRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			cascadedShadowRootSignatureDesc.Desc_1_1.NumParameters = eCascadeShadowRP::eCascadeShadow_eCnt;
			cascadedShadowRootSignatureDesc.Desc_1_1.pParameters = cascadedShadowRootParameter;
			cascadedShadowRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			cascadedShadowRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			cascadedShadowRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			hr = D3D12SerializeVersionedRootSignature(&cascadedShadowRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pCasacadePassRootSignature));
			assert(SUCCEEDED(hr));
			m_pCasacadePassRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pCasacadePassRootSignature") - 1, "Shader12::m_pCasacadePassRootSignature");

			//Spot Shadow

			D3D12_DESCRIPTOR_RANGE1 spotShadowRanges[eSpotShadowRP::eSpotShadow_eCnt] = {};
			spotShadowRanges[eSpotShadowRP::eSpotShadow_eVsMat].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_eVsMat].NumDescriptors = 1;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_eVsMat].BaseShaderRegister = 0;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_eVsMat].RegisterSpace = 0;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_eVsMat].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].NumDescriptors = 1;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].BaseShaderRegister = 0;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].RegisterSpace = 0;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsSampler].NumDescriptors = 1;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsSampler].BaseShaderRegister = 0;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsSampler].RegisterSpace = 0;
			spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


			D3D12_ROOT_PARAMETER1 spotShadowRootParameter[eSpotShadowRP::eSpotShadow_eCnt] = {};
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_eVsMat].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_eVsMat].DescriptorTable.NumDescriptorRanges = 1;
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_eVsMat].DescriptorTable.pDescriptorRanges = &spotShadowRanges[eSpotShadow_eVsMat];
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_eVsMat].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].DescriptorTable.NumDescriptorRanges = 1;
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].DescriptorTable.pDescriptorRanges = &spotShadowRanges[eSpotShadow_ePsDiffuseMap];
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_ePsDiffuseMap].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_ePsSampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_ePsSampler].DescriptorTable.NumDescriptorRanges = 1;
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_ePsSampler].DescriptorTable.pDescriptorRanges = &spotShadowRanges[eSpotShadowRP::eSpotShadow_ePsSampler];
			spotShadowRootParameter[eSpotShadowRP::eSpotShadow_ePsSampler].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



			D3D12_VERSIONED_ROOT_SIGNATURE_DESC spotShadowRootSignatureDesc = {};
			spotShadowRootSignatureDesc.Version= D3D_ROOT_SIGNATURE_VERSION_1_1;
			spotShadowRootSignatureDesc.Desc_1_1.NumParameters = eSpotShadowRP::eSpotShadow_eCnt;
			spotShadowRootSignatureDesc.Desc_1_1.pParameters = spotShadowRootParameter;
			spotShadowRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			spotShadowRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			spotShadowRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr=D3D12SerializeVersionedRootSignature(&spotShadowRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pSpotShadowRootSignature));
			assert(SUCCEEDED(hr));
			m_pSpotShadowRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pSpotShadowRootSignature") - 1, "Shader12::m_pSpotShadowRootSignature");

			//CubeShadow
		
			D3D12_DESCRIPTOR_RANGE1 cubeShadowRanges[eCubeShadowRP::eCubeShadow_eCnt] = {};
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eVsMat].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eVsMat].NumDescriptors = 1;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eVsMat].BaseShaderRegister = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eVsMat].RegisterSpace = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eVsMat].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eGsMat].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eGsMat].NumDescriptors = 1;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eGsMat].BaseShaderRegister = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eGsMat].RegisterSpace = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_eGsMat].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsLightPos].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsLightPos].NumDescriptors = 1;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsLightPos].BaseShaderRegister = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsLightPos].RegisterSpace = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsLightPos].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].NumDescriptors = 1;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].BaseShaderRegister = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].RegisterSpace = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsSampler].NumDescriptors = 1;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsSampler].BaseShaderRegister = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsSampler].RegisterSpace = 0;
			cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


			D3D12_ROOT_PARAMETER1 cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eCnt] = {};
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eVsMat].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eVsMat].DescriptorTable.NumDescriptorRanges = 1;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eVsMat].DescriptorTable.pDescriptorRanges = &cubeShadowRanges[eCubeShadowRP::eCubeShadow_eVsMat];
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eVsMat].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eGsMat].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eGsMat].DescriptorTable.NumDescriptorRanges = 1;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eGsMat].DescriptorTable.pDescriptorRanges = &cubeShadowRanges[eCubeShadowRP::eCubeShadow_eGsMat];
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_eGsMat].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsLightPos].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsLightPos].DescriptorTable.NumDescriptorRanges = 1;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsLightPos].DescriptorTable.pDescriptorRanges = &cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsLightPos];
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsLightPos].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].DescriptorTable.NumDescriptorRanges = 1;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].DescriptorTable.pDescriptorRanges = &cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsDiffuseMap];
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsDiffuseMap].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsSampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsSampler].DescriptorTable.NumDescriptorRanges = 1;
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsSampler].DescriptorTable.pDescriptorRanges = &cubeShadowRanges[eCubeShadowRP::eCubeShadow_ePsSampler];
			cubeShadowRootParameter[eCubeShadowRP::eCubeShadow_ePsSampler].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



			D3D12_VERSIONED_ROOT_SIGNATURE_DESC cubeShadowRootSignatureDesc = {};
			cubeShadowRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			cubeShadowRootSignatureDesc.Desc_1_1.NumParameters = eCubeShadowRP::eCubeShadow_eCnt;
			cubeShadowRootSignatureDesc.Desc_1_1.pParameters = cubeShadowRootParameter;
			cubeShadowRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			cubeShadowRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			cubeShadowRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr = D3D12SerializeVersionedRootSignature(&cubeShadowRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pCubeShadowRootSignature));
			assert(SUCCEEDED(hr));
			m_pCubeShadowRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pCubeShadowRootSignature") - 1, "Shader12::m_pCubeShadowRootSignature");


			//SkyBox
		
			D3D12_DESCRIPTOR_RANGE1 skyboxRanges[eSkyboxRP::eSkybox_eCnt] = {};
			skyboxRanges[eSkyboxRP::eSkybox_eVsMat].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			skyboxRanges[eSkyboxRP::eSkybox_eVsMat].NumDescriptors = 1;
			skyboxRanges[eSkyboxRP::eSkybox_eVsMat].BaseShaderRegister = 0;
			skyboxRanges[eSkyboxRP::eSkybox_eVsMat].RegisterSpace = 0;
			skyboxRanges[eSkyboxRP::eSkybox_eVsMat].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			skyboxRanges[eSkyboxRP::eSkybox_ePsDiffuseMap].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			skyboxRanges[eSkyboxRP::eSkybox_ePsDiffuseMap].NumDescriptors = 1;
			skyboxRanges[eSkyboxRP::eSkybox_ePsDiffuseMap].BaseShaderRegister = 0;
			skyboxRanges[eSkyboxRP::eSkybox_ePsDiffuseMap].RegisterSpace = 0;
			skyboxRanges[eSkyboxRP::eSkybox_ePsDiffuseMap].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			skyboxRanges[eSkyboxRP::eSkybox_ePsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			skyboxRanges[eSkyboxRP::eSkybox_ePsSampler].NumDescriptors = 1;
			skyboxRanges[eSkyboxRP::eSkybox_ePsSampler].BaseShaderRegister = 0;
			skyboxRanges[eSkyboxRP::eSkybox_ePsSampler].RegisterSpace = 0;
			skyboxRanges[eSkyboxRP::eSkybox_ePsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


			D3D12_ROOT_PARAMETER1 skyboxRootParameter[eSkyboxRP::eSkybox_eCnt] = {};
			skyboxRootParameter[eSkyboxRP::eSkybox_eVsMat].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			skyboxRootParameter[eSkyboxRP::eSkybox_eVsMat].DescriptorTable.NumDescriptorRanges = 1;
			skyboxRootParameter[eSkyboxRP::eSkybox_eVsMat].DescriptorTable.pDescriptorRanges = &skyboxRanges[eSkybox_eVsMat];
			skyboxRootParameter[eSkyboxRP::eSkybox_eVsMat].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			skyboxRootParameter[eSkyboxRP::eSkybox_ePsDiffuseMap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			skyboxRootParameter[eSkyboxRP::eSkybox_ePsDiffuseMap].DescriptorTable.NumDescriptorRanges = 1;
			skyboxRootParameter[eSkyboxRP::eSkybox_ePsDiffuseMap].DescriptorTable.pDescriptorRanges = &skyboxRanges[eSkybox_ePsDiffuseMap];
			skyboxRootParameter[eSkyboxRP::eSkybox_ePsDiffuseMap].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			skyboxRootParameter[eSkyboxRP::eSkybox_ePsSampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			skyboxRootParameter[eSkyboxRP::eSkybox_ePsSampler].DescriptorTable.NumDescriptorRanges = 1;
			skyboxRootParameter[eSkyboxRP::eSkybox_ePsSampler].DescriptorTable.pDescriptorRanges = &skyboxRanges[eSkyboxRP::eSkybox_ePsSampler];
			skyboxRootParameter[eSkyboxRP::eSkybox_ePsSampler].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



			D3D12_VERSIONED_ROOT_SIGNATURE_DESC skyboxRootSignature = {};
			skyboxRootSignature.Version=D3D_ROOT_SIGNATURE_VERSION_1_1;
			skyboxRootSignature.Desc_1_1.NumParameters = eSkyboxRP::eSkybox_eCnt;
			skyboxRootSignature.Desc_1_1.pParameters = skyboxRootParameter;
			skyboxRootSignature.Desc_1_1.NumStaticSamplers = 0;
			skyboxRootSignature.Desc_1_1.pStaticSamplers = nullptr;
			skyboxRootSignature.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr=D3D12SerializeVersionedRootSignature(&skyboxRootSignature, &signature, &error);
			assert(SUCCEEDED(hr));
			hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pSkyboxRootSignature));
			assert(SUCCEEDED(hr));
			m_pSkyboxRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pSkyboxRootSignature") - 1, "Shader12::m_pSkyboxRootSignature");

			//PBR Geometry
	
			D3D12_DESCRIPTOR_RANGE1 PbrGeometryRanges[ePbrGeoRP::ePbrGeo_eCnt] = {};
			for (int i = ePbrGeoRP::ePbrGeo_eVsMat; i < ePbrGeoRP::ePbrGeo_ePsDiffuse; ++i)
			{	
				PbrGeometryRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				PbrGeometryRanges[i].NumDescriptors = 1;
				PbrGeometryRanges[i].BaseShaderRegister = i;
				PbrGeometryRanges[i].RegisterSpace = 0;
				PbrGeometryRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}

			for (int i = ePbrGeoRP::ePbrGeo_ePsDiffuse; i < ePbrGeo_ePsSampler; ++i)
			{
				PbrGeometryRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				PbrGeometryRanges[i].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
				PbrGeometryRanges[i].NumDescriptors = 1;
				PbrGeometryRanges[i].BaseShaderRegister = i- ePbrGeo_ePsDiffuse;
				PbrGeometryRanges[i].RegisterSpace = 0;
				PbrGeometryRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}

			PbrGeometryRanges[ePbrGeoRP::ePbrGeo_ePsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			PbrGeometryRanges[ePbrGeoRP::ePbrGeo_ePsSampler].NumDescriptors = 1;
			PbrGeometryRanges[ePbrGeoRP::ePbrGeo_ePsSampler].BaseShaderRegister = 0;
			PbrGeometryRanges[ePbrGeoRP::ePbrGeo_ePsSampler].RegisterSpace = 0;
			PbrGeometryRanges[ePbrGeoRP::ePbrGeo_ePsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			for (int i = ePbrGeo_ePsCamPos; i < ePbrGeo_eCnt; ++i)
			{
				PbrGeometryRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				PbrGeometryRanges[i].NumDescriptors = 1;
				PbrGeometryRanges[i].BaseShaderRegister = i- ePbrGeo_ePsCamPos;
				PbrGeometryRanges[i].RegisterSpace = 0;
				PbrGeometryRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}



			D3D12_ROOT_PARAMETER1 PbrGeometryRootParameter[ePbrGeoRP::ePbrGeo_eCnt] = {};
			for (int i = ePbrGeoRP::ePbrGeo_eVsMat; i < ePbrGeoRP::ePbrGeo_ePsDiffuse; ++i)
			{
				PbrGeometryRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				PbrGeometryRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
				PbrGeometryRootParameter[i].DescriptorTable.pDescriptorRanges = &PbrGeometryRanges[i];
				PbrGeometryRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			}

			for (int i = ePbrGeoRP::ePbrGeo_ePsDiffuse; i < ePbrGeo_eCnt; ++i)
			{
				PbrGeometryRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				PbrGeometryRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
				PbrGeometryRootParameter[i].DescriptorTable.pDescriptorRanges = &PbrGeometryRanges[i];
				PbrGeometryRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			}


			D3D12_VERSIONED_ROOT_SIGNATURE_DESC PbrGeometryRootSignatureDesc = {};
			PbrGeometryRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			PbrGeometryRootSignatureDesc.Desc_1_1.NumParameters = ePbrGeoRP::ePbrGeo_eCnt;
			PbrGeometryRootSignatureDesc.Desc_1_1.pParameters = PbrGeometryRootParameter;
			PbrGeometryRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			PbrGeometryRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			PbrGeometryRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr=D3D12SerializeVersionedRootSignature(&PbrGeometryRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));

			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pPBRGeoRootSignature));
			assert(SUCCEEDED(hr));
			m_pPBRGeoRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pPBRGeoRootSignature") - 1, "Shader12::m_pPBRGeoRootSignature");


			//SSAO 
			
			D3D12_DESCRIPTOR_RANGE1 SSAORanges[eSsaoRP::eSsao_eCnt] = {};

			for (int i = eSsao_eCsNoise; i < eSsao_eCsUAV; ++i)
			{
				SSAORanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				SSAORanges[i].NumDescriptors = 1;
				SSAORanges[i].BaseShaderRegister = i;
				SSAORanges[i].RegisterSpace = 0;
				SSAORanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			}

			SSAORanges[eSsao_eCsUAV].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			SSAORanges[eSsao_eCsUAV].NumDescriptors = 1;
			SSAORanges[eSsao_eCsUAV].BaseShaderRegister = 0;
			SSAORanges[eSsao_eCsUAV].RegisterSpace = 0;
			SSAORanges[eSsao_eCsUAV].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			
			for (int i = eSsao_eCsWrap; i < eSsao_eCsSamplePoints; ++i)
			{
				SSAORanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				SSAORanges[i].NumDescriptors = 1;
				SSAORanges[i].BaseShaderRegister = i- eSsao_eCsWrap;
				SSAORanges[i].RegisterSpace = 0;
				SSAORanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}
			
			for (int i = eSsao_eCsSamplePoints; i < eSsao_eCnt; ++i)
			{
				SSAORanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				SSAORanges[i].NumDescriptors = 1;
				SSAORanges[i].BaseShaderRegister = i- eSsao_eCsSamplePoints;
				SSAORanges[i].RegisterSpace = 0;
				SSAORanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}

			D3D12_ROOT_PARAMETER1 SSAORootParameter[eSsao_eCnt] = {};
			for (int i = eSsao_eCsNoise; i < eSsao_eCnt; ++i)
			{
				SSAORootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				SSAORootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
				SSAORootParameter[i].DescriptorTable.pDescriptorRanges = &SSAORanges[i];
				SSAORootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}


			D3D12_VERSIONED_ROOT_SIGNATURE_DESC SSAORootSignatureDesc = {};
			SSAORootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			SSAORootSignatureDesc.Desc_1_1.NumParameters = eSsaoRP::eSsao_eCnt;
			SSAORootSignatureDesc.Desc_1_1.pParameters = SSAORootParameter;
			SSAORootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			SSAORootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			SSAORootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;


			hr=D3D12SerializeVersionedRootSignature(&SSAORootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pSSAORootSignature));
			assert(SUCCEEDED(hr));
			m_pSSAORootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pSSAORootSignature") - 1, "Shader12::m_pSSAORootSignature");

			//SSAOBlur

			D3D12_DESCRIPTOR_RANGE1 SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCnt] = {};

			for (int i = 0; i < eSsaoBlur_eCsSsao; ++i)
			{
				SSAOBlurRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				SSAOBlurRanges[i].NumDescriptors = 1;
				SSAOBlurRanges[i].BaseShaderRegister = i;
				SSAOBlurRanges[i].RegisterSpace = 0;
				SSAOBlurRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}

			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsSsao].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsSsao].NumDescriptors = 1;
			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsSsao].BaseShaderRegister = 0;
			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsSsao].RegisterSpace = 0;
			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsSsao].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsWrap].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsWrap].NumDescriptors = 1;
			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsWrap].BaseShaderRegister = 0;
			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsWrap].RegisterSpace = 0;
			SSAOBlurRanges[eSsaoBlurRP::eSsaoBlur_eCsWrap].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


			D3D12_ROOT_PARAMETER1 SSAOBlurRootParameter[eSsaoBlurRP::eSsaoBlur_eCnt] = {};

			for (int i = 0; i < eSsaoBlur_eCnt; ++i)
			{
				SSAOBlurRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				SSAOBlurRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
				SSAOBlurRootParameter[i].DescriptorTable.pDescriptorRanges = &SSAOBlurRanges[i];
				SSAOBlurRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}


			D3D12_VERSIONED_ROOT_SIGNATURE_DESC SSAOBlurRootSignatureDesc = {};
			SSAOBlurRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			SSAOBlurRootSignatureDesc.Desc_1_1.NumParameters = eSsaoBlurRP::eSsaoBlur_eCnt;
			SSAOBlurRootSignatureDesc.Desc_1_1.pParameters = SSAOBlurRootParameter;
			SSAOBlurRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			SSAOBlurRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			SSAOBlurRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;


			hr=D3D12SerializeVersionedRootSignature(&SSAOBlurRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pSSAOBlurRootSignature));
			assert(SUCCEEDED(hr));
			m_pSSAOBlurRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pSSAOBlurRootSignature") - 1, "Shader12::m_pSSAOBlurRootSignature");

			//PbrLight
		
			D3D12_DESCRIPTOR_RANGE1 PbrLightRanges[ePbrLightRP::ePbrLight_eCnt] = {};
			for (int i = ePbrLight_ePsPos; i < ePbrLight_ePsWrap; ++i)
			{
				PbrLightRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				PbrLightRanges[i].NumDescriptors = 1;
				PbrLightRanges[i].BaseShaderRegister = i;
				PbrLightRanges[i].RegisterSpace = 0;
				PbrLightRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}

			for (int i = ePbrLight_ePsWrap; i<ePbrLight_ePsCamPos; ++i)
			{
				PbrLightRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				PbrLightRanges[i].NumDescriptors = 1;
				PbrLightRanges[i].BaseShaderRegister = i- ePbrLight_ePsWrap;
				PbrLightRanges[i].RegisterSpace = 0;
				PbrLightRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}

			for (int i = ePbrLight_ePsCamPos; i < ePbrLight_eCnt; ++i)
			{
				PbrLightRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				PbrLightRanges[i].NumDescriptors = 1;
				PbrLightRanges[i].BaseShaderRegister = i- ePbrLight_ePsCamPos;
				PbrLightRanges[i].RegisterSpace = 0;
				PbrLightRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}


			D3D12_ROOT_PARAMETER1 PbrLightRootParameter[ePbrLightRP::ePbrLight_eCnt] = {};
			for (int i = ePbrLight_ePsPos; i < ePbrLight_eCnt; ++i)
			{
				PbrLightRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				PbrLightRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
				PbrLightRootParameter[i].DescriptorTable.pDescriptorRanges = &PbrLightRanges[i];
				PbrLightRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			}

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC PbrLightRootSignatureDesc = {};
			PbrLightRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			PbrLightRootSignatureDesc.Desc_1_1.NumParameters = ePbrLightRP::ePbrLight_eCnt;
			PbrLightRootSignatureDesc.Desc_1_1.pParameters = PbrLightRootParameter;
			PbrLightRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			PbrLightRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			PbrLightRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr=D3D12SerializeVersionedRootSignature(&PbrLightRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pPBRLightRootSignature));
			assert(SUCCEEDED(hr));
			m_pPBRLightRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pPBRLightRootSignature") - 1, "Shader12::m_pPBRLightRootSignature");

			//OutlinerTest
			
			D3D12_DESCRIPTOR_RANGE1 OutlinerTestRanges[eOutlinerRP::eOutliner_eCnt] = {};
			OutlinerTestRanges[eOutlinerRP::eOutliner_eVsMat].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			OutlinerTestRanges[eOutlinerRP::eOutliner_eVsMat].NumDescriptors = 1;
			OutlinerTestRanges[eOutlinerRP::eOutliner_eVsMat].BaseShaderRegister = 0;
			OutlinerTestRanges[eOutlinerRP::eOutliner_eVsMat].RegisterSpace = 0;
			OutlinerTestRanges[eOutlinerRP::eOutliner_eVsMat].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsAlbedo].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsAlbedo].NumDescriptors = 1;
			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsAlbedo].BaseShaderRegister = 0;
			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsAlbedo].RegisterSpace = 0;
			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsAlbedo].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsWrap].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsWrap].NumDescriptors = 1;
			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsWrap].BaseShaderRegister = 0;
			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsWrap].RegisterSpace = 0;
			OutlinerTestRanges[eOutlinerRP::eOutliner_ePsWrap].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER1 OutlinerTestRootParameter[eOutlinerRP::eOutliner_eCnt] = {};
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_eVsMat].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_eVsMat].DescriptorTable.NumDescriptorRanges = 1;
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_eVsMat].DescriptorTable.pDescriptorRanges = &OutlinerTestRanges[eOutlinerRP::eOutliner_eVsMat];
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_eVsMat].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			OutlinerTestRootParameter[eOutlinerRP::eOutliner_ePsAlbedo].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_ePsAlbedo].DescriptorTable.NumDescriptorRanges = 1;
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_ePsAlbedo].DescriptorTable.pDescriptorRanges = &OutlinerTestRanges[eOutlinerRP::eOutliner_ePsAlbedo];
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_ePsAlbedo].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			OutlinerTestRootParameter[eOutlinerRP::eOutliner_ePsWrap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_ePsWrap].DescriptorTable.NumDescriptorRanges =1;
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_ePsWrap].DescriptorTable.pDescriptorRanges = &OutlinerTestRanges[eOutlinerRP::eOutliner_ePsWrap];
			OutlinerTestRootParameter[eOutlinerRP::eOutliner_ePsWrap].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC outlinerTestRootSignatureDesc = {};
			outlinerTestRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			outlinerTestRootSignatureDesc.Desc_1_1.NumParameters = eOutlinerRP::eOutliner_eCnt;
			outlinerTestRootSignatureDesc.Desc_1_1.pParameters = OutlinerTestRootParameter;
			outlinerTestRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			outlinerTestRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			outlinerTestRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr=D3D12SerializeVersionedRootSignature(&outlinerTestRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pOutlinerTestRootSignature));
			assert(SUCCEEDED(hr));
			m_pOutlinerTestRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pOutlinerTestRootSignature") - 1, "Shader12::m_pOutlinerTestRootSignature");

			//Final
		
			D3D12_DESCRIPTOR_RANGE1 finalRanges[eFinalRP::eFinal_eCnt] = {};

			finalRanges[eFinalRP::eFinal_ePsTex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			finalRanges[eFinalRP::eFinal_ePsTex].NumDescriptors = 1;
			finalRanges[eFinalRP::eFinal_ePsTex].BaseShaderRegister = 0;
			finalRanges[eFinalRP::eFinal_ePsTex].RegisterSpace = 0;
			finalRanges[eFinalRP::eFinal_ePsTex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			finalRanges[eFinalRP::eFinal_ePsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			finalRanges[eFinalRP::eFinal_ePsSampler].NumDescriptors = 1;
			finalRanges[eFinalRP::eFinal_ePsSampler].BaseShaderRegister = 0;
			finalRanges[eFinalRP::eFinal_ePsSampler].RegisterSpace = 0;
			finalRanges[eFinalRP::eFinal_ePsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			finalRanges[eFinalRP::eFinal_ePsCb].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			finalRanges[eFinalRP::eFinal_ePsCb].NumDescriptors = 1;
			finalRanges[eFinalRP::eFinal_ePsCb].BaseShaderRegister = 0;
			finalRanges[eFinalRP::eFinal_ePsCb].RegisterSpace = 0;
			finalRanges[eFinalRP::eFinal_ePsCb].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;



			D3D12_ROOT_PARAMETER1 finalRootParameter[eFinalRP::eFinal_eCnt] = {};

			finalRootParameter[eFinalRP::eFinal_ePsTex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			finalRootParameter[eFinalRP::eFinal_ePsTex].DescriptorTable.NumDescriptorRanges = 1;
			finalRootParameter[eFinalRP::eFinal_ePsTex].DescriptorTable.pDescriptorRanges = &finalRanges[eFinalRP::eFinal_ePsTex];
			finalRootParameter[eFinalRP::eFinal_ePsTex].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			finalRootParameter[eFinalRP::eFinal_ePsSampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			finalRootParameter[eFinalRP::eFinal_ePsSampler].DescriptorTable.NumDescriptorRanges = 1;
			finalRootParameter[eFinalRP::eFinal_ePsSampler].DescriptorTable.pDescriptorRanges = &finalRanges[eFinalRP::eFinal_ePsSampler];
			finalRootParameter[eFinalRP::eFinal_ePsSampler].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			finalRootParameter[eFinalRP::eFinal_ePsCb].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			finalRootParameter[eFinalRP::eFinal_ePsCb].DescriptorTable.NumDescriptorRanges = 1;
			finalRootParameter[eFinalRP::eFinal_ePsCb].DescriptorTable.pDescriptorRanges = &finalRanges[eFinalRP::eFinal_ePsCb];
			finalRootParameter[eFinalRP::eFinal_ePsCb].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



			D3D12_VERSIONED_ROOT_SIGNATURE_DESC finalRootSignatureDesc = {};
			finalRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			finalRootSignatureDesc.Desc_1_1.NumParameters = eFinalRP::eFinal_eCnt;
			finalRootSignatureDesc.Desc_1_1.pParameters = finalRootParameter;
			finalRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			finalRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			finalRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr=D3D12SerializeVersionedRootSignature(&finalRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pFinalRootSignature));
			assert(SUCCEEDED(hr));
			m_pFinalRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pFinalRootSignature") - 1, "Shader12::m_pFinalRootSignature");

			//BRDF empty
			D3D12_VERSIONED_ROOT_SIGNATURE_DESC emptyRootSignatureDesc = {};
			emptyRootSignatureDesc.Version= D3D_ROOT_SIGNATURE_VERSION_1_1;
			emptyRootSignatureDesc.Desc_1_1.NumParameters = 0;
			emptyRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			hr=D3D12SerializeVersionedRootSignature(&emptyRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pBrdfRootSignature));
			assert(SUCCEEDED(hr));
			m_pBrdfRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pBrdfRootSignature") - 1, "Shader12::m_pBrdfRootSignature");


			//DiffuseIrradiance srv, ss
		
			D3D12_DESCRIPTOR_RANGE1 diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eCnt] = {};
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].NumDescriptors = 1;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].BaseShaderRegister = 0;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].RegisterSpace = 0;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].NumDescriptors = 1;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].BaseShaderRegister = 0;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].RegisterSpace = 0;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].NumDescriptors = 1;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].BaseShaderRegister = 0;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].RegisterSpace = 0;
			diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER1 diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eCnt] = {};

			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].DescriptorTable.NumDescriptorRanges = 1;
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].DescriptorTable.pDescriptorRanges = &diffuseIrradianceRanges[eDiffuseIrraidiance_eGsCb];
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].DescriptorTable.NumDescriptorRanges = 1;
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].DescriptorTable.pDescriptorRanges = &diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex];
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].DescriptorTable.NumDescriptorRanges = 1;
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].DescriptorTable.pDescriptorRanges = &diffuseIrradianceRanges[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler];
			diffuseIrradianceRootParameter[eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC diffuseIrradianceRootSignature = {};
			diffuseIrradianceRootSignature.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			diffuseIrradianceRootSignature.Desc_1_1.NumParameters = eDiffuseIrraidianceRP::eDiffuseIrraidiance_eCnt;
			diffuseIrradianceRootSignature.Desc_1_1.pParameters = diffuseIrradianceRootParameter;
			diffuseIrradianceRootSignature.Desc_1_1.NumStaticSamplers = 0;
			diffuseIrradianceRootSignature.Desc_1_1.pStaticSamplers = nullptr;
			diffuseIrradianceRootSignature.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr=D3D12SerializeVersionedRootSignature(&diffuseIrradianceRootSignature, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pDiffuseIrradianceRootSignature));
			assert(SUCCEEDED(hr));
			m_pDiffuseIrradianceRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pDiffuseIrradianceRootSignature") - 1, "Shader12::m_pDiffuseIrradianceRootSignature");


			//Prefilter 
		
			D3D12_DESCRIPTOR_RANGE1 prefilterRanges[ePrefilterRP::ePrefilter_eCnt] = {};
			prefilterRanges[ePrefilterRP::ePrefilter_eGsCb].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			prefilterRanges[ePrefilterRP::ePrefilter_eGsCb].NumDescriptors = 1;
			prefilterRanges[ePrefilterRP::ePrefilter_eGsCb].BaseShaderRegister = 0;
			prefilterRanges[ePrefilterRP::ePrefilter_eGsCb].RegisterSpace = 0;
			prefilterRanges[ePrefilterRP::ePrefilter_eGsCb].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			prefilterRanges[ePrefilterRP::ePrefilter_ePsTex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsTex].NumDescriptors = 1;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsTex].BaseShaderRegister = 0;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsTex].RegisterSpace = 0;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsTex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			prefilterRanges[ePrefilterRP::ePrefilter_ePsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsSampler].NumDescriptors = 1;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsSampler].BaseShaderRegister = 0;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsSampler].RegisterSpace = 0;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			prefilterRanges[ePrefilterRP::ePrefilter_ePsCb].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsCb].NumDescriptors = 1;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsCb].BaseShaderRegister = 0;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsCb].RegisterSpace = 0;
			prefilterRanges[ePrefilterRP::ePrefilter_ePsCb].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER1 prefilterRootParmeter[ePrefilterRP::ePrefilter_eCnt] = {};

			prefilterRootParmeter[ePrefilterRP::ePrefilter_eGsCb].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			prefilterRootParmeter[ePrefilterRP::ePrefilter_eGsCb].DescriptorTable.NumDescriptorRanges = 1;
			prefilterRootParmeter[ePrefilterRP::ePrefilter_eGsCb].DescriptorTable.pDescriptorRanges = &prefilterRanges[ePrefilter_eGsCb];
			prefilterRootParmeter[ePrefilterRP::ePrefilter_eGsCb].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsTex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsTex].DescriptorTable.NumDescriptorRanges = 1;
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsTex].DescriptorTable.pDescriptorRanges = &prefilterRanges[ePrefilterRP::ePrefilter_ePsTex];
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsTex].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsSampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsSampler].DescriptorTable.NumDescriptorRanges =1;
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsSampler].DescriptorTable.pDescriptorRanges = &prefilterRanges[ePrefilterRP::ePrefilter_ePsSampler];
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsSampler].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsCb].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsCb].DescriptorTable.NumDescriptorRanges = 1;
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsCb].DescriptorTable.pDescriptorRanges = &prefilterRanges[ePrefilterRP::ePrefilter_ePsCb];
			prefilterRootParmeter[ePrefilterRP::ePrefilter_ePsCb].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC prefilterRootSignatureDesc = {};
			prefilterRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			prefilterRootSignatureDesc.Desc_1_1.NumParameters = ePrefilterRP::ePrefilter_eCnt;
			prefilterRootSignatureDesc.Desc_1_1.pParameters = prefilterRootParmeter;
			prefilterRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			prefilterRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			prefilterRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


			hr=D3D12SerializeVersionedRootSignature(&prefilterRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pPrefilterRootSignature));
			assert(SUCCEEDED(hr));
			m_pPrefilterRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pPrefilterRootSignature") - 1, "Shader12::m_pPrefilterRootSignature");


			//Equirect2Cube GS:cb PS:srv,ss
			
			D3D12_DESCRIPTOR_RANGE1 equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_eCnt] = {};
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_eGsCb].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_eGsCb].NumDescriptors = 1;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_eGsCb].BaseShaderRegister = 0;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_eGsCb].RegisterSpace = 0;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_eGsCb].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsTex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsTex].NumDescriptors = 1;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsTex].BaseShaderRegister = 0;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsTex].RegisterSpace = 0;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsTex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].NumDescriptors = 1;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].BaseShaderRegister = 0;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].RegisterSpace = 0;
			equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER1 equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_eCnt] = {};
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_eGsCb].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_eGsCb].DescriptorTable.NumDescriptorRanges = 1;
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_eGsCb].DescriptorTable.pDescriptorRanges = &equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_eGsCb];
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_eGsCb].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_ePsTex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_ePsTex].DescriptorTable.NumDescriptorRanges = 1;
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_ePsTex].DescriptorTable.pDescriptorRanges = &equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsTex];
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_ePsTex].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].DescriptorTable.NumDescriptorRanges = 1;
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].DescriptorTable.pDescriptorRanges = &equirect2CubeRanges[eEquirect2CubeRP::eEquirect2Cube_ePsSampler];
			equirect2CubeRootParmeter[eEquirect2CubeRP::eEquirect2Cube_ePsSampler].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC equirect2CubeRootSignatureDesc = {};
			equirect2CubeRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			equirect2CubeRootSignatureDesc.Desc_1_1.NumParameters = eEquirect2CubeRP::eEquirect2Cube_eCnt;
			equirect2CubeRootSignatureDesc.Desc_1_1.pParameters = equirect2CubeRootParmeter;
			equirect2CubeRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			equirect2CubeRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			equirect2CubeRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			hr=D3D12SerializeVersionedRootSignature(&equirect2CubeRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pEquirect2cubeRootSignature));
			assert(SUCCEEDED(hr));
			m_pEquirect2cubeRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12::m_pEquirect2cubeRootSignature") - 1, "Shader12::m_pEquirect2cubeRootSignature");


			D3D12_DESCRIPTOR_RANGE1 genMipRanges[eGenMipRP::eGenMipRP_eCnt] = {};
			genMipRanges[eGenMipRP::eGenMipRP_eCsTex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			genMipRanges[eGenMipRP::eGenMipRP_eCsTex].NumDescriptors = 1;
			genMipRanges[eGenMipRP::eGenMipRP_eCsTex].BaseShaderRegister = 0;
			genMipRanges[eGenMipRP::eGenMipRP_eCsTex].RegisterSpace = 0;
			genMipRanges[eGenMipRP::eGenMipRP_eCsTex].OffsetInDescriptorsFromTableStart= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			genMipRanges[eGenMipRP::eGenMipRP_eCsUAV].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			genMipRanges[eGenMipRP::eGenMipRP_eCsUAV].NumDescriptors = 1;
			genMipRanges[eGenMipRP::eGenMipRP_eCsUAV].BaseShaderRegister = 0;
			genMipRanges[eGenMipRP::eGenMipRP_eCsUAV].RegisterSpace = 0;
			genMipRanges[eGenMipRP::eGenMipRP_eCsUAV].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			genMipRanges[eGenMipRP::eGenMipRP_eCsSampler].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			genMipRanges[eGenMipRP::eGenMipRP_eCsSampler].NumDescriptors = 1;
			genMipRanges[eGenMipRP::eGenMipRP_eCsSampler].BaseShaderRegister = 0;
			genMipRanges[eGenMipRP::eGenMipRP_eCsSampler].RegisterSpace = 0;
			genMipRanges[eGenMipRP::eGenMipRP_eCsSampler].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			genMipRanges[eGenMipRP::eGenMipRP_eCsCb].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			genMipRanges[eGenMipRP::eGenMipRP_eCsCb].NumDescriptors = 1;
			genMipRanges[eGenMipRP::eGenMipRP_eCsCb].BaseShaderRegister = 0;
			genMipRanges[eGenMipRP::eGenMipRP_eCsCb].RegisterSpace = 0;
			genMipRanges[eGenMipRP::eGenMipRP_eCsCb].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


			D3D12_ROOT_PARAMETER1 genMipRootParmeter[eGenMipRP::eGenMipRP_eCnt] = {};
			for (int i = eGenMipRP_eCsTex; i < eGenMipRP_eCnt; ++i)
			{
				genMipRootParmeter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				genMipRootParmeter[i].DescriptorTable.NumDescriptorRanges = 1;
				genMipRootParmeter[i].DescriptorTable.pDescriptorRanges = &genMipRanges[i];
				genMipRootParmeter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC genMipRootSignatureDesc = {};
			genMipRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			genMipRootSignatureDesc.Desc_1_1.NumParameters = eGenMipRP::eGenMipRP_eCnt;
			genMipRootSignatureDesc.Desc_1_1.pParameters = genMipRootParmeter;
			genMipRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
			genMipRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
			genMipRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

			hr = D3D12SerializeVersionedRootSignature(&genMipRootSignatureDesc, &signature, &error);
			assert(SUCCEEDED(hr));
			hr=pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pGenMipMapRootsignature));
			assert(SUCCEEDED(hr));
			m_pGenMipMapRootsignature->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Shader12:m_pGenMipMapRootsignature") - 1, "Shader12::m_pGenMipMapRootsignature");

		}
	}

	Shader12::~Shader12()
	{

		//Delete D3D12Resources
		{
			if (m_pForwardVS != nullptr)
			{
				m_pForwardVS->Release();
				m_pForwardVS = nullptr;
			}

			if (m_pForwardPS != nullptr)
			{
				m_pForwardPS->Release();
				m_pForwardPS = nullptr;
			}

			if (m_pSkyBox12VS != nullptr)
			{
				m_pSkyBox12VS->Release();
				m_pSkyBox12VS = nullptr;
			}

			if (m_pSkyBox12PS != nullptr)
			{
				m_pSkyBox12PS->Release();
				m_pSkyBox12PS = nullptr;
			}



			if (m_pShadow12VS != nullptr)
			{
				m_pShadow12VS->Release();
				m_pShadow12VS = nullptr;
			}
			if (m_pShadow12PS != nullptr)
			{
				m_pShadow12PS->Release();
				m_pShadow12PS = nullptr;
			}



			if (m_pPosOnly12VS != nullptr)
			{
				m_pPosOnly12VS->Release();
				m_pPosOnly12VS = nullptr;
			}
			if (m_pEquirect2Cube12GS != nullptr)
			{
				m_pEquirect2Cube12GS->Release();
				m_pEquirect2Cube12GS = nullptr;
			}
			if (m_pEquirect2Cube12PS != nullptr)
			{
				m_pEquirect2Cube12PS->Release();
				m_pEquirect2Cube12PS = nullptr;
			}
			if (m_pDiffuseIrradiance12PS != nullptr)
			{
				m_pDiffuseIrradiance12PS->Release();
				m_pDiffuseIrradiance12PS = nullptr;
			}
			if (m_pPrefilter12PS != nullptr)
			{
				m_pPrefilter12PS->Release();
				m_pPrefilter12PS = nullptr;
			}
			if (m_pBRDF12PS != nullptr)
			{
				m_pBRDF12PS->Release();
				m_pBRDF12PS = nullptr;
			}
			if (m_pCascadeDir12VS != nullptr)
			{
				m_pCascadeDir12VS->Release();
				m_pCascadeDir12VS = nullptr;
			}

			if (m_pCascadeDir12GS != nullptr)
			{
				m_pCascadeDir12GS->Release();
				m_pCascadeDir12GS = nullptr;
			}

			if (m_pCascadeDir12PS != nullptr)
			{
				m_pCascadeDir12PS->Release();
				m_pCascadeDir12PS = nullptr;
			}

			if (m_pOmniDirShadow12VS != nullptr)
			{
				m_pOmniDirShadow12VS->Release();
				m_pOmniDirShadow12VS = nullptr;
			}

			if (m_pOmniDirShadow12GS != nullptr)
			{
				m_pOmniDirShadow12GS->Release();
				m_pOmniDirShadow12GS = nullptr;
			}

			if (m_pOmniDirShadow12PS != nullptr)
			{
				m_pOmniDirShadow12PS->Release();
				m_pOmniDirShadow12PS = nullptr;
			}

			if (m_pTex12VS != nullptr)
			{
				m_pTex12VS->Release();
				m_pTex12VS = nullptr;
			}

			if (m_pGeometry12VS != nullptr)
			{
				m_pGeometry12VS->Release();
				m_pGeometry12VS = nullptr;
			}
			if (m_pGeometry12PS != nullptr)
			{
				m_pGeometry12PS->Release();
				m_pGeometry12PS = nullptr;
			}

			if (m_pSSAO12VS != nullptr)
			{
				m_pSSAO12VS->Release();
				m_pSSAO12VS = nullptr;
			}

			if (m_pSSAO12PS != nullptr)
			{
				m_pSSAO12PS->Release();
				m_pSSAO12PS = nullptr;
			}
			if (m_pSSAOBlur12PS != nullptr)
			{
				m_pSSAOBlur12PS->Release();
				m_pSSAOBlur12PS = nullptr;
			}

			if (m_pDeferred12PS != nullptr)
			{
				m_pDeferred12PS->Release();
				m_pDeferred12PS = nullptr;
			}

			if (m_pPBRGeometry12VS != nullptr)
			{
				m_pPBRGeometry12VS->Release();
				m_pPBRGeometry12VS = nullptr;
			}
			if (m_pPBRGeometryNormal12VS != nullptr)
			{
				m_pPBRGeometryNormal12VS->Release();
				m_pPBRGeometryNormal12VS = nullptr;
			}
			if (m_pPBRGeometry12PS != nullptr)
			{
				m_pPBRGeometry12PS->Release();
				m_pPBRGeometry12PS = nullptr;
			}
			if (m_pPBRGeometryEmissive12PS != nullptr)
			{
				m_pPBRGeometryEmissive12PS->Release();
				m_pPBRGeometryEmissive12PS = nullptr;
			}
			if (m_pPBRGeometryNormal12PS != nullptr)
			{
				m_pPBRGeometryNormal12PS->Release();
				m_pPBRGeometryNormal12PS = nullptr;
			}
			if (m_pPBRGeometryNormalHeight12PS != nullptr)
			{
				m_pPBRGeometryNormalHeight12PS->Release();
				m_pPBRGeometryNormalHeight12PS = nullptr;
			}
			if (m_pPBRGeometryNormalHeightEmissive12PS != nullptr)
			{
				m_pPBRGeometryNormalHeightEmissive12PS->Release();
				m_pPBRGeometryNormalHeightEmissive12PS = nullptr;
			}
			if (m_pPBRDeferred12PS != nullptr)
			{
				m_pPBRDeferred12PS->Release();
				m_pPBRDeferred12PS = nullptr;
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

			if (m_pOutlinerTest12PS != nullptr)
			{
				m_pOutlinerTest12PS->Release();
				m_pOutlinerTest12PS = nullptr;
			}

			if (m_pBlur12PS != nullptr)
			{
				m_pBlur12PS->Release();
				m_pBlur12PS = nullptr;
			}

			if (m_pFin12PS != nullptr)
			{
				m_pFin12PS->Release();
				m_pFin12PS = nullptr;
			}
			if (m_pAABB12VS != nullptr)
			{
				m_pAABB12VS->Release();
				m_pAABB12VS = nullptr;
			}
			if (m_pAABB12PS != nullptr)
			{
				m_pAABB12PS->Release();
				m_pAABB12PS = nullptr;
			}

			if (m_pGenMipCS != nullptr)
			{
				m_pGenMipCS->Release();
				m_pGenMipCS = nullptr;
			}

			if (m_pSSAOCS != nullptr)
			{
				m_pSSAOCS->Release();
				m_pSSAOCS = nullptr;
			}

			if (m_pSSAOBlurCS != nullptr)
			{
				m_pSSAOBlurCS->Release();
				m_pSSAOBlurCS = nullptr;
			}

			if (m_pZpassRootSignature != nullptr)
			{
				m_pZpassRootSignature->Release();
				m_pZpassRootSignature = nullptr;
			}

			if (m_pCasacadePassRootSignature != nullptr)
			{
				m_pCasacadePassRootSignature->Release();
				m_pCasacadePassRootSignature = nullptr;
			}
			if (m_pSpotShadowRootSignature != nullptr)
			{
				m_pSpotShadowRootSignature->Release();
				m_pSpotShadowRootSignature = nullptr;
			}
			if (m_pCubeShadowRootSignature != nullptr)
			{
				m_pCubeShadowRootSignature->Release();
				m_pCubeShadowRootSignature = nullptr;
			}
			if (m_pSkyboxRootSignature != nullptr)
			{
				m_pSkyboxRootSignature->Release();
				m_pSkyboxRootSignature = nullptr;
			}
			if (m_pPBRGeoRootSignature != nullptr)
			{
				m_pPBRGeoRootSignature->Release();
				m_pPBRGeoRootSignature = nullptr;
			}
			if (m_pSSAORootSignature != nullptr)
			{
				m_pSSAORootSignature->Release();
				m_pSSAORootSignature = nullptr;
			}
			if (m_pSSAOBlurRootSignature != nullptr)
			{
				m_pSSAOBlurRootSignature->Release();
				m_pSSAOBlurRootSignature = nullptr;
			}
			if (m_pPBRLightRootSignature != nullptr)
			{
				m_pPBRLightRootSignature->Release();
				m_pPBRLightRootSignature = nullptr;
			}
			if (m_pOutlinerTestRootSignature != nullptr)
			{
				m_pOutlinerTestRootSignature->Release();
				m_pOutlinerTestRootSignature = nullptr;
			}
			if (m_pFinalRootSignature != nullptr)
			{
				m_pFinalRootSignature->Release();
				m_pFinalRootSignature = nullptr;
			}

			if (m_pPrefilterRootSignature != nullptr)
			{
				m_pPrefilterRootSignature->Release();
				m_pPrefilterRootSignature = nullptr;
			}

			if (m_pBrdfRootSignature != nullptr)
			{
				m_pBrdfRootSignature->Release();
				m_pBrdfRootSignature = nullptr;
			}

			if (m_pDiffuseIrradianceRootSignature != nullptr)
			{
				m_pDiffuseIrradianceRootSignature->Release();
				m_pDiffuseIrradianceRootSignature = nullptr;
			}

			if (m_pEquirect2cubeRootSignature != nullptr)
			{
				m_pEquirect2cubeRootSignature->Release();
				m_pEquirect2cubeRootSignature = nullptr;
			}

			if (m_pBlurRootSignature != nullptr)
			{
				m_pBlurRootSignature->Release();
				m_pBlurRootSignature = nullptr;
			}

			if (m_pAABBRootSignature != nullptr)
			{
				m_pAABBRootSignature->Release();
				m_pAABBRootSignature = nullptr;
			}

			if (m_pCubeRootsignature != nullptr)
			{
				m_pCubeRootsignature->Release();
				m_pCubeRootsignature = nullptr;
			}

			if (m_pGeoRootSignature != nullptr)
			{
				m_pGeoRootSignature->Release();
				m_pGeoRootSignature = nullptr;
			}

			if (m_pLightRootSignature != nullptr)
			{
				m_pLightRootSignature->Release();
				m_pLightRootSignature = nullptr;
			}

			if (m_pGenMipMapRootsignature != nullptr)
			{
				m_pGenMipMapRootsignature->Release();
				m_pGenMipMapRootsignature = nullptr;
			}
		}

	}

}