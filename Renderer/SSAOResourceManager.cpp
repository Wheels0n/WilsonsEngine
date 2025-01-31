
#include "HeapManager.h"
#include "ResourceBarrierHandler.h"
#include "D3D12.h"
#include "TextureManager.h"
#include "SSAOResourceManager.h"
namespace wilson
{
	void SSAOResourceManager::SetBeginSceneBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSsaoTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSsaoBlurTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSsaoBlurDebugTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	}

	void SSAOResourceManager::SetSAAOBlurPassBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSsaoTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSsaoBlurDebugTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	}

	void SSAOResourceManager::SetLightPassBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSsaoBlurTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}

	SSAOResourceManager::SSAOResourceManager(UINT width, UINT height, ComPtr<ID3D12GraphicsCommandList> pCmdList)
		:m_nSsaoSample(_NUM_SSAO_SAMPLE), m_ssaoBias(_SSAO_BIAS), m_ssaoRadius(_SSAO_RADIUS)
	{
		Init(width, height);
		//GenSSAOResources
		{
			m_matrixCbKey = g_pHeapManager->AllocateCb(sizeof(XMMATRIX));
			m_ssaoKernalCbKey = g_pHeapManager->AllocateCb(sizeof(SamplePoints));
			m_ssaoParametersCbKey = g_pHeapManager->AllocateCb(sizeof(FLOAT) + sizeof(UINT) * 2);

			//Gen Sample points
			SamplePoints kernels;
			std::uniform_real_distribution<float> randomFloats(0.0, 1.0f);
			std::default_random_engine gen;
			for (int i = 0; i < _KERNEL_COUNT; ++i)
			{
				XMFLOAT3 sample(
					randomFloats(gen) * 2.0f - 1.0f,
					randomFloats(gen) * 2.0f - 1.0f,
					randomFloats(gen));
				XMVECTOR sampleV = XMLoadFloat3(&sample);
				sampleV = XMVector3Normalize(sampleV);
				sampleV = XMVectorScale(sampleV, randomFloats(gen));
				float scale = i / static_cast<float>(_KERNEL_COUNT);
				scale = 0.1f + (1.0f - 0.1f) * scale * scale;
				sampleV = XMVectorScale(sampleV, scale);

				kernels.coord[i] = sampleV;
			}
			g_pHeapManager->CopyDataToCb(m_ssaoKernalCbKey, sizeof(kernels), &kernels);


			XMFLOAT4 noiseVecs[_NOISE_VEC_COUNT];
			//Gen noise texture
			for (int i = 0; i < _NOISE_VEC_COUNT; ++i)
			{
				XMFLOAT4 rot(
					randomFloats(gen) * 2.0f - 1.0f,
					randomFloats(gen) * 2.0f - 1.0f,
					0,0);
				noiseVecs[i] = rot;
			}

			m_pNoiseTex = g_pTextureManager->CreateTexture2D(_NOISE_TEX_SIZE, _NOISE_TEX_SIZE, 1, 1, (BYTE)0b010,
				DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			SET_PRIVATE_DATA(m_pNoiseTex->tex);
			g_pTextureManager->UploadTexThroughCB(m_pNoiseTex->tex->GetDesc(), sizeof(XMVECTOR) * _NOISE_TEX_SIZE, reinterpret_cast<UINT8*>(noiseVecs),
				m_pNoiseTex->tex, m_pNoiseTex->cb, pCmdList);

			g_pResourceBarrierHandler->Push(
				ResourceBarrierHandler::CreateResourceBarrier(m_pNoiseTex->tex,
					D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
			g_pResourceBarrierHandler->Flush(pCmdList);
		
			UpdateSSAOParemeters();
		}


	}

	void SSAOResourceManager::UploadSSAO(bool bUAV, ComPtr<ID3D12GraphicsCommandList> pCmdlist)
	{
		if (bUAV)
		{
			g_pTextureManager->UploadComputeUav(pCmdlist, m_pSsaoTex, E_TO_UINT(eSsaoRP::csUAV));
		}
		else
		{
			g_pTextureManager->UploadComputeSrv(pCmdlist, m_pSsaoTex, E_TO_UINT(eSsaoBlurRP::csSsao));
		}
	}


	void SSAOResourceManager::DrawSSAOBlurredImage()
	{
		g_pTextureManager->SetImGuiImage(m_pSsaoBlurDebugTex, _VIEWPORT_WIDTH, _VIEWPORT_HEIGHT);
	}

	void SSAOResourceManager::Init(UINT width, UINT height)
	{
		m_pSsaoTex = g_pTextureManager->CreateTexture2D(width / 2, height / 2, 1, 1, (BYTE)0b0011,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		SET_PRIVATE_DATA(m_pSsaoTex->tex);

		m_pSsaoBlurTex = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b0011,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		SET_PRIVATE_DATA(m_pSsaoBlurTex->tex);

		m_pSsaoBlurDebugTex = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b0011,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		SET_PRIVATE_DATA(m_pSsaoBlurDebugTex->tex);
	}

	void SSAOResourceManager::UpdateSSAOParemeters()
	{
		//SSAO Pass
		g_pHeapManager->CopyDataToCb(m_ssaoParametersCbKey, sizeof(FLOAT), &m_ssaoRadius);
		g_pHeapManager->CopyDataToCbWithOffset(m_ssaoParametersCbKey, sizeof(FLOAT), sizeof(FLOAT), &m_ssaoBias);
		g_pHeapManager->CopyDataToCbWithOffset(m_ssaoParametersCbKey, sizeof(UINT), sizeof(FLOAT) * 2, &m_nSsaoSample);
	}

	void SSAOResourceManager::UpdateProjMat(XMMATRIX* pProj)
	{
		g_pHeapManager->CopyDataToCb(m_matrixCbKey, sizeof(XMMATRIX), pProj);
	}

	void SSAOResourceManager::SetLightingPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		g_pTextureManager->UploadGraphicsSrv(pCmdList, m_pSsaoBlurTex, E_TO_UINT(ePbrLightRP::psAO));
	}

	void SSAOResourceManager::SetSSAOPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		UploadSSAO(true, pCmdList);

		g_pTextureManager	->UploadComputeSrv(pCmdList, m_pNoiseTex, E_TO_UINT(eSsaoRP::csNoise));
		g_pHeapManager		->UploadComputeCb(m_ssaoKernalCbKey, E_TO_UINT(eSsaoRP::csSamplePoints), pCmdList);
		g_pHeapManager		->UploadComputeCb(m_ssaoParametersCbKey, E_TO_UINT(eSsaoRP::csParameters), pCmdList);
		g_pHeapManager		->UploadComputeCb(m_matrixCbKey, E_TO_UINT(eSsaoRP::csProj), pCmdList);
	}

	void SSAOResourceManager::SetSSAOBlurPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		UploadSSAO(false, pCmdList);

		g_pTextureManager->UploadComputeUav(pCmdList, m_pSsaoBlurTex, E_TO_UINT(eSsaoBlurRP::csDst));
		g_pTextureManager->UploadComputeUav(pCmdList, m_pSsaoBlurDebugTex, E_TO_UINT(eSsaoBlurRP::csDebug));
	}

	SSAOResourceManager::~SSAOResourceManager()
	{

		if (m_pSsaoTex != nullptr)
		{
			m_pSsaoTex = nullptr;
		}
		

		if (m_pSsaoBlurTex != nullptr)
		{
			m_pSsaoBlurTex = nullptr;
		}

		if (m_pSsaoBlurDebugTex != nullptr)
		{
			m_pSsaoBlurDebugTex = nullptr;
		}
	}
}
