
#include <d3dcompiler.h>
#include "Shader12.h"
#include "typedef.h"
#pragma comment(lib, "d3dcompiler.lib")
namespace wilson
{
	Shader12::Shader12(ID3D12Device* const pDevice)
	{
	
		//Compile .hlsl
		{
			HRESULT hr;
			ID3DBlob* pErrorBlob;
			hr = D3DCompileFromFile(L"VS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, m_pForwardVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PS.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, m_pForwardPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PosOnlyVS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, m_pPosOnlyVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CascadeVS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, m_pCascadeDirVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CascadeGS.hlsl", nullptr, nullptr, "main", "gs_5_0", 0, 0, m_pCascadeDirGs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CascadePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pCascadeDirPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"Equirectangular2CubeMapGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3DCOMPILE_DEBUG, 0, m_pEquirect2CubeGs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"Equirectangular2CubeMapPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pEquirect2CubePs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"DiffuseIrradiancePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pDiffuseIrradiancePs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PrefilterPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pPrefilterPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"BRDFPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pBrdfPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SkyBoxVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pSkyBoxVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SkyBoxPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pSkyBoxPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"ShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pShadowVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"ShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pShadowPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CubeShadowVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pCubeShadowVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CubeShadowGS.hlsl", nullptr, nullptr, "main", "gs_5_0", D3DCOMPILE_DEBUG, 0, m_pCubeShadowGs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"CubeShadowPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pCubeShadowPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));


			hr = D3DCompileFromFile(L"TexVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pTexVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));


			hr = D3DCompileFromFile(L"GeometryVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pGeometryVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"GeometryPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pGeometryPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pSsaoVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pSsaoPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOCS.hlsl", nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG, 0, m_pSsaoCs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOBlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pSsaoBlurPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"SSAOBlurCS.hlsl", nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG, 0, m_pSsaoBlurCs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"DeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pDeferredPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryVS.hlsli", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pPBRGeometryVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryVS_Normal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pPBRGeometryNormalVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryPS.hlsli", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pPbrGeometryPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));
		
			hr = D3DCompileFromFile(L"PBRGeometryPS_Emissive.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pPbrGeometryEmissivePs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryPS_Normal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pPbrGeometryNormalPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PBRGeometryPS_Nomral_Height.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pPbrGeometryNormalHeightPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));
			
			hr = D3DCompileFromFile(L"PBRGeometryPS_Nomral_Height_Emissive.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pPbrGeometryNormalHeightEmissivePs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));
			
			hr = D3DCompileFromFile(L"PBRDeferredPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pPbrDeferredPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));


			hr = D3DCompileFromFile(L"MatrixTransformVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pMatrixTransformVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));
			hr = D3DCompileFromFile(L"MatrixTransformInstancedVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pMatrixTransformInstancedVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"ConstantPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pConstantPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"AABBVS.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, m_pAabbVs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"AABBPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pAabbPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"DownSamplePS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pDownSamplePs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));


			hr = D3DCompileFromFile(L"OutlinerPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pOutlinerTestPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"BlurPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pBlurPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"FinPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pPostProcessPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"Hierarchical-zGenPS.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, m_pGenHiZPs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"Hierarchical-zCullCS.hlsl", nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG, 0, m_pHiZCullCs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"PostProcessCS.hlsl", nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG, 0, m_pPostProcessCs.GetAddressOf(), &pErrorBlob);
			assert(SUCCEEDED(hr));

			hr = D3DCompileFromFile(L"GenMipCS.hlsl", nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG, 0, m_pGenMipCs.GetAddressOf(), &pErrorBlob);
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
			{
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
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pZpassRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pZpassRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pZpassRootSignature") - 1, "Shader12::m_pZpassRootSignature");
			}		
			//DownSample
			{
				D3D12_DESCRIPTOR_RANGE1 DownSampleRanges[static_cast<UINT>(eDownSampleRP::cnt)] = {};
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::depthMap)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::depthMap)].NumDescriptors = 1;
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::depthMap)].BaseShaderRegister = 0;
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::depthMap)].RegisterSpace = 0;
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::depthMap)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				DownSampleRanges[static_cast<UINT>(eDownSampleRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::psSampler)].NumDescriptors = 1;
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::psSampler)].BaseShaderRegister = 0;
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::psSampler)].RegisterSpace = 0;
				DownSampleRanges[static_cast<UINT>(eDownSampleRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER1 DownSampleRootParameter[static_cast<UINT>(eDownSampleRP::cnt)] = {};
				for (int i = 0; i < static_cast<UINT>(eDownSampleRP::cnt); ++i)
				{
					DownSampleRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					DownSampleRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					DownSampleRootParameter[i].DescriptorTable.pDescriptorRanges = &DownSampleRanges[i];
					DownSampleRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				}
				D3D12_VERSIONED_ROOT_SIGNATURE_DESC  DownSampleRootSignatureDesc;
				DownSampleRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				DownSampleRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eDownSampleRP::cnt);
				DownSampleRootSignatureDesc.Desc_1_1.pParameters = DownSampleRootParameter;
				DownSampleRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				DownSampleRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				DownSampleRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&DownSampleRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pDownSampleRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pDownSampleRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pDownSampleRootSignature") - 1, "Shader12::m_pDownSampleRootSignature");
			}
			//Gen Hi-Z Pass
			{
				D3D12_DESCRIPTOR_RANGE1 GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::cnt)] = {};

				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastResoltion)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastResoltion)].NumDescriptors = 1;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastResoltion)].BaseShaderRegister = 0;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastResoltion)].RegisterSpace = 0;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastResoltion)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastMip)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastMip)].NumDescriptors = 1;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastMip)].BaseShaderRegister = 0;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastMip)].RegisterSpace = 0;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psLastMip)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psSampler)].NumDescriptors = 1;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psSampler)].BaseShaderRegister = 0;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psSampler)].RegisterSpace = 0;
				GenHiZPassRanges[static_cast<UINT>(eGenHiZRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER1 genHiZPassRootParameter[static_cast<UINT>(eGenHiZRP::cnt)] = {};
				for (int i = 0; i < static_cast<UINT>(eGenHiZRP::cnt); ++i)
				{
					genHiZPassRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					genHiZPassRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					genHiZPassRootParameter[i].DescriptorTable.pDescriptorRanges = &GenHiZPassRanges[i];
					genHiZPassRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				}
				D3D12_VERSIONED_ROOT_SIGNATURE_DESC  genHiZPassRootSignatureDesc;
				genHiZPassRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				genHiZPassRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eGenHiZRP::cnt);
				genHiZPassRootSignatureDesc.Desc_1_1.pParameters = genHiZPassRootParameter;
				genHiZPassRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				genHiZPassRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				genHiZPassRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&genHiZPassRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pGenHiZpassRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pGenHiZpassRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pGenHiZpassRootSignature") - 1, "Shader12::m_pGenHiZpassRootSignature");
			}
			//Hi-Z Culling Pass
			{
				D3D12_DESCRIPTOR_RANGE1 hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::cnt)] = {};
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csHiZ)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csHiZ)].NumDescriptors = 1;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csHiZ)].BaseShaderRegister = 0;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csHiZ)].RegisterSpace = 0;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csHiZ)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csDst)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csDst)].NumDescriptors = 1;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csDst)].BaseShaderRegister = 0;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csDst)].RegisterSpace = 0;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csDst)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				for (int i = static_cast<UINT>(eHiZCullRP::csMatrix); i < static_cast<UINT>(eHiZCullRP::csBorder); ++i)
				{
					hiZCullPassRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					hiZCullPassRanges[i].NumDescriptors = 1;
					hiZCullPassRanges[i].BaseShaderRegister = i - static_cast<UINT>(eHiZCullRP::csMatrix);
					hiZCullPassRanges[i].RegisterSpace = 0;
					hiZCullPassRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}

				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csBorder)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csBorder)].NumDescriptors = 1;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csBorder)].BaseShaderRegister = 0;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csBorder)].RegisterSpace = 0;
				hiZCullPassRanges[static_cast<UINT>(eHiZCullRP::csBorder)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER1 hiZCullPassRootParameter[static_cast<UINT>(eHiZCullRP::cnt)] = {};
				for (int i = 0; i < static_cast<UINT>(eHiZCullRP::cnt); ++i)
				{
					hiZCullPassRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					hiZCullPassRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					hiZCullPassRootParameter[i].DescriptorTable.pDescriptorRanges = &hiZCullPassRanges[i];
					hiZCullPassRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				}

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC hiZCullPassRootSignatureDesc = {};
				hiZCullPassRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				hiZCullPassRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eHiZCullRP::cnt);
				hiZCullPassRootSignatureDesc.Desc_1_1.pParameters = hiZCullPassRootParameter;
				hiZCullPassRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				hiZCullPassRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				hiZCullPassRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&hiZCullPassRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pHiZCullPassRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pHiZCullPassRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pHiZCullPassRootSignature") - 1, "Shader12::m_pHiZCullPassRootSignature");
			}
			//CascadeShadow Pass 
			{
				D3D12_DESCRIPTOR_RANGE1 cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::cnt)] = {};

				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::vsMat)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::vsMat)].NumDescriptors = 1;
				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::vsMat)].BaseShaderRegister = 0;
				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::vsMat)].RegisterSpace = 0;
				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::vsMat)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::gsMat)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::gsMat)].NumDescriptors = 1;
				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::gsMat)].BaseShaderRegister = 0;
				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::gsMat)].RegisterSpace = 0;
				cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::gsMat)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER1 cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::cnt)] = {};

				cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::vsMat)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::vsMat)].DescriptorTable.NumDescriptorRanges = 1;
				cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::vsMat)].DescriptorTable.pDescriptorRanges = &cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::vsMat)];
				cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::vsMat)].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::gsMat)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::gsMat)].DescriptorTable.NumDescriptorRanges = 1;
				cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::gsMat)].DescriptorTable.pDescriptorRanges = &cascadedShadowRanges[static_cast<UINT>(eCascadeShadowRP::gsMat)];
				cascadedShadowRootParameter[static_cast<UINT>(eCascadeShadowRP::gsMat)].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC cascadedShadowRootSignatureDesc;
				cascadedShadowRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				cascadedShadowRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eCascadeShadowRP::cnt);
				cascadedShadowRootSignatureDesc.Desc_1_1.pParameters = cascadedShadowRootParameter;
				cascadedShadowRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				cascadedShadowRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				cascadedShadowRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&cascadedShadowRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pCasacadePassRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pCasacadePassRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pCasacadePassRootSignature") - 1, "Shader12::m_pCasacadePassRootSignature");
			}
			//Spot Shadow
			{
				D3D12_DESCRIPTOR_RANGE1 spotShadowRanges[static_cast<UINT>(eSpotShadowRP::cnt)] = {};
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::vsMat)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::vsMat)].NumDescriptors = 1;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::vsMat)].BaseShaderRegister = 0;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::vsMat)].RegisterSpace = 0;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::vsMat)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].NumDescriptors = 1;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].BaseShaderRegister = 0;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].RegisterSpace = 0;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psSampler)].NumDescriptors = 1;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psSampler)].BaseShaderRegister = 0;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psSampler)].RegisterSpace = 0;
				spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


				D3D12_ROOT_PARAMETER1 spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::cnt)] = {};
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::vsMat)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::vsMat)].DescriptorTable.NumDescriptorRanges = 1;
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::vsMat)].DescriptorTable.pDescriptorRanges = &spotShadowRanges[static_cast<UINT>(eSpotShadowRP::vsMat)];
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::vsMat)].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].DescriptorTable.NumDescriptorRanges = 1;
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].DescriptorTable.pDescriptorRanges = &spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)];
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::psDiffuseMap)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::psSampler)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::psSampler)].DescriptorTable.NumDescriptorRanges = 1;
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::psSampler)].DescriptorTable.pDescriptorRanges = &spotShadowRanges[static_cast<UINT>(eSpotShadowRP::psSampler)];
				spotShadowRootParameter[static_cast<UINT>(eSpotShadowRP::psSampler)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



				D3D12_VERSIONED_ROOT_SIGNATURE_DESC spotShadowRootSignatureDesc = {};
				spotShadowRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				spotShadowRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eSpotShadowRP::cnt);
				spotShadowRootSignatureDesc.Desc_1_1.pParameters = spotShadowRootParameter;
				spotShadowRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				spotShadowRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				spotShadowRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&spotShadowRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pSpotShadowRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pSpotShadowRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pSpotShadowRootSignature") - 1, "Shader12::m_pSpotShadowRootSignature");

			}
			//CubeShadow
			{
				D3D12_DESCRIPTOR_RANGE1 cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::cnt)] = {};
				for (int i = 0; i < static_cast<UINT>(eCubeShadowRP::psDiffuseMap); ++i)
				{
					cubeShadowRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					cubeShadowRanges[i].NumDescriptors = 1;
					cubeShadowRanges[i].BaseShaderRegister = 0;
					cubeShadowRanges[i].RegisterSpace = 0;
					cubeShadowRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}
				
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psDiffuseMap)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psDiffuseMap)].NumDescriptors = 1;
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psDiffuseMap)].BaseShaderRegister = 0;
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psDiffuseMap)].RegisterSpace = 0;
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psDiffuseMap)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psSampler)].NumDescriptors = 1;
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psSampler)].BaseShaderRegister = 0;
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psSampler)].RegisterSpace = 0;
				cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


				D3D12_ROOT_PARAMETER1 cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::cnt)] = {};
				cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::vsMat)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::vsMat)].DescriptorTable.NumDescriptorRanges = 1;
				cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::vsMat)].DescriptorTable.pDescriptorRanges = &cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::vsMat)];
				cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::vsMat)].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::gsMat)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::gsMat)].DescriptorTable.NumDescriptorRanges = 1;
				cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::gsMat)].DescriptorTable.pDescriptorRanges = &cubeShadowRanges[static_cast<UINT>(eCubeShadowRP::gsMat)];
				cubeShadowRootParameter[static_cast<UINT>(eCubeShadowRP::gsMat)].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

				for (int i = static_cast<UINT>(eCubeShadowRP::psLightPos); i < static_cast<UINT>(eCubeShadowRP::cnt); ++i)
				{
					cubeShadowRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					cubeShadowRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					cubeShadowRootParameter[i].DescriptorTable.pDescriptorRanges = &cubeShadowRanges[i];
					cubeShadowRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				}
				
				D3D12_VERSIONED_ROOT_SIGNATURE_DESC cubeShadowRootSignatureDesc = {};
				cubeShadowRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				cubeShadowRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eCubeShadowRP::cnt);
				cubeShadowRootSignatureDesc.Desc_1_1.pParameters = cubeShadowRootParameter;
				cubeShadowRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				cubeShadowRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				cubeShadowRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&cubeShadowRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pCubeShadowRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pCubeShadowRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pCubeShadowRootSignature") - 1, "Shader12::m_pCubeShadowRootSignature");

			}
			//SkyBox
			{
				D3D12_DESCRIPTOR_RANGE1 skyboxRanges[static_cast<UINT>(eSkyboxRP::cnt)] = {};
				skyboxRanges[static_cast<UINT>(eSkyboxRP::vsMat)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::vsMat)].NumDescriptors = 1;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::vsMat)].BaseShaderRegister = 0;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::vsMat)].RegisterSpace = 0;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::vsMat)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				skyboxRanges[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].NumDescriptors = 1;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].BaseShaderRegister = 0;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].RegisterSpace = 0;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				skyboxRanges[static_cast<UINT>(eSkyboxRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::psSampler)].NumDescriptors = 1;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::psSampler)].BaseShaderRegister = 0;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::psSampler)].RegisterSpace = 0;
				skyboxRanges[static_cast<UINT>(eSkyboxRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


				D3D12_ROOT_PARAMETER1 skyboxRootParameter[static_cast<UINT>(eSkyboxRP::cnt)] = {};
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::vsMat)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::vsMat)].DescriptorTable.NumDescriptorRanges = 1;
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::vsMat)].DescriptorTable.pDescriptorRanges = &skyboxRanges[static_cast<UINT>(eSkyboxRP::vsMat)];
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::vsMat)].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].DescriptorTable.NumDescriptorRanges = 1;
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].DescriptorTable.pDescriptorRanges = &skyboxRanges[static_cast<UINT>(eSkyboxRP::psDiffuseMap)];
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::psDiffuseMap)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::psSampler)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::psSampler)].DescriptorTable.NumDescriptorRanges = 1;
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::psSampler)].DescriptorTable.pDescriptorRanges = &skyboxRanges[static_cast<UINT>(eSkyboxRP::psSampler)];
				skyboxRootParameter[static_cast<UINT>(eSkyboxRP::psSampler)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



				D3D12_VERSIONED_ROOT_SIGNATURE_DESC skyboxRootSignature = {};
				skyboxRootSignature.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				skyboxRootSignature.Desc_1_1.NumParameters = static_cast<UINT>(eSkyboxRP::cnt);
				skyboxRootSignature.Desc_1_1.pParameters = skyboxRootParameter;
				skyboxRootSignature.Desc_1_1.NumStaticSamplers = 0;
				skyboxRootSignature.Desc_1_1.pStaticSamplers = nullptr;
				skyboxRootSignature.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&skyboxRootSignature, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pSkyboxRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pSkyboxRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pSkyboxRootSignature") - 1, "Shader12::m_pSkyboxRootSignature");

			}
			//PBR Geometry
			{
				D3D12_DESCRIPTOR_RANGE1 PbrGeometryRanges[static_cast<UINT>(ePbrGeoRP::cnt)] = {};
				for (int i = static_cast<UINT>(ePbrGeoRP::vsMat); i < static_cast<UINT>(ePbrGeoRP::psDiffuse); ++i)
				{
					PbrGeometryRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					PbrGeometryRanges[i].NumDescriptors = 1;
					PbrGeometryRanges[i].BaseShaderRegister = i;
					PbrGeometryRanges[i].RegisterSpace = 0;
					PbrGeometryRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}

				for (int i = static_cast<UINT>(ePbrGeoRP::psDiffuse); i < static_cast<UINT>(ePbrGeoRP::psSampler); ++i)
				{
					PbrGeometryRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					PbrGeometryRanges[i].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
					PbrGeometryRanges[i].NumDescriptors = 1;
					PbrGeometryRanges[i].BaseShaderRegister = i - static_cast<UINT>(ePbrGeoRP::psDiffuse);
					PbrGeometryRanges[i].RegisterSpace = 0;
					PbrGeometryRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}

				PbrGeometryRanges[static_cast<UINT>(ePbrGeoRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				PbrGeometryRanges[static_cast<UINT>(ePbrGeoRP::psSampler)].NumDescriptors = 1;
				PbrGeometryRanges[static_cast<UINT>(ePbrGeoRP::psSampler)].BaseShaderRegister = 0;
				PbrGeometryRanges[static_cast<UINT>(ePbrGeoRP::psSampler)].RegisterSpace = 0;
				PbrGeometryRanges[static_cast<UINT>(ePbrGeoRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				for (int i = static_cast<UINT>(ePbrGeoRP::psCamPos); i < static_cast<UINT>(ePbrGeoRP::cnt); ++i)
				{
					PbrGeometryRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					PbrGeometryRanges[i].NumDescriptors = 1;
					PbrGeometryRanges[i].BaseShaderRegister = i - static_cast<UINT>(ePbrGeoRP::psCamPos);
					PbrGeometryRanges[i].RegisterSpace = 0;
					PbrGeometryRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}



				D3D12_ROOT_PARAMETER1 PbrGeometryRootParameter[static_cast<UINT>(ePbrGeoRP::cnt)] = {};
				for (int i = static_cast<UINT>(ePbrGeoRP::vsMat); i < static_cast<UINT>(ePbrGeoRP::psDiffuse); ++i)
				{
					PbrGeometryRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					PbrGeometryRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					PbrGeometryRootParameter[i].DescriptorTable.pDescriptorRanges = &PbrGeometryRanges[i];
					PbrGeometryRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
				}

				for (int i = static_cast<UINT>(ePbrGeoRP::psDiffuse); i < static_cast<UINT>(ePbrGeoRP::cnt); ++i)
				{
					PbrGeometryRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					PbrGeometryRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					PbrGeometryRootParameter[i].DescriptorTable.pDescriptorRanges = &PbrGeometryRanges[i];
					PbrGeometryRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				}


				D3D12_VERSIONED_ROOT_SIGNATURE_DESC PbrGeometryRootSignatureDesc = {};
				PbrGeometryRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				PbrGeometryRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(ePbrGeoRP::cnt);
				PbrGeometryRootSignatureDesc.Desc_1_1.pParameters = PbrGeometryRootParameter;
				PbrGeometryRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				PbrGeometryRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				PbrGeometryRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&PbrGeometryRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));

				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pPbrGeoRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pPbrGeoRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pPbrGeoRootSignature") - 1, "Shader12::m_pPbrGeoRootSignature");
			}
			//SSAO 
			{
				D3D12_DESCRIPTOR_RANGE1 ssaoRanges[static_cast<UINT>(eSsaoRP::cnt)] = {};

				for (int i = static_cast<UINT>(eSsaoRP::csNoise); i < static_cast<UINT>(eSsaoRP::csUAV); ++i)
				{
					ssaoRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					ssaoRanges[i].NumDescriptors = 1;
					ssaoRanges[i].BaseShaderRegister = i;
					ssaoRanges[i].RegisterSpace = 0;
					ssaoRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				}

				ssaoRanges[static_cast<UINT>(eSsaoRP::csUAV)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				ssaoRanges[static_cast<UINT>(eSsaoRP::csUAV)].NumDescriptors = 1;
				ssaoRanges[static_cast<UINT>(eSsaoRP::csUAV)].BaseShaderRegister = 0;
				ssaoRanges[static_cast<UINT>(eSsaoRP::csUAV)].RegisterSpace = 0;
				ssaoRanges[static_cast<UINT>(eSsaoRP::csUAV)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				for (int i = static_cast<UINT>(eSsaoRP::csWrap); i < static_cast<UINT>(eSsaoRP::csSamplePoints); ++i)
				{
					ssaoRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					ssaoRanges[i].NumDescriptors = 1;
					ssaoRanges[i].BaseShaderRegister = i - static_cast<UINT>(eSsaoRP::csWrap);
					ssaoRanges[i].RegisterSpace = 0;
					ssaoRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}

				for (int i = static_cast<UINT>(eSsaoRP::csSamplePoints); i < static_cast<UINT>(eSsaoRP::cnt); ++i)
				{
					ssaoRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					ssaoRanges[i].NumDescriptors = 1;
					ssaoRanges[i].BaseShaderRegister = i - static_cast<UINT>(eSsaoRP::csSamplePoints);
					ssaoRanges[i].RegisterSpace = 0;
					ssaoRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}

				D3D12_ROOT_PARAMETER1 ssaoRootParameter[static_cast<UINT>(eSsaoRP::cnt)] = {};
				for (int i = static_cast<UINT>(eSsaoRP::csNoise); i < static_cast<UINT>(eSsaoRP::cnt); ++i)
				{
					ssaoRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					ssaoRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					ssaoRootParameter[i].DescriptorTable.pDescriptorRanges = &ssaoRanges[i];
					ssaoRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				}


				D3D12_VERSIONED_ROOT_SIGNATURE_DESC ssaoRootSignatureDesc = {};
				ssaoRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				ssaoRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eSsaoRP::cnt);
				ssaoRootSignatureDesc.Desc_1_1.pParameters = ssaoRootParameter;
				ssaoRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				ssaoRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				ssaoRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&ssaoRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pSsaoRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pSsaoRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pSsaoRootSignature") - 1, "Shader12::m_pSsaoRootSignature");

			}
			//SSAOBlur
			{
				D3D12_DESCRIPTOR_RANGE1 ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::cnt)] = {};

				for (int i = 0; i < static_cast<UINT>(eSsaoBlurRP::csSsao); ++i)
				{
					ssaoBlurRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					ssaoBlurRanges[i].NumDescriptors = 1;
					ssaoBlurRanges[i].BaseShaderRegister = i;
					ssaoBlurRanges[i].RegisterSpace = 0;
					ssaoBlurRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}

				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csSsao)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csSsao)].NumDescriptors = 1;
				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csSsao)].BaseShaderRegister = 0;
				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csSsao)].RegisterSpace = 0;
				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csSsao)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csWrap)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csWrap)].NumDescriptors = 1;
				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csWrap)].BaseShaderRegister = 0;
				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csWrap)].RegisterSpace = 0;
				ssaoBlurRanges[static_cast<UINT>(eSsaoBlurRP::csWrap)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


				D3D12_ROOT_PARAMETER1 ssaoBlurRootParameter[static_cast<UINT>(eSsaoBlurRP::cnt)] = {};

				for (int i = 0; i < static_cast<UINT>(eSsaoBlurRP::cnt); ++i)
				{
					ssaoBlurRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					ssaoBlurRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					ssaoBlurRootParameter[i].DescriptorTable.pDescriptorRanges = &ssaoBlurRanges[i];
					ssaoBlurRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				}


				D3D12_VERSIONED_ROOT_SIGNATURE_DESC ssaoBlurRootSignatureDesc = {};
				ssaoBlurRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				ssaoBlurRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eSsaoBlurRP::cnt);
				ssaoBlurRootSignatureDesc.Desc_1_1.pParameters = ssaoBlurRootParameter;
				ssaoBlurRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				ssaoBlurRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				ssaoBlurRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&ssaoBlurRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pSsaoBlurRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pSsaoBlurRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pSsaoBlurRootSignature") - 1, "Shader12::m_pSsaoBlurRootSignature");

			}
			//PbrLight
			{
				D3D12_DESCRIPTOR_RANGE1 PbrLightRanges[static_cast<UINT>(ePbrLightRP::cnt)] = {};
				for (int i = static_cast<UINT>(ePbrLightRP::psPos); i < static_cast<UINT>(ePbrLightRP::psWrap); ++i)
				{
					PbrLightRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					PbrLightRanges[i].NumDescriptors = 1;
					PbrLightRanges[i].BaseShaderRegister = i;
					PbrLightRanges[i].RegisterSpace = 0;
					PbrLightRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}

				for (int i = static_cast<UINT>(ePbrLightRP::psWrap); i < static_cast<UINT>(ePbrLightRP::psCamPos); ++i)
				{
					PbrLightRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					PbrLightRanges[i].NumDescriptors = 1;
					PbrLightRanges[i].BaseShaderRegister = i - static_cast<UINT>(ePbrLightRP::psWrap);
					PbrLightRanges[i].RegisterSpace = 0;
					PbrLightRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}

				for (int i = static_cast<UINT>(ePbrLightRP::psCamPos); i < static_cast<UINT>(ePbrLightRP::cnt); ++i)
				{
					PbrLightRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					PbrLightRanges[i].NumDescriptors = 1;
					PbrLightRanges[i].BaseShaderRegister = i - static_cast<UINT>(ePbrLightRP::psCamPos);
					PbrLightRanges[i].RegisterSpace = 0;
					PbrLightRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				}


				D3D12_ROOT_PARAMETER1 PbrLightRootParameter[static_cast<UINT>(ePbrLightRP::cnt)] = {};
				for (int i = static_cast<UINT>(ePbrLightRP::psPos); i < static_cast<UINT>(ePbrLightRP::cnt); ++i)
				{
					PbrLightRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					PbrLightRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					PbrLightRootParameter[i].DescriptorTable.pDescriptorRanges = &PbrLightRanges[i];
					PbrLightRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				}

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC PbrLightRootSignatureDesc = {};
				PbrLightRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				PbrLightRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(ePbrLightRP::cnt);
				PbrLightRootSignatureDesc.Desc_1_1.pParameters = PbrLightRootParameter;
				PbrLightRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				PbrLightRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				PbrLightRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&PbrLightRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pPbrLightRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pPbrLightRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pPbrLightRootSignature") - 1, "Shader12::m_pPbrLightRootSignature");

			}
			//AABB
			{
				D3D12_DESCRIPTOR_RANGE1 aabbRanges[1] = {};
				aabbRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				aabbRanges[0].NumDescriptors = 1;
				aabbRanges[0].BaseShaderRegister = 0;
				aabbRanges[0].RegisterSpace = 0;
				aabbRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				D3D12_ROOT_PARAMETER1 aabbRootParameter[1] = {};
				aabbRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				aabbRootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
				aabbRootParameter[0].DescriptorTable.pDescriptorRanges = &aabbRanges[0];
				aabbRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC aabbRootSignatureDesc = {};
				aabbRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				aabbRootSignatureDesc.Desc_1_1.NumParameters = 1;
				aabbRootSignatureDesc.Desc_1_1.pParameters = aabbRootParameter;
				aabbRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				aabbRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				aabbRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&aabbRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pAabbRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pAabbRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pAabbRootSignature") - 1, "Shader12::m_pAabbRootSignature");


			}
			//OutlinerTest
			{
				D3D12_DESCRIPTOR_RANGE1 OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::cnt)] = {};
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::vsMat)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::vsMat)].NumDescriptors = 1;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::vsMat)].BaseShaderRegister = 0;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::vsMat)].RegisterSpace = 0;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::vsMat)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psAlbedo)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psAlbedo)].NumDescriptors = 1;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psAlbedo)].BaseShaderRegister = 0;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psAlbedo)].RegisterSpace = 0;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psAlbedo)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psWrap)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psWrap)].NumDescriptors = 1;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psWrap)].BaseShaderRegister = 0;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psWrap)].RegisterSpace = 0;
				OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psWrap)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER1 OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::cnt)] = {};
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::vsMat)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::vsMat)].DescriptorTable.NumDescriptorRanges = 1;
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::vsMat)].DescriptorTable.pDescriptorRanges = &OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::vsMat)];
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::vsMat)].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::psAlbedo)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::psAlbedo)].DescriptorTable.NumDescriptorRanges = 1;
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::psAlbedo)].DescriptorTable.pDescriptorRanges = &OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psAlbedo)];
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::psAlbedo)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::psWrap)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::psWrap)].DescriptorTable.NumDescriptorRanges = 1;
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::psWrap)].DescriptorTable.pDescriptorRanges = &OutlinerTestRanges[static_cast<UINT>(eOutlinerRP::psWrap)];
				OutlinerTestRootParameter[static_cast<UINT>(eOutlinerRP::psWrap)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC outlinerTestRootSignatureDesc = {};
				outlinerTestRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				outlinerTestRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eOutlinerRP::cnt);
				outlinerTestRootSignatureDesc.Desc_1_1.pParameters = OutlinerTestRootParameter;
				outlinerTestRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				outlinerTestRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				outlinerTestRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&outlinerTestRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pOutlinerTestRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pOutlinerTestRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pOutlinerTestRootSignature") - 1, "Shader12::m_pOutlinerTestRootSignature");

			}
			//PostProcess
			{
				D3D12_DESCRIPTOR_RANGE1 PostProcessRanges[static_cast<UINT>(ePostProcessRP::cnt)] = {};

				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csTex)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csTex)].NumDescriptors = 1;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csTex)].BaseShaderRegister = 0;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csTex)].RegisterSpace = 0;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csTex)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csUav)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csUav)].NumDescriptors = 1;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csUav)].BaseShaderRegister = 0;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csUav)].RegisterSpace = 0;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csUav)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csSampler)].NumDescriptors = 1;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csSampler)].BaseShaderRegister = 0;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csSampler)].RegisterSpace = 0;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csExposure)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csExposure)].NumDescriptors = 1;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csExposure)].BaseShaderRegister = 0;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csExposure)].RegisterSpace = 0;
				PostProcessRanges[static_cast<UINT>(ePostProcessRP::csExposure)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;



				D3D12_ROOT_PARAMETER1 PostProcessRootParameter[static_cast<UINT>(ePostProcessRP::cnt)] = {};

				for (int i = 0; i < static_cast<UINT>(ePostProcessRP::cnt); ++i)
				{
					PostProcessRootParameter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					PostProcessRootParameter[i].DescriptorTable.NumDescriptorRanges = 1;
					PostProcessRootParameter[i].DescriptorTable.pDescriptorRanges = &PostProcessRanges[i];
					PostProcessRootParameter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				}


				D3D12_VERSIONED_ROOT_SIGNATURE_DESC PostProcessRootSignatureDesc = {};
				PostProcessRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				PostProcessRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(ePostProcessRP::cnt);
				PostProcessRootSignatureDesc.Desc_1_1.pParameters = PostProcessRootParameter;
				PostProcessRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				PostProcessRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				PostProcessRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&PostProcessRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pPostProcessRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pPostProcessRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pPostProcessRootSignature") - 1, "Shader12::m_pPostProcessRootSignature");

			}
			//BRDF empty
			{
				D3D12_VERSIONED_ROOT_SIGNATURE_DESC emptyRootSignatureDesc = {};
				emptyRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				emptyRootSignatureDesc.Desc_1_1.NumParameters = 0;
				emptyRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&emptyRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pBrdfRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pBrdfRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pBrdfRootSignature") - 1, "Shader12::m_pBrdfRootSignature");
			}
			//DiffuseIrradiance srv, ss
			{
				D3D12_DESCRIPTOR_RANGE1 diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::cnt)] = {};
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].NumDescriptors = 1;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].BaseShaderRegister = 0;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].RegisterSpace = 0;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].NumDescriptors = 1;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].BaseShaderRegister = 0;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].RegisterSpace = 0;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].NumDescriptors = 1;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].BaseShaderRegister = 0;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].RegisterSpace = 0;
				diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER1 diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::cnt)] = {};

				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].DescriptorTable.NumDescriptorRanges = 1;
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].DescriptorTable.pDescriptorRanges = &diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)];
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::gsCb)].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].DescriptorTable.NumDescriptorRanges = 1;
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].DescriptorTable.pDescriptorRanges = &diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)];
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::psTex)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].DescriptorTable.NumDescriptorRanges = 1;
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].DescriptorTable.pDescriptorRanges = &diffuseIrradianceRanges[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)];
				diffuseIrradianceRootParameter[static_cast<UINT>(eDiffuseIrraidianceRP::psSampler)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC diffuseIrradianceRootSignature = {};
				diffuseIrradianceRootSignature.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				diffuseIrradianceRootSignature.Desc_1_1.NumParameters = static_cast<UINT>(eDiffuseIrraidianceRP::cnt);
				diffuseIrradianceRootSignature.Desc_1_1.pParameters = diffuseIrradianceRootParameter;
				diffuseIrradianceRootSignature.Desc_1_1.NumStaticSamplers = 0;
				diffuseIrradianceRootSignature.Desc_1_1.pStaticSamplers = nullptr;
				diffuseIrradianceRootSignature.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&diffuseIrradianceRootSignature, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pDiffuseIrradianceRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pDiffuseIrradianceRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pDiffuseIrradianceRootSignature") - 1, "Shader12::m_pDiffuseIrradianceRootSignature");


			}
			//Prefilter 
			{
				D3D12_DESCRIPTOR_RANGE1 prefilterRanges[static_cast<UINT>(ePrefilterRP::cnt)] = {};
				prefilterRanges[static_cast<UINT>(ePrefilterRP::gsCb)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::gsCb)].NumDescriptors = 1;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::gsCb)].BaseShaderRegister = 0;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::gsCb)].RegisterSpace = 0;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::gsCb)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				prefilterRanges[static_cast<UINT>(ePrefilterRP::psTex)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psTex)].NumDescriptors = 1;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psTex)].BaseShaderRegister = 0;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psTex)].RegisterSpace = 0;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psTex)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				prefilterRanges[static_cast<UINT>(ePrefilterRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psSampler)].NumDescriptors = 1;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psSampler)].BaseShaderRegister = 0;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psSampler)].RegisterSpace = 0;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				prefilterRanges[static_cast<UINT>(ePrefilterRP::psCb)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psCb)].NumDescriptors = 1;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psCb)].BaseShaderRegister = 0;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psCb)].RegisterSpace = 0;
				prefilterRanges[static_cast<UINT>(ePrefilterRP::psCb)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER1 prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::cnt)] = {};

				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::gsCb)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::gsCb)].DescriptorTable.NumDescriptorRanges = 1;
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::gsCb)].DescriptorTable.pDescriptorRanges = &prefilterRanges[static_cast<UINT>(ePrefilterRP::gsCb)];
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::gsCb)].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psTex)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psTex)].DescriptorTable.NumDescriptorRanges = 1;
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psTex)].DescriptorTable.pDescriptorRanges = &prefilterRanges[static_cast<UINT>(ePrefilterRP::psTex)];
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psTex)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psSampler)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psSampler)].DescriptorTable.NumDescriptorRanges = 1;
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psSampler)].DescriptorTable.pDescriptorRanges = &prefilterRanges[static_cast<UINT>(ePrefilterRP::psSampler)];
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psSampler)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psCb)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psCb)].DescriptorTable.NumDescriptorRanges = 1;
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psCb)].DescriptorTable.pDescriptorRanges = &prefilterRanges[static_cast<UINT>(ePrefilterRP::psCb)];
				prefilterRootParmeter[static_cast<UINT>(ePrefilterRP::psCb)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC prefilterRootSignatureDesc = {};
				prefilterRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				prefilterRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(ePrefilterRP::cnt);
				prefilterRootSignatureDesc.Desc_1_1.pParameters = prefilterRootParmeter;
				prefilterRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				prefilterRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				prefilterRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&prefilterRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pPrefilterRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pPrefilterRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pPrefilterRootSignature") - 1, "Shader12::m_pPrefilterRootSignature");
			}
			//Equirect2Cube GS:cb PS:srv,ss
			{
				D3D12_DESCRIPTOR_RANGE1 equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::cnt)] = {};
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::gsCb)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::gsCb)].NumDescriptors = 1;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::gsCb)].BaseShaderRegister = 0;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::gsCb)].RegisterSpace = 0;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::gsCb)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psTex)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psTex)].NumDescriptors = 1;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psTex)].BaseShaderRegister = 0;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psTex)].RegisterSpace = 0;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psTex)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psSampler)].NumDescriptors = 1;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psSampler)].BaseShaderRegister = 0;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psSampler)].RegisterSpace = 0;
				equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER1 equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::cnt)] = {};
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::gsCb)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::gsCb)].DescriptorTable.NumDescriptorRanges = 1;
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::gsCb)].DescriptorTable.pDescriptorRanges = &equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::gsCb)];
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::gsCb)].ShaderVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;

				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::psTex)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::psTex)].DescriptorTable.NumDescriptorRanges = 1;
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::psTex)].DescriptorTable.pDescriptorRanges = &equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psTex)];
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::psTex)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::psSampler)].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::psSampler)].DescriptorTable.NumDescriptorRanges = 1;
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::psSampler)].DescriptorTable.pDescriptorRanges = &equirect2CubeRanges[static_cast<UINT>(eEquirect2CubeRP::psSampler)];
				equirect2CubeRootParmeter[static_cast<UINT>(eEquirect2CubeRP::psSampler)].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC equirect2CubeRootSignatureDesc = {};
				equirect2CubeRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				equirect2CubeRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eEquirect2CubeRP::cnt);
				equirect2CubeRootSignatureDesc.Desc_1_1.pParameters = equirect2CubeRootParmeter;
				equirect2CubeRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				equirect2CubeRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				equirect2CubeRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&equirect2CubeRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pEquirect2cubeRootSignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pEquirect2cubeRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12::m_pEquirect2cubeRootSignature") - 1, "Shader12::m_pEquirect2cubeRootSignature");

			}
			//GenMip
			{
				D3D12_DESCRIPTOR_RANGE1 genMipRanges[static_cast<UINT>(eGenMipRP::cnt)] = {};
				genMipRanges[static_cast<UINT>(eGenMipRP::csTex)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				genMipRanges[static_cast<UINT>(eGenMipRP::csTex)].NumDescriptors = 1;
				genMipRanges[static_cast<UINT>(eGenMipRP::csTex)].BaseShaderRegister = 0;
				genMipRanges[static_cast<UINT>(eGenMipRP::csTex)].RegisterSpace = 0;
				genMipRanges[static_cast<UINT>(eGenMipRP::csTex)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				genMipRanges[static_cast<UINT>(eGenMipRP::csUAV)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				genMipRanges[static_cast<UINT>(eGenMipRP::csUAV)].NumDescriptors = 1;
				genMipRanges[static_cast<UINT>(eGenMipRP::csUAV)].BaseShaderRegister = 0;
				genMipRanges[static_cast<UINT>(eGenMipRP::csUAV)].RegisterSpace = 0;
				genMipRanges[static_cast<UINT>(eGenMipRP::csUAV)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				genMipRanges[static_cast<UINT>(eGenMipRP::csSampler)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				genMipRanges[static_cast<UINT>(eGenMipRP::csSampler)].NumDescriptors = 1;
				genMipRanges[static_cast<UINT>(eGenMipRP::csSampler)].BaseShaderRegister = 0;
				genMipRanges[static_cast<UINT>(eGenMipRP::csSampler)].RegisterSpace = 0;
				genMipRanges[static_cast<UINT>(eGenMipRP::csSampler)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				genMipRanges[static_cast<UINT>(eGenMipRP::csCb)].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				genMipRanges[static_cast<UINT>(eGenMipRP::csCb)].NumDescriptors = 1;
				genMipRanges[static_cast<UINT>(eGenMipRP::csCb)].BaseShaderRegister = 0;
				genMipRanges[static_cast<UINT>(eGenMipRP::csCb)].RegisterSpace = 0;
				genMipRanges[static_cast<UINT>(eGenMipRP::csCb)].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


				D3D12_ROOT_PARAMETER1 genMipRootParmeter[static_cast<UINT>(eGenMipRP::cnt)] = {};
				for (int i = static_cast<UINT>(eGenMipRP::csTex); i < static_cast<UINT>(eGenMipRP::cnt); ++i)
				{
					genMipRootParmeter[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					genMipRootParmeter[i].DescriptorTable.NumDescriptorRanges = 1;
					genMipRootParmeter[i].DescriptorTable.pDescriptorRanges = &genMipRanges[i];
					genMipRootParmeter[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				}

				D3D12_VERSIONED_ROOT_SIGNATURE_DESC genMipRootSignatureDesc = {};
				genMipRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
				genMipRootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(eGenMipRP::cnt);
				genMipRootSignatureDesc.Desc_1_1.pParameters = genMipRootParmeter;
				genMipRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
				genMipRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
				genMipRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

				ID3DBlob* signature;
				ID3DBlob* error;
				hr = D3D12SerializeVersionedRootSignature(&genMipRootSignatureDesc, &signature, &error);
				assert(SUCCEEDED(hr));
				hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pGenMipMapRootsignature.GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pGenMipMapRootsignature->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("Shader12:m_pGenMipMapRootsignature") - 1, "Shader12::m_pGenMipMapRootsignature");

			}
		}

	}

	Shader12::~Shader12()
	{
	}
}