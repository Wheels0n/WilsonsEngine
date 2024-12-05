
#include "D3D12.h"
#include "stb_image.h"
#include "../ImGui/imgui_impl_dx12.h"

namespace wilson
{	

	D3D12* D3D12::g_pD3D12 = nullptr;
	void D3D12::WorkerThread(const UINT threadIndex)
	{

		while (true)
		{
			//Zpass
			WaitForSingleObject(m_workerBeginFrame[threadIndex], INFINITE);
			DrawObject(ePass::zPass, threadIndex, -1);
			SetEvent(m_workerFinishZpass[threadIndex]);

			//ShadowPass
			WaitForSingleObject(m_workerBeginShadowPass[threadIndex], INFINITE);
			std::vector<std::unique_ptr<DirectionalLight12>>& dirLights = m_pLightCb->GetDirLights();
			std::vector<std::unique_ptr<CubeLight12>>& CubeLights = m_pLightCb->GetCubeLights();
			std::vector<std::unique_ptr<SpotLight12>>& spotLights = m_pLightCb->GetSpotLights();
			UINT nLights[3] = { dirLights.size(), CubeLights.size(), spotLights.size() };
			UINT nLightTotal = nLights[static_cast<UINT>(eLIGHT_TYPE::DIR)]
				+ nLights[static_cast<UINT>(eLIGHT_TYPE::CUBE)]
				+ nLights[static_cast<UINT>(eLIGHT_TYPE::SPT)];
			if (nLightTotal)
			{
				for (int i = 0; i < dirLights.size(); ++i)
				{
					DrawObject(ePass::cascadeDirShadowPass, threadIndex, i);
				}

				
				for (int i = 0; i < spotLights.size(); ++i)
				{
					DrawObject(ePass::spotShadowPass, threadIndex, i);
				}
				
				for (int i = 0; i <CubeLights.size(); ++i)
				{
					DrawObject(ePass::cubeShadowPass, threadIndex, i);
				}

			}
			SetEvent(m_workerFinishShadowPass[threadIndex]);

			//HiZpass
			WaitForSingleObject(m_workerBeginHiZpass[threadIndex], INFINITE);
			HiZCull(threadIndex);
			SetEvent(m_workerFinishHiZpass[threadIndex]);

			//HWOcclusionTestPass
			WaitForSingleObject(m_workerBeginHWOcclusionTestPass[threadIndex], INFINITE);
			m_nHWOcclusionPassed[threadIndex] = 0;
			HWQueryForOcclusion(threadIndex);
			SetEvent(m_workerFinishHWOcclusionTestPass[threadIndex]);
			

			//PbrGeoPass
			WaitForSingleObject(m_workerBeginDeferredGeoPass[threadIndex], INFINITE);
			DrawObject(ePass::geoPass, threadIndex, -1);
			SetEvent(m_workerEndFrame[threadIndex]);
		}
		return;
	}
	UINT __stdcall D3D12::WrapperWorkerThreadFun(LPVOID pParameter)
	{	
		UINT threadIndex = *(reinterpret_cast<UINT*>(pParameter));
		g_pD3D12->WorkerThread(threadIndex);
		return 0;
	}
	void D3D12::SsaoThread()
	{
		while (true)
		{	
			WaitForSingleObject(m_ssaoBeginFrame, INFINITE);

			ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };

			m_pSsaoCommandList->SetPipelineState(m_pSsaoPso.Get());
			m_pSsaoCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pSsaoCommandList->SetComputeRootSignature(m_pShader->GetSsaoShaderRootSingnature());
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csNoise), m_noiseSrv);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csVpos), m_GBufSrvs[static_cast<UINT>(eGbuf::vPos)]);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csVnomral), m_GBufSrvs[static_cast<UINT>(eGbuf::vNormal)]);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csUAV), m_ssaoUav);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csWrap), m_wrapSsv);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csClamp), m_clampSsv);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csSamplePoints), m_ssaoKernelCbv);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoRP::csParameters), m_ssaoParameterCbv);
			m_pMatricesCb->UploadProjMat(m_pSsaoCommandList.Get(), true);
			//8º¸´Ù ³·À» °æ¿ì °¨¾È
			m_pSsaoCommandList->Dispatch(ceil(m_clientWidth / static_cast<float>(16)), ceil(m_clientHeight / static_cast<float>(16)), 1);

			//Blur SSAOTex
			D3D12_RESOURCE_BARRIER blurBarriers[] =
			{
				CreateResourceBarrier(m_pSsaoTex.Get(),
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
				CreateResourceBarrier(m_pSsaoBlurDebugTex.Get(),
					 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			};

			m_pSsaoCommandList->ResourceBarrier(2, blurBarriers);
			m_pSsaoCommandList->SetPipelineState(m_pSsaoBlurPso.Get());
			m_pSsaoCommandList->SetComputeRootSignature(m_pShader->GetSsaoBlurShaderRootSingnature());
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoBlurRP::csDst), m_ssaoBlurUav);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoBlurRP::csDebug), m_ssaoBlurDebugUav);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoBlurRP::csSsao), m_ssaoSrv);
			m_pSsaoCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eSsaoBlurRP::csWrap), m_wrapSsv);
			m_pSsaoCommandList->Dispatch(ceil(m_clientWidth / (float)8), ceil(m_clientHeight / (float)8), 1);

			
			m_pSsaoCommandList->Close();
			m_pComputeCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pSsaoCommandList.GetAddressOf()));
			
			UINT fenceValue = m_ssaoFenceValue++;
			m_pComputeCommandQueue->Signal(m_pSsaoFence.Get(), fenceValue);
			if (m_pSsaoFence->GetCompletedValue() < fenceValue)
			{
				m_pSsaoFence->SetEventOnCompletion(fenceValue, m_ssaoFenceEvent);
				WaitForSingleObject(m_ssaoFenceEvent, INFINITE);
			}
			m_pSsaoCommandAllocator->Reset();
			m_pSsaoCommandList->Reset(m_pSsaoCommandAllocator.Get(), nullptr);
			SetEvent(m_ssaoEndFrame);
		}
		return;
	}
	UINT __stdcall D3D12::WrapperSsaoThreadFun(LPVOID pParameter)
	{
		g_pD3D12->SsaoThread();
		return 0;
	}

	D3D12_RESOURCE_BARRIER D3D12::CreateResourceBarrier(ID3D12Resource* const pResource, 
		const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState)
	{	
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = pResource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = beforeState;
		barrier.Transition.StateAfter = afterState;
		return barrier;
	}
	D3D12_RESOURCE_BARRIER D3D12::CreateResourceBarrier(ID3D12Resource* const pResource, 
		const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState, const UINT mipLevel)
	{
		D3D12_RESOURCE_BARRIER barrier = CreateResourceBarrier(pResource, beforeState, afterState);
		barrier.Transition.Subresource = mipLevel;
		return barrier;
	}
	void D3D12::ExecuteCommandLists(ID3D12GraphicsCommandList** const ppCmdLists, const UINT cnt)
	{	 
		UINT fenceValue = m_fenceValue++;
		m_pMainCommandQueue->ExecuteCommandLists(cnt, reinterpret_cast<ID3D12CommandList**>(ppCmdLists));
		m_pMainCommandQueue->Signal(m_pFence.Get(), fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void D3D12::ResizeBackBuffer(const UINT newWidth, const UINT newHeight)
	{
		UINT fenceVal = m_fenceValue++;
		m_pMainCommandQueue->Signal(m_pFence.Get(), fenceVal);
		if (m_pFence->GetCompletedValue() < fenceVal)
		{
			m_pFence->SetEventOnCompletion(fenceVal, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		DestroyTexture();
		DestroySceneDepthTex();
		DestroyBackBuffer();

		m_clientWidth = newWidth;
		m_clientHeight = newHeight;

		if (!CreateRtvSrv(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D11::CreateRtvSrv()Failed");
		}

		if (!CreateDsv(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D11::CreateDsv()onScreenFailed");
		}

		m_pSwapChain->ResizeBuffers(_BUFFER_COUNT, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_EFFECT_FLIP_DISCARD);

		HRESULT hr;
		for (UINT i = 0; i < _BUFFER_COUNT; ++i)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pHeapManager->GetCurRtvHandle();
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pScreenTexs[i].GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pDevice->CreateRenderTargetView(m_pScreenTexs[i].Get(), nullptr, rtvHandle);
			m_screenRtvs[i] = rtvHandle;
			m_pHeapManager->IncreaseRtvHandleOffset();
			m_curFrame = 0;

		}


		m_viewport.Width = static_cast<float>(m_clientWidth);
		m_viewport.Height = static_cast<float>(m_clientHeight);
		m_scissorRect.right = m_clientWidth;
		m_scissorRect.bottom = m_clientHeight;

		{
			D3D12_RESOURCE_BARRIER barrier = 
				CreateResourceBarrier(m_pHdrTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
			m_pPbrSetupCommandList->ResourceBarrier(1, &barrier);

			ConvertEquirectagular2Cube();
			CreateDiffuseIrradianceMap();
			CreatePrefileterMap();
			CreateMipMap();
			CreateBrdfMap();
			UINT fenceValue = m_pbrSetupFenceValue++;
			m_pPbrSetupCommandList->Close();
			m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pPbrSetupCommandList.GetAddressOf()));
			m_pMainCommandQueue->Signal(m_pPbrSetupFence.Get(), fenceValue);
			if (m_pPbrSetupFence->GetCompletedValue() < fenceValue)
			{
				m_pPbrSetupFence->SetEventOnCompletion(fenceValue, m_pbrSetupEvent);
				WaitForSingleObject(m_pbrSetupEvent, INFINITE);
			}

			m_pPbrSetupCommandAllocator->Reset();
			m_pPbrSetupCommandList->Reset(m_pPbrSetupCommandAllocator.Get(), nullptr);
			PopulateBundle();
		}
	}
	void D3D12::WaitForGpu()
	{
		UINT fenceVal = m_fenceValue;
		m_pMainCommandQueue->Signal(m_pFence.Get(), fenceVal);
		if (m_pFence->GetCompletedValue() < fenceVal)
		{
			m_pFence->SetEventOnCompletion(fenceVal, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

	}
	bool D3D12::CreateRtvSrv(const UINT width, const UINT height)
	{
		//SSAO, À§Ä¡, ¹ý¼±, PBR ÅØ½ºÃÄµéÀº DXGI_FORMAT_R16G16B16A16_FLOAT;
		//Hi-Z´Â ±íÀÌ¶ó¼­ d32
		//±×¿Ü´Â DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D12_RESOURCE_DESC	texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		texDesc.Alignment = 0;
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = texDesc.Format;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Format = texDesc.Format;

		m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			m_pSceneTex.GetAddressOf(), m_pDevice.Get());
		m_pSceneTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pSceneTex") - 1, "D3D12:::m_pSceneTex");
		m_sceneRtv = m_pHeapManager->GetRtv(rtvDesc, m_pSceneTex.Get(), m_pDevice.Get());
		m_sceneSrv = m_pHeapManager->GetSrv(srvDesc, m_pSceneTex.Get(), m_pDevice.Get());
	
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ,
			m_pViewportTex.GetAddressOf(), m_pDevice.Get());
		m_pViewportTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pViewportTex") - 1, "D3D12::m_pViewportTex");
		m_viewportUav = m_pHeapManager->GetUav(uavDesc, m_pViewportTex.Get(), m_pDevice.Get());
		m_viewportSrv = m_pHeapManager->GetSrv(srvDesc, m_pViewportTex.Get(), m_pDevice.Get());


		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		for (int i = 0; i < static_cast<UINT>(eGbuf::cnt); ++i)
		{	
			if (i <static_cast<UINT>(eGbuf::albedo) || i >static_cast<UINT>(eGbuf::emissive))
			{
				texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			}
			else
			{
				texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			}
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ,
				m_pGBufTexs[i].GetAddressOf(), m_pDevice.Get());
			m_pGBufTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pGBufTexs[i]") - 1, "D3D12::m_pGBufTexs[i]");
		
			m_GBufRtvs[i] = m_pHeapManager->GetRtv(rtvDesc, m_pGBufTexs[i].Get(), m_pDevice.Get());
			m_GBufSrvs[i] = m_pHeapManager->GetSrv(srvDesc, m_pGBufTexs[i].Get(), m_pDevice.Get());
		}
		//Create DepthDebugTex
		texDesc.Format = DXGI_FORMAT_R32_FLOAT;
		rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
			m_pDepthDebugTex.GetAddressOf(), m_pDevice.Get());
		m_pDepthDebugTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pDepthDebugTex") - 1, "D3D12::m_pDepthDebugTex");
		m_pDepthDebugRtv = m_pHeapManager->GetRtv(rtvDesc, m_pDepthDebugTex.Get(), m_pDevice.Get());

		
		{

			UINT mipLevels = 1 + static_cast<UINT>(floorf(log2f(fmaxf(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE/2))));
			m_hiZTempRtvs = std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>(mipLevels);
			m_hiZTempSrvs = std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>(mipLevels);

			texDesc.MipLevels = mipLevels;
			texDesc.Width = _SHADOWMAP_SIZE;
			texDesc.Height = _SHADOWMAP_SIZE/2;
			texDesc.Format = DXGI_FORMAT_R32_FLOAT;
			rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
				m_pHiZTempTex.GetAddressOf(), m_pDevice.Get());
			m_pHiZTempTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pHiZTempTex") - 1, "D3D12:::m_pHiZTempTex");

			for (int i = 0; i < mipLevels; ++i)
			{
				rtvDesc.Texture2D.MipSlice = i;
				srvDesc.Texture2D.MostDetailedMip =i;
				m_hiZTempRtvs[i] = m_pHeapManager->GetRtv(rtvDesc, m_pHiZTempTex.Get(), m_pDevice.Get());
				m_hiZTempSrvs[i] = m_pHeapManager->GetSrv(srvDesc, m_pHiZTempTex.Get(), m_pDevice.Get());
			}

			//Culling¿ë
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			srvDesc.Texture2D.MipLevels = mipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			m_hiZTempSrvs.push_back(m_pHeapManager->GetSrv(srvDesc, m_pHiZTempTex.Get(), m_pDevice.Get()));


			texDesc.Width = sqrt(_HI_Z_CULL_COUNT);
			texDesc.Height = texDesc.Width;
			texDesc.Format = DXGI_FORMAT_R32_UINT;
			texDesc.MipLevels = 1;
			uavDesc.Format = DXGI_FORMAT_R32_UINT;
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					m_pHiZCullListTex[i].GetAddressOf(), m_pDevice.Get());
				m_pHiZCullListTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pHiZCullListTex") - 1, "D3D12:::m_pHiZCullListTex");

				m_hiZCullListUav[i] = m_pHeapManager->GetUav(uavDesc, m_pHiZCullListTex[i].Get(), m_pDevice.Get());
			}
			
		}
		

		//SSAO & Post process and UAV..
		{
			texDesc.Width = width/2;
			texDesc.Height = height/2;
			texDesc.MipLevels = 1;
			texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = 1;
			uavDesc.Format = texDesc.Format;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				m_pSsaoTex.GetAddressOf(), m_pDevice.Get());
			m_pSsaoTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pSsaoTex") - 1, "D3D12:::m_pSsaoTex");
			m_ssaoSrv = m_pHeapManager->GetSrv(srvDesc, m_pSsaoTex.Get(), m_pDevice.Get());
			m_ssaoUav = m_pHeapManager->GetUav(uavDesc, m_pSsaoTex.Get(), m_pDevice.Get());

			texDesc.Width = width;
			texDesc.Height = height;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				m_pSsaoBlurTex.GetAddressOf(), m_pDevice.Get());
			m_pSsaoBlurTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pSsaoBlurTex") - 1, "D3D12:::m_pSsaoBlurTex");
			m_ssaoBlurSrv = m_pHeapManager->GetSrv(srvDesc, m_pSsaoBlurTex.Get(), m_pDevice.Get());
			m_ssaoBlurUav = m_pHeapManager->GetUav(uavDesc, m_pSsaoBlurTex.Get(), m_pDevice.Get());

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS ,
				m_pSsaoBlurDebugTex.GetAddressOf(), m_pDevice.Get());
			m_pSsaoBlurDebugTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pSsaoBlurDebugTex") - 1, "D3D12:::m_pSsaoBlurDebugTex");
			m_ssaoBlurDebugSrv = m_pHeapManager->GetSrv(srvDesc, m_pSsaoBlurDebugTex.Get(), m_pDevice.Get());
			m_ssaoBlurDebugUav = m_pHeapManager->GetUav(uavDesc, m_pSsaoBlurDebugTex.Get(), m_pDevice.Get());
		}


		//PBR
		{ 
			texDesc.Width = _SHADOWMAP_SIZE;
			texDesc.Height = _SHADOWMAP_SIZE;
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			rtvDesc.Texture2D.MipSlice = 0;
			srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				m_pBrdfTex.GetAddressOf(), m_pDevice.Get());
			m_pBrdfTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pBrdfTex") - 1, "D3D12:::m_pBrdfTex");
			m_brdfRtv = m_pHeapManager->GetRtv(rtvDesc, m_pBrdfTex.Get(), m_pDevice.Get());
			m_brdfSrv = m_pHeapManager->GetSrv(srvDesc, m_pBrdfTex.Get(), m_pDevice.Get());
			
			texDesc.DepthOrArraySize = 6;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = 6;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = 1;
			srvDesc.TextureCube.MostDetailedMip = 0;

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				m_pSkyBoxTex.GetAddressOf(), m_pDevice.Get());
			m_pSkyBoxTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pSkyBoxTex") - 1, "D3D12:::m_pSkyBoxTex");
			m_skyBoxRtv = m_pHeapManager->GetRtv(rtvDesc, m_pSkyBoxTex.Get(), m_pDevice.Get());
			m_skyBoxSrv = m_pHeapManager->GetSrv(srvDesc, m_pSkyBoxTex.Get(), m_pDevice.Get());

			texDesc.Width = _DIFFIRRAD_WIDTH;
			texDesc.Height = _DIFFIRRAD_HEIGHT;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				m_pDiffIrradianceTex.GetAddressOf(), m_pDevice.Get());
			m_pDiffIrradianceTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pDiffIrradianceTex") - 1, "D3D12:::m_pDiffIrradianceTex");
			m_diffIrradianceRtv = m_pHeapManager->GetRtv(rtvDesc, m_pDiffIrradianceTex.Get(), m_pDevice.Get());
			m_diffIrradianceSrv = m_pHeapManager->GetSrv(srvDesc, m_pDiffIrradianceTex.Get(), m_pDevice.Get());
			

			texDesc.Width = _PREFILTER_WIDTH;
			texDesc.Height = _PREFILTER_HEIGHT;
			texDesc.MipLevels = _PREFILTER_MIP_LEVELS;
			srvDesc.TextureCube.MipLevels = _PREFILTER_MIP_LEVELS;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				m_pPrefilterTex.GetAddressOf(), m_pDevice.Get());
			m_pPrefilterTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPrefilterTex") - 1, "D3D12:::m_pPrefilterTex");
			m_prefilterRtv = m_pHeapManager->GetRtv(rtvDesc, m_pPrefilterTex.Get(), m_pDevice.Get());
			m_prefilterSrv = m_pHeapManager->GetSrv(srvDesc, m_pPrefilterTex.Get(), m_pDevice.Get());


			texDesc.Width = _PREFILTER_WIDTH;
			texDesc.Height = _PREFILTER_HEIGHT;
			texDesc.MipLevels = _PREFILTER_MIP_LEVELS;
			texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				m_pGenMipUavTex.GetAddressOf(), m_pDevice.Get());
			m_pGenMipUavTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pGenMipUavTex") - 1, "D3D12:::m_pGenMipUavTex");
		}
		return true;
	}
	bool D3D12::CreateDsv(const UINT width, const UINT height)
	{	
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		texDesc.Alignment = 0;
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;

		D3D12_CLEAR_VALUE clearVal = {};
		clearVal.Format = DXGI_FORMAT_D32_FLOAT;
		clearVal.DepthStencil.Depth = 1.0f;

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		
		m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
			m_pScreenDepthTex.GetAddressOf(), m_pDevice.Get());
		m_pScreenDepthTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pScreenDepthTex") - 1, "D3D12::m_pScreenDepthTex");
		m_screenDsv = m_pHeapManager->GetDsv(dsvDesc, m_pScreenDepthTex.Get(), m_pDevice.Get());

		m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE ,
			m_pSceneDepthTex.GetAddressOf(), m_pDevice.Get());
		m_pSceneDepthTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pSceneDepthTex") - 1, "D3D12::m_pSceneDepthTex");
		m_sceneDsv = m_pHeapManager->GetDsv(dsvDesc, m_pSceneDepthTex.Get(), m_pDevice.Get());
	
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		m_sceneDepthSrv = m_pHeapManager->GetSrv(srvDesc, m_pSceneDepthTex.Get(), m_pDevice.Get());
	
		return true;
	}

	void D3D12::DestroyTexture()
	{
		if (m_pViewportTex != nullptr)
		{
			m_pViewportTex.Reset();
		}

		if (m_pSceneTex != nullptr)
		{
			m_pSceneTex.Reset();
		}

		if (m_pSsaoTex != nullptr)
		{
			m_pSsaoTex.Reset();
		}

		if (m_pHiZTempTex != nullptr)
		{
			m_pHiZTempTex.Reset();
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			if (m_pHiZCullListTex[i] != nullptr)
			{
				m_pHiZCullListTex[i].Reset();
			}
		}

		if (m_pSsaoBlurTex != nullptr)
		{
			m_pSsaoBlurTex.Reset();
		}

		if (m_pSsaoBlurDebugTex != nullptr)
		{	
			m_pSsaoBlurDebugTex.Reset();
		}

		if (m_pBrdfTex != nullptr)
		{
			m_pBrdfTex.Reset();
		}

		if (m_pSkyBoxTex != nullptr)
		{
			m_pSkyBoxTex.Reset();
		}

		if (m_pDiffIrradianceTex != nullptr)
		{
			m_pDiffIrradianceTex.Reset();
		}

		if (m_pPrefilterTex != nullptr)
		{
			m_pPrefilterTex.Reset();
		}

		if (m_pGenMipUavTex != nullptr)
		{
			m_pGenMipUavTex.Reset();
		}

		for (int i = 0; i < static_cast<UINT>(eGbuf::cnt); ++i)
		{
			if (m_pGBufTexs[i] != nullptr)
			{
				m_pGBufTexs[i].Reset();
			}

		}
	}
	void D3D12::DestroySceneDepthTex()
	{
		if (m_pSceneDepthTex != nullptr)
		{
			m_pSceneDepthTex.Reset();
		}

	}

	bool D3D12::CreateEquirentangularMap(const char* pPath)
	{
		HRESULT hr;
		int width, height, nrComponents;
		//8perChannel->32perChannel
		float* pData = stbi_loadf(pPath,
			&width, &height, &nrComponents, STBI_rgb_alpha);
		if (pData)
		{
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC	texDesc = {};
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			texDesc.Alignment = 0;
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			DestroyHdr();

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				m_pHdrTex.GetAddressOf(), m_pDevice.Get());
			m_pHdrTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pHdrTex") - 1, "D3D12::m_pHdrTex");

			UploadTexThroughCB(texDesc, sizeof(XMVECTOR)*width, reinterpret_cast<UINT8*>(pData), m_pHdrTex.Get(), m_pHdrUploadCb.GetAddressOf(), m_pPbrSetupCommandList.Get());
			
			ID3D12CommandList* ppCommandList[1] = { m_pPbrSetupCommandList.Get() };
			ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };

			m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_hdrSrv = m_pHeapManager->GetSrv(srvDesc, m_pHdrTex.Get(), m_pDevice.Get());
			
			return true;
		}
		else
		{
			return false;
		}
	}

	void D3D12::ConvertEquirectagular2Cube()
	{
		
		UINT stride;
		UINT offset = 0;

		XMMATRIX capProj = XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f),
			1.0F, 0.1f, 10.0f);
		XMVECTOR eyePos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMMATRIX capView[6] =
		{	//º¯È¯µÈ ÅØ½ºÃÄ°¡ µÚÁýÇôÁ®ÀÖÀ½¿¡ À¯ÀÇ
			XMMatrixLookAtLH(eyePos, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
			XMMatrixLookAtLH(eyePos, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
		};
		XMMATRIX* pMatrix = reinterpret_cast<XMMATRIX*>(m_pEquirect2CubeCbBegin);
		for (int i = 0; i < 6; ++i)
		{
			pMatrix[i] = XMMatrixMultiplyTranspose(capView[i], capProj);

		}
		

		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		D3D12_RESOURCE_BARRIER barriers[] = {
			CreateResourceBarrier(m_pSkyBoxTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
			CreateResourceBarrier(m_pHdrTex.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		};
		m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
		m_pPbrSetupCommandList->SetPipelineState(m_pEquirect2CubePso.Get());
		m_pPbrSetupCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pPbrSetupCommandList->IASetVertexBuffers(0, 1, &m_skyBoxVbv);
		m_pPbrSetupCommandList->IASetIndexBuffer(&m_skyBoxIbv);
		m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pPbrSetupCommandList->SetGraphicsRootSignature(m_pShader->GetEquirect2CubeRootSingnature());
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eEquirect2CubeRP::gsCb), m_equirect2CubeCbv);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eEquirect2CubeRP::psTex), m_hdrSrv);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eEquirect2CubeRP::psSampler), m_wrapSsv);
		m_pPbrSetupCommandList->RSSetViewports(1, m_pShadowMap->GetViewport());
		m_pPbrSetupCommandList->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
		m_pPbrSetupCommandList->OMSetRenderTargets(1, &m_skyBoxRtv, TRUE, nullptr);
		m_pPbrSetupCommandList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);
		
	}

	void D3D12::CreateDiffuseIrradianceMap()
	{		
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		D3D12_RESOURCE_BARRIER barriers[] = {
			CreateResourceBarrier(m_pSkyBoxTex.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CreateResourceBarrier(m_pDiffIrradianceTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
		};

		m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers)/sizeof(D3D12_RESOURCE_BARRIER), barriers);
		m_pPbrSetupCommandList->SetPipelineState(m_pDiffuseIrradiancePso.Get());
		m_pPbrSetupCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pPbrSetupCommandList->IASetVertexBuffers(0, 1, &m_skyBoxVbv);
		m_pPbrSetupCommandList->IASetIndexBuffer(&m_skyBoxIbv);
		m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pPbrSetupCommandList->SetGraphicsRootSignature(m_pShader->GetDiffuseIrradianceRootSingnature());
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eDiffuseIrraidianceRP::gsCb), m_equirect2CubeCbv);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eDiffuseIrraidianceRP::psTex), m_skyBoxSrv);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eDiffuseIrraidianceRP::psSampler), m_clampSsv);
		m_pPbrSetupCommandList->RSSetViewports(1, &m_diffIrradViewport);
		m_pPbrSetupCommandList->RSSetScissorRects(1, &m_diffIrradRect);
		m_pPbrSetupCommandList->OMSetRenderTargets(1, &m_diffIrradianceRtv, TRUE, nullptr);
		m_pPbrSetupCommandList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);
		
	}

	void D3D12::CreatePrefileterMap()
	{	
		FLOAT r = 0;
		memcpy(m_pRoughnessCbBegin, &r, sizeof(FLOAT));

		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		D3D12_RESOURCE_BARRIER barriers[] = {
			CreateResourceBarrier(m_pDiffIrradianceTex.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CreateResourceBarrier(m_pPrefilterTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
		};

		m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
		m_pPbrSetupCommandList->SetPipelineState(m_pPrefilterPso.Get());

		m_pPbrSetupCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pPbrSetupCommandList->IASetVertexBuffers(0, 1, &m_skyBoxVbv);
		m_pPbrSetupCommandList->IASetIndexBuffer(&m_skyBoxIbv);
		m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pPbrSetupCommandList->SetGraphicsRootSignature(m_pShader->GetPrefilterRootSingnature());
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePrefilterRP::gsCb), m_equirect2CubeCbv);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePrefilterRP::psCb), m_roughnessCbv);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePrefilterRP::psTex), m_skyBoxSrv);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePrefilterRP::psSampler), m_clampSsv);
		m_pPbrSetupCommandList->RSSetViewports(1, &m_prefilterViewport);
		m_pPbrSetupCommandList->RSSetScissorRects(1, &m_prefilterRect);
		m_pPbrSetupCommandList->OMSetRenderTargets(1, &m_prefilterRtv, TRUE, nullptr);
		m_pPbrSetupCommandList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);

		D3D12_RESOURCE_BARRIER rtvToSrvBarrier =
			CreateResourceBarrier(m_pPrefilterTex.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_pPbrSetupCommandList->ResourceBarrier(1, &rtvToSrvBarrier);
	}

	void D3D12::CreateMipMap()
	{

		HRESULT hr;
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		//Gen Null UAVs
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Format = m_pPrefilterTex->GetDesc().Format;
		uavDesc.Texture2DArray.ArraySize = 6;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.pResource = m_pPrefilterTex.Get();

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.pResource = m_pGenMipUavTex.Get();


		UINT8* pMipCB;
		D3D12_RANGE readRange = {};
		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_pHeapManager->GetCurCbvSrvCpuHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_pHeapManager->GetCurCbvSrvGpuHandle();
	

		for (int i = 1; i < _PREFILTER_MIP_LEVELS; ++i)
		{
			{
				D3D12_RESOURCE_BARRIER srvToNsrvBarrier =
					CreateResourceBarrier(m_pPrefilterTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				m_pPbrSetupCommandList->ResourceBarrier(1, &srvToNsrvBarrier);

				uavDesc.Texture2DArray.MipSlice = i;

				cbvSrvCpuHandle = m_pHeapManager->GetCurCbvSrvCpuHandle();
				cbvSrvGpuHandle = m_pHeapManager->GetCurCbvSrvGpuHandle();
				m_pDevice->CreateUnorderedAccessView(m_pGenMipUavTex.Get(), nullptr, &uavDesc, cbvSrvCpuHandle);
				m_pHeapManager->IncreaseCbvSrvHandleOffset();

				FLOAT texelSize[2] = { _PREFILTER_WIDTH >> i, _PREFILTER_HEIGHT >> i };

				memcpy(m_pResolutionCbBegin, texelSize, sizeof(texelSize));

				m_pPbrSetupCommandList->SetPipelineState(m_pGenMipmapPso.Get());
				m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
				m_pPbrSetupCommandList->SetComputeRootSignature(m_pShader->GetGenMipShaderRootSingnature());
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eGenMipRP::csTex), m_prefilterSrv);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eGenMipRP::csUAV), cbvSrvGpuHandle);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eGenMipRP::csSampler), m_clampSsv);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(static_cast<UINT>(eGenMipRP::csCb), m_resolutionCbv);
				m_pPbrSetupCommandList->Dispatch(texelSize[0], texelSize[1], 6);//tex Dimesion


				D3D12_RESOURCE_BARRIER barriers[] = {
					CreateResourceBarrier(m_pPrefilterTex.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,  D3D12_RESOURCE_STATE_COPY_DEST),
					CreateResourceBarrier(m_pGenMipUavTex.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
				};
				m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
				m_pPbrSetupCommandList->Close();

				//È­¸é Å©±â Àç¼³Á¤ °¨¾È
				UINT fenceValue = m_pbrSetupFenceValue++;
				m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pPbrSetupCommandList.GetAddressOf()));
				m_pMainCommandQueue->Signal(m_pPbrSetupFence.Get(), fenceValue);
				if (m_pPbrSetupFence->GetCompletedValue() < fenceValue)
				{
					m_pPbrSetupFence->SetEventOnCompletion(fenceValue, m_pbrSetupEvent);
					WaitForSingleObject(m_pbrSetupEvent, INFINITE);
				}
			
				m_pPbrSetupCommandAllocator->Reset();				
				m_pPbrSetupCommandList->Reset(m_pPbrSetupCommandAllocator.Get(), nullptr);
			}
			
			{
				for (int j = 0; j < 6; ++j)
				{
					dst.SubresourceIndex = i + (_PREFILTER_MIP_LEVELS * j);
					src.SubresourceIndex = i + (_PREFILTER_MIP_LEVELS * j);
					m_pPbrSetupCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
				}

				D3D12_RESOURCE_BARRIER barriers[] = {
					CreateResourceBarrier(m_pPrefilterTex.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
					CreateResourceBarrier(m_pGenMipUavTex.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
				};

				m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
				m_pPbrSetupCommandList->Close();

				UINT fenceValue = m_pbrSetupFenceValue++;
				m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pPbrSetupCommandList.GetAddressOf()));
				m_pMainCommandQueue->Signal(m_pPbrSetupFence.Get(), fenceValue);
				if (m_pPbrSetupFence->GetCompletedValue() < fenceValue)
				{
					m_pPbrSetupFence->SetEventOnCompletion(fenceValue, m_pbrSetupEvent);
					WaitForSingleObject(m_pbrSetupEvent, INFINITE);
				}
				m_pPbrSetupCommandAllocator->Reset();
				m_pPbrSetupCommandList->Reset(m_pPbrSetupCommandAllocator.Get(), nullptr);
			}
			
			
		}


		//https://learn.microsoft.com/en-us/windows/win32/direct3d12/subresources

	}

	void D3D12::CreateBrdfMap()
	{
		//Gen BRDFMap
		ID3D12CommandList* ppCommandList[1] = { m_pPbrSetupCommandList.Get() };
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		D3D12_RESOURCE_BARRIER rtvToSrvBarrier = CreateResourceBarrier(m_pBrdfTex.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		D3D12_RESOURCE_BARRIER srvToRtvBarrier = CreateResourceBarrier(m_pBrdfTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

		m_pPbrSetupCommandList->ResourceBarrier(1, &srvToRtvBarrier);
		m_pPbrSetupCommandList->SetPipelineState(m_pBrdfPso.Get());
		m_pPbrSetupCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pPbrSetupCommandList->IASetVertexBuffers(0, 1, &m_quadVbv);
		m_pPbrSetupCommandList->IASetIndexBuffer(&m_quadIbv);
		m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pPbrSetupCommandList->SetGraphicsRootSignature(m_pShader->GetBrdfRootSingnature());
		m_pPbrSetupCommandList->RSSetViewports(1, m_pShadowMap->GetViewport());
		m_pPbrSetupCommandList->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
		m_pPbrSetupCommandList->OMSetRenderTargets(1, &m_brdfRtv, TRUE, nullptr);
		m_pPbrSetupCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);

		m_pPbrSetupCommandList->ResourceBarrier(1, &rtvToSrvBarrier);

	}

	void D3D12::DestroyHdr()
	{
		if (m_pHdrTex != nullptr)
		{
			m_pHdrTex->Release();
		}
	}

	void D3D12::DestroyBackBuffer()
	{
		if (m_pScreenDepthTex != nullptr)
		{
			m_pScreenDepthTex.Reset();
		}

		for (int i = 0; i < _BUFFER_COUNT; ++i)
		{	
			if (m_pScreenTexs[i] != nullptr)
			{
				m_pScreenTexs[i].Reset();
			}
		}
		

	}
	void D3D12::DrawObject(const ePass curPass, const UINT threadIndex, const UINT lightIdx)
	{
		XMMATRIX worldMat, invWorldMat;
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		std::vector<std::unique_ptr<DirectionalLight12>>& dirLights = m_pLightCb->GetDirLights();
		std::vector<std::unique_ptr<CubeLight12>>& cubeLights = m_pLightCb->GetCubeLights();
		std::vector<std::unique_ptr<SpotLight12>>& spotLights = m_pLightCb->GetSpotLights();

		UINT pass = static_cast<UINT>(curPass);
		while (!m_pMeshletQueue[pass].empty())
		{
			DWORD result =WaitForSingleObject(m_workerMutex, INFINITE);

			if (result==WAIT_OBJECT_0)
			{	
				if (m_lastMeshletIdx == m_pMeshletQueue[pass].size())
				{
					ReleaseMutex(m_workerMutex);
					break;
				}

				UINT nMeshes =min(m_pMeshletQueue[pass].size()- m_lastMeshletIdx, _OBJECT_PER_THREAD);
				UINT startMeshletIdx = m_lastMeshletIdx;
				m_lastMeshletIdx += nMeshes;	
				UINT lastMeshletIdx = m_lastMeshletIdx;
				m_nDrawCall +=nMeshes;
				//Set-up Status
				
				ReleaseMutex(m_workerMutex);
				switch (curPass)
				{
				case wilson::ePass::zPass:
				{
					m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pZpassPso.Get());
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootSignature(m_pShader->GetZpassRootSignature());
					m_pWorkerCommandLists[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pWorkerCommandLists[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootDescriptorTable(static_cast<UINT>(eZpassRP::psWrap), m_wrapSsv);
					m_pWorkerCommandLists[threadIndex]->RSSetViewports(1, &m_viewport);
					m_pWorkerCommandLists[threadIndex]->RSSetScissorRects(1, &m_scissorRect);
					m_pWorkerCommandLists[threadIndex]->OMSetRenderTargets(1, &m_pDepthDebugRtv, FALSE, &m_sceneDsv);
				}
					break;
				case wilson::ePass::cascadeDirShadowPass:
				{
					m_pWorkerCommandLists[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pCascadeDirShadowPso.Get());
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootSignature(m_pShader->GetCascadeDirShadowRootSingnature());
					m_pWorkerCommandLists[threadIndex]->RSSetViewports(1, m_pShadowMap->GetViewport());
					m_pWorkerCommandLists[threadIndex]->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
					m_pWorkerCommandLists[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					dirLights[lightIdx]->UpdateLightSpaceMatrices();
					dirLights[lightIdx]->UploadShadowMatrices(m_pWorkerCommandLists[threadIndex].Get());
					m_pShadowMap->BindDirDsv(m_pWorkerCommandLists[threadIndex].Get(), lightIdx);
					
				}
					break;
				case wilson::ePass::spotShadowPass:
				{
					m_pWorkerCommandLists[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pSpotShadowPso.Get());
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootSignature(m_pShader->GetSpotShadowRootSingnature());
					m_pWorkerCommandLists[threadIndex]->RSSetViewports(1, m_pShadowMap->GetViewport());
					m_pWorkerCommandLists[threadIndex]->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
					m_pWorkerCommandLists[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					spotLights[lightIdx]->UpdateLitMat();
					m_pLitMat = spotLights[lightIdx]->GetLightSpaceMat();
					m_pShadowMap->BindSpotDsv(m_pWorkerCommandLists[threadIndex].Get(), lightIdx);
				}
					break;
				case wilson::ePass::cubeShadowPass:
				{	
					m_pWorkerCommandLists[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pCubeShadowPso.Get());
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootSignature(m_pShader->GetCubeShadowRootSingnature());
					m_pWorkerCommandLists[threadIndex]->RSSetViewports(1, m_pShadowMap->GetViewport());
					m_pWorkerCommandLists[threadIndex]->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
					m_pWorkerCommandLists[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootDescriptorTable(static_cast<UINT>(eCubeShadowRP::psSampler), m_wrapSsv);
					m_pShadowMap->BindCubeDsv(m_pWorkerCommandLists[threadIndex].Get(), lightIdx);
					cubeLights[lightIdx]->UploadShadowMatrices(m_pWorkerCommandLists[threadIndex].Get());
					cubeLights[lightIdx]->UploadLightPos(m_pWorkerCommandLists[threadIndex].Get());
				}
					break;
				case wilson::ePass::geoPass:
				{

					m_pWorkerCommandLists[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootSignature(m_pShader->GetPbrDeferredGeoShaderRootSingnature());
					m_pWorkerCommandLists[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pWorkerCommandLists[threadIndex]->RSSetViewports(1, &m_viewport);
					m_pWorkerCommandLists[threadIndex]->RSSetScissorRects(1, &m_scissorRect);

					m_pCam->UploadCamPos(m_pWorkerCommandLists[threadIndex].Get(), true);
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psSampler), m_wrapSsv);
					m_pWorkerCommandLists[threadIndex]->OMSetRenderTargets(static_cast<UINT>(eGbuf::cnt), m_GBufRtvs, FALSE, &m_sceneDsv);
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrGeoRP::psHeightScale), m_heightScaleCbv);
				}
					break;
				default:
					break;
				}
			
				for (UINT i = startMeshletIdx; i < lastMeshletIdx; ++i)
				{	
					Mesh12* pParentMesh = m_pMeshletQueue[pass][i]->GetParentMesh();
					MatrixHandler12* pMatBuffer = pParentMesh->GetMatBuffer();
					if (m_pCam->GetDirtyBit())
					{
						pMatBuffer->SetViewMatrix(m_pCam->GetViewMatrix());
						pMatBuffer->SetProjMatrix(m_pCam->GetProjectionMatrix());
						pMatBuffer->SetDirtyBit(true);
					}
					if (pMatBuffer->GetDirtyBit())
					{
						pMatBuffer->UpdateCombinedMat(false);
						pMatBuffer->SetDirtyBit(false);
					}
				
					switch (curPass)
					{
					case wilson::ePass::zPass:
						pMatBuffer->UploadCombinedMat(m_pWorkerCommandLists[threadIndex].Get());
						break;
					case wilson::ePass::spotShadowPass:
						pMatBuffer->SetLightSpaceMatrix(m_pLitMat);
						pMatBuffer->UploadCombinedMat(m_pWorkerCommandLists[threadIndex].Get());
						break;
					case wilson::ePass::cascadeDirShadowPass:
					case wilson::ePass::cubeShadowPass:
					case wilson::ePass::geoPass:
						pMatBuffer->UploadMatBuffer(m_pWorkerCommandLists[threadIndex].Get());
						break;
					default:
						break;
					}

					if (curPass == ePass::geoPass||curPass==ePass::zPass)
					{
						int meshletIdx = m_pMeshletQueue[pass][i]->GetMeshletIdx();
						{
							
							{
								PerModel perModel = *(pParentMesh->GetPerModel(0));
								if (curPass == ePass::geoPass)
								{
									if (perModel.hasNormal)
									{
										if (m_bHeightOn)
										{
											if (perModel.hasEmissive)
											{
												m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalHeightEmissivePso.Get());
											}
											else
											{
												m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalHeightPso.Get());
											}
										}
										else
										{	
											if (perModel.hasEmissive)
											{
												m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalEmissivePso.Get());
											}
											else
											{
												m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalPso.Get());
											}
											
										}
									}
									else
									{
										if (perModel.hasEmissive)
										{
											m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pPbrDeferredGeoEmissivePso.Get());
										}
										else
										{
											m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pPbrDeferredGeoPso.Get());
										}
										
									}
								}
								
							}

							pParentMesh->UploadBuffers(m_pWorkerCommandLists[threadIndex].Get(), meshletIdx, curPass);
							m_pWorkerCommandLists[threadIndex]->DrawIndexedInstanced(pParentMesh->GetIndexCount(meshletIdx), 1,
								0, 0, 0);
						}
					}
					else
					{
						pParentMesh->UploadBuffers(m_pWorkerCommandLists[threadIndex].Get(), 0, curPass);
						m_pWorkerCommandLists[threadIndex]->DrawIndexedInstanced(pParentMesh->GetTotalIndexCount(), 1,
							0, 0, 0);
					}
					
				}
				m_pWorkerCommandLists[threadIndex]->Close();
				m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pWorkerCommandLists[threadIndex].GetAddressOf()));
				UINT fenceValue = m_workerFenceValues[threadIndex]++;
				m_pMainCommandQueue->Signal(m_pWorkerFences[threadIndex].Get(), fenceValue);
				if (m_pWorkerFences[threadIndex]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFences[threadIndex]->SetEventOnCompletion(fenceValue, m_workerFenceEvents[threadIndex]);
					WaitForSingleObject(m_workerFenceEvents[threadIndex], INFINITE);
				}
				
				HRESULT hr = m_pWorkerCommandAllocators[threadIndex]->Reset();
				assert(SUCCEEDED(hr));
				m_pWorkerCommandLists[threadIndex]->Reset(m_pWorkerCommandAllocators[threadIndex].Get(), nullptr);
			}
		}
		

	}
	void D3D12::HiZCull(const UINT threadIndex)
	{
		//Hi-Z Occlusion Pass
		{
			UINT hi_zPass = static_cast<UINT>(ePass::hi_zPass);
			UINT hwOcclusionPass = static_cast<UINT>(ePass::HWOcclusionPass);
			UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE/2)));

			ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
										*(m_pHeapManager->GetCbvSrvUavHeap()) };
			XMMATRIX& vpMat = m_pCam->GetViewProjectionMatrix();
			while (m_lastMeshletIdx < m_pMeshletQueue[hi_zPass].size())
			{

				DWORD result = WaitForSingleObject(m_workerMutex, INFINITE);

				if (result == WAIT_OBJECT_0)
				{
					if (m_lastMeshletIdx == m_pMeshletQueue[hi_zPass].size())
					{
						ReleaseMutex(m_workerMutex);
						break;
					}

					UINT nMeshes = min(m_pMeshletQueue[hi_zPass].size() - m_lastMeshletIdx, _HI_Z_CULL_COUNT);

					UINT startMeshletIdx = m_lastMeshletIdx;
					m_lastMeshletIdx += nMeshes;
					UINT lastMeshletIdx = m_lastMeshletIdx;

					m_pHiZQueue[threadIndex].reserve(nMeshes);
					ReleaseMutex(m_workerMutex);

					UINT8* pAABBCbBegin = m_pAabbCbBegin[threadIndex];
					UINT8* pHiZCullMatrixCbBegin = m_pHiZCullMatrixCbBegin[threadIndex];


					UINT threadGroupXCnt = sqrt(_HI_Z_CULL_COUNT);
					std::vector<XMVECTOR> vertices(nMeshes * 8);
					std::vector<XMMATRIX> matrices(nMeshes);
					for (int i = startMeshletIdx; i < lastMeshletIdx; ++i)
					{
						Meshlet* pMeshlet = m_pMeshletQueue[hi_zPass][i];
						Mesh12* pParentMesh = pMeshlet->GetParentMesh();
						
						UINT cbufferIdx = i - startMeshletIdx;
						matrices[cbufferIdx] = vpMat;
						{
							{
								AABB* aabb = pMeshlet->GetAABB();
								XMFLOAT3* pVertices = aabb->GetVertices();
								for (int j = 0; j < 8; ++j)
								{
									int idx = cbufferIdx * 8 + j;
									vertices[idx] =
										XMVectorSet(pVertices[j].x, pVertices[j].y, pVertices[j].z, 1.0f);
								}
							}
						}
					}

					memcpy(pAABBCbBegin, &vertices[0], sizeof(XMVECTOR) * 8 * nMeshes);
					memcpy(pHiZCullMatrixCbBegin, &matrices[0], sizeof(XMMATRIX) * nMeshes);
					m_pWorkerCommandLists[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pHiZCullPassPso.Get());
					m_pWorkerCommandLists[threadIndex]->SetComputeRootSignature(m_pShader->GetHiZCullPassRootSignature());
					m_pWorkerCommandLists[threadIndex]->SetComputeRootDescriptorTable(static_cast<UINT>(eHiZCullRP::csHiZ), m_hiZTempSrvs[mipLevels]);
					m_pWorkerCommandLists[threadIndex]->SetComputeRootDescriptorTable(static_cast<UINT>(eHiZCullRP::csDst), m_hiZCullListUav[threadIndex]);
					m_pWorkerCommandLists[threadIndex]->SetComputeRootDescriptorTable(static_cast<UINT>(eHiZCullRP::csResolution), m_resolutionCbv);
					m_pWorkerCommandLists[threadIndex]->SetComputeRootDescriptorTable(static_cast<UINT>(eHiZCullRP::csBound), m_aabbCbv[threadIndex]);
					m_pWorkerCommandLists[threadIndex]->SetComputeRootDescriptorTable(static_cast<UINT>(eHiZCullRP::csMatrix), m_hiZCullMatrixCbv[threadIndex]);
					m_pWorkerCommandLists[threadIndex]->SetComputeRootDescriptorTable(static_cast<UINT>(eHiZCullRP::csBorder), m_borderSsv);
					m_pWorkerCommandLists[threadIndex]->Dispatch(threadGroupXCnt, 1, 1);

					D3D12_RESOURCE_BARRIER fininshHiZ[] =
					{
						CreateResourceBarrier(m_pHiZCullListTex[threadIndex].Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE)
					};
					m_pWorkerCommandLists[threadIndex]->ResourceBarrier(sizeof(fininshHiZ) / sizeof(D3D12_RESOURCE_BARRIER), fininshHiZ);
					//ReadBackResult;
					{
						UINT len = sqrt(_HI_Z_CULL_COUNT);
						D3D12_TEXTURE_COPY_LOCATION dst = {};
						dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R32_UINT;
						dst.PlacedFootprint.Footprint.Width = len;
						dst.PlacedFootprint.Footprint.RowPitch = CUSTUM_ALIGN(sizeof(UINT) * len, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
						dst.PlacedFootprint.Footprint.Height = len;
						dst.PlacedFootprint.Footprint.Depth = 1;

						dst.pResource = m_pHiZCullReadCb[threadIndex].Get();
						dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

						D3D12_TEXTURE_COPY_LOCATION src = {};
						src.pResource = m_pHiZCullListTex[threadIndex].Get();
						src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
						src.SubresourceIndex = 0;
						//Á÷Á¢ º¹»çÇÒ°Í
						m_pWorkerCommandLists[threadIndex]->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
					}
					//TODO:READ기다린다고 루프가 멈춰선 안됌
					D3D12_RESOURCE_BARRIER finishReadBack[] =
					{
						CreateResourceBarrier(m_pHiZCullListTex[threadIndex].Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
					};
					m_pWorkerCommandLists[threadIndex]->ResourceBarrier(sizeof(finishReadBack) / sizeof(D3D12_RESOURCE_BARRIER), finishReadBack);

					m_pWorkerCommandLists[threadIndex]->Close();
					m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pWorkerCommandLists[threadIndex].GetAddressOf()));

					UINT fenceValue = m_workerFenceValues[threadIndex]++;
					m_pMainCommandQueue->Signal(m_pWorkerFences[threadIndex].Get(), fenceValue);
					if (m_pWorkerFences[threadIndex]->GetCompletedValue() < fenceValue)
					{
						m_pWorkerFences[threadIndex]->SetEventOnCompletion(fenceValue, m_workerFenceEvents[threadIndex]);
						WaitForSingleObject(m_workerFenceEvents[threadIndex], INFINITE);
					}

					m_pWorkerCommandAllocators[threadIndex]->Reset();
					m_pWorkerCommandLists[threadIndex]->Reset(m_pWorkerCommandAllocators[threadIndex].Get(), nullptr);

					UINT* pHiZCullReadCbBegin = reinterpret_cast<UINT*>(m_pHiZCullReadCbBegin[threadIndex]);

					//Refresh queue
					for (int j = 0; j < nMeshes; ++j)
					{
						//4096으로 했던이유가 최소 256바이트로 cb정렬이되어서 였음. 정렬감안
						int y= j / threadGroupXCnt;
						int x = j % threadGroupXCnt;
						int offset = y * 64 + x;
						UINT result = *(pHiZCullReadCbBegin + offset);
						Meshlet* pMeshlet = m_pMeshletQueue[hi_zPass][startMeshletIdx + j];
						if (result)
						{
							m_pHiZQueue[threadIndex].push_back(pMeshlet);
						}
					}

				}
			}
		}
	}

	void D3D12::HWQueryForOcclusion(const UINT threadIndex)
	{	
		ID3D12DescriptorHeap* ppHeaps[] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		UINT pass = static_cast<UINT>(ePass::HWOcclusionPass);
		while (!m_pMeshletQueue[pass].empty())
		{
			DWORD result = WaitForSingleObject(m_workerMutex, INFINITE);

			if (result == WAIT_OBJECT_0)
			{
				if (m_lastMeshletIdx == m_pMeshletQueue[pass].size())
				{
					ReleaseMutex(m_workerMutex);
					break;
				}
				UINT nMeshes = min(m_pMeshletQueue[pass].size() - m_lastMeshletIdx, _OBJECT_PER_THREAD);

				UINT startMeshletIdx = m_lastMeshletIdx;
				m_lastMeshletIdx += nMeshes;
				UINT lastMeshletIdx = m_lastMeshletIdx;
				m_nDrawCall += nMeshes;
				
				ReleaseMutex(m_workerMutex);
				UINT nQuery = 0;
				for (UINT i = startMeshletIdx; i < lastMeshletIdx; ++i)
				{
					Mesh12* pParentMesh = m_pMeshletQueue[pass][i]->GetParentMesh();
					int meshletIdx = m_pMeshletQueue[pass][i]->GetMeshletIdx();
					
					MatrixHandler12* pMatBuffer = pParentMesh->GetMatBuffer();
					//m_pHWOcclusionQueryPso´Â DSS¸¸ Zpass¶û ´Ù¸§
					m_pWorkerCommandLists[threadIndex]->SetPipelineState(m_pHWOcclusionQueryPso.Get());
					m_pWorkerCommandLists[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandLists[threadIndex]->SetGraphicsRootSignature(m_pShader->GetZpassRootSignature());
					m_pWorkerCommandLists[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pWorkerCommandLists[threadIndex]->RSSetViewports(1, &m_viewport);
					m_pWorkerCommandLists[threadIndex]->RSSetScissorRects(1, &m_scissorRect);
					m_pWorkerCommandLists[threadIndex]->OMSetRenderTargets(1, &m_sceneRtv, FALSE, &m_sceneDsv);

					pMatBuffer->UploadCombinedMat(m_pWorkerCommandLists[threadIndex].Get());

					pParentMesh->UploadBuffers(m_pWorkerCommandLists[threadIndex].Get(), meshletIdx, ePass::occlusionTestPass);
					m_pWorkerCommandLists[threadIndex]->BeginQuery(m_pQueryHeap[threadIndex].Get(), D3D12_QUERY_TYPE_OCCLUSION, nQuery);
					m_pWorkerCommandLists[threadIndex]->DrawIndexedInstanced(pParentMesh->GetIndexCount(meshletIdx), 1,
						0, 0, 0);
					m_pWorkerCommandLists[threadIndex]->EndQuery(m_pQueryHeap[threadIndex].Get(), D3D12_QUERY_TYPE_OCCLUSION, nQuery++);
				}

				ID3D12Resource* pQueryBlock = m_pHeapManager->GetQueryBlock();
				m_pWorkerCommandLists[threadIndex]->ResolveQueryData(m_pQueryHeap[threadIndex].Get(), D3D12_QUERY_TYPE_OCCLUSION, 0, nQuery,
					pQueryBlock, m_queryResultOffsets[threadIndex]);

				D3D12_RESOURCE_BARRIER copyDstToSrc =
				{
					CreateResourceBarrier(pQueryBlock, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE)
				};
				m_pWorkerCommandLists[threadIndex]->ResourceBarrier(1, &copyDstToSrc);
				m_pWorkerCommandLists[threadIndex]->CopyBufferRegion(m_pQueryReadBuffers[threadIndex].Get(), 0, pQueryBlock, m_queryResultOffsets[threadIndex],
					sizeof(UINT64) * min(nMeshes, _OBJECT_PER_THREAD));

				D3D12_RESOURCE_BARRIER copySrcToDst =
				{
					CreateResourceBarrier(pQueryBlock, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST)
				};
				m_pWorkerCommandLists[threadIndex]->ResourceBarrier(1, &copySrcToDst);

				m_pWorkerCommandLists[threadIndex]->Close();
				m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pWorkerCommandLists[threadIndex].GetAddressOf()));
				UINT fenceValue = m_workerFenceValues[threadIndex]++;
				m_pMainCommandQueue->Signal(m_pWorkerFences[threadIndex].Get(), fenceValue);
				if (m_pWorkerFences[threadIndex]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFences[threadIndex]->SetEventOnCompletion(fenceValue, m_workerFenceEvents[threadIndex]);
					WaitForSingleObject(m_workerFenceEvents[threadIndex], INFINITE);
				}
				m_pWorkerCommandAllocators[threadIndex]->Reset();
				m_pWorkerCommandLists[threadIndex]->Reset(m_pWorkerCommandAllocators[threadIndex].Get(), nullptr);

				m_pHWOcclusionQueue[threadIndex].reserve(nMeshes);
				for (int i = 0; i < nMeshes; ++i)
				{
					UINT64 result = *(reinterpret_cast<UINT64*>(m_pQueryReadCbBegins[threadIndex]) + i);

					if (result)
					{
						m_pHWOcclusionQueue[threadIndex].push_back(m_pMeshletQueue[pass][i+startMeshletIdx]);
					}
				}

			}
		}
		
	}
	void D3D12::PopulateBundle()
	{
		UINT fenceValue = m_pbrSetupFenceValue++;
		m_pPbrSetupCommandList->Close();
		m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pPbrSetupCommandList.GetAddressOf()));
		m_pMainCommandQueue->Signal(m_pPbrSetupFence.Get(), fenceValue);
		if (m_pPbrSetupFence->GetCompletedValue() < fenceValue)
		{
			m_pPbrSetupFence->SetEventOnCompletion(fenceValue, m_pbrSetupEvent);
			WaitForSingleObject(m_pbrSetupEvent, INFINITE);
		}

		m_pPbrSetupCommandAllocator->Reset();
		m_pPbrSetupCommandList->Reset(m_pPbrSetupCommandAllocator.Get(), nullptr);
		{
			ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };

			for (int i = 0; i < static_cast<UINT>(ePass::cnt); ++i)
			{	
				if ((i == static_cast<UINT>(ePass::zPass)) ||
					(i == static_cast<UINT>(ePass::occlusionTestPass)) ||
					(i == static_cast<UINT>(ePass::skyBoxPass)) ||
					(i == static_cast<UINT>(ePass::lightingPass)) ||
					(i == static_cast<UINT>(ePass::postProcess)))
				{
					m_pBundleAllocators[i]->Reset();
					m_pBundles[i]->Reset(m_pBundleAllocators[i].Get(), nullptr);
				}
			}

			m_pBundles[static_cast<UINT>(ePass::zPass)]->SetPipelineState(m_pZpassPso.Get());
			m_pBundles[static_cast<UINT>(ePass::zPass)]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pBundles[static_cast<UINT>(ePass::zPass)]->SetGraphicsRootSignature(m_pShader->GetZpassRootSignature());
			m_pBundles[static_cast<UINT>(ePass::zPass)]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pBundles[static_cast<UINT>(ePass::zPass)]->Close();


			m_pBundles[static_cast<UINT>(ePass::occlusionTestPass)]->SetPipelineState(m_pHWOcclusionQueryPso.Get());
			m_pBundles[static_cast<UINT>(ePass::occlusionTestPass)]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pBundles[static_cast<UINT>(ePass::occlusionTestPass)]->SetGraphicsRootSignature(m_pShader->GetZpassRootSignature());
			m_pBundles[static_cast<UINT>(ePass::occlusionTestPass)]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pBundles[static_cast<UINT>(ePass::occlusionTestPass)]->Close();

			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->SetPipelineState(m_pSkyBoxPso.Get());
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->SetGraphicsRootSignature(m_pShader->GetSkyBoxRootSingnature());
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(eSkyboxRP::psDiffuseMap), m_skyBoxSrv);
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(eSkyboxRP::psSampler), m_wrapSsv);
			m_pMatricesCb->SetWorldMatrix(&m_idMat);
			m_pMatricesCb->SetViewMatrix(m_pCam->GetViewMatrix());
			m_pMatricesCb->SetProjMatrix(m_pCam->GetProjectionMatrix());
			m_pMatricesCb->UpdateCombinedMat(false);
			m_pMatricesCb->UploadCombinedMat(m_pBundles[static_cast<UINT>(ePass::skyBoxPass)].Get());
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->IASetVertexBuffers(0, 1, &m_skyBoxVbv);
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->IASetIndexBuffer(&m_skyBoxIbv);
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);
			m_pBundles[static_cast<UINT>(ePass::skyBoxPass)]->Close();

			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetPipelineState(m_pPbrDeferredLightingPso.Get());
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootSignature(m_pShader->GetPbrDeferredLightingShaderRootSingnature());
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psPos), m_GBufSrvs[static_cast<UINT>(eGbuf::pos)]);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psNormal), m_GBufSrvs[static_cast<UINT>(eGbuf::normal)]);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psAlbedo), m_GBufSrvs[static_cast<UINT>(eGbuf::albedo)]);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psSpecular), m_GBufSrvs[static_cast<UINT>(eGbuf::specular)]);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psEmissive), m_GBufSrvs[static_cast<UINT>(eGbuf::emissive)]);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psAO), m_ssaoBlurSrv);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psIrradiance), m_diffIrradianceSrv);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psPrefilter), m_prefilterSrv);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psBrdf), m_brdfSrv);
			m_pShadowMap->BindDirSrv(m_pBundles[static_cast<UINT>(ePass::lightingPass)].Get());
			m_pShadowMap->BindSpotSrv(m_pBundles[static_cast<UINT>(ePass::lightingPass)].Get());
			m_pShadowMap->BindCubeSrv(m_pBundles[static_cast<UINT>(ePass::lightingPass)].Get());
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psWrap), m_wrapSsv);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psCubeShadowSampler), m_pShadowMap->GetCubeShadowSamplerView());
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psShadowSampler), m_pShadowMap->GetDirShadowSamplerView());
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->IASetVertexBuffers(0, 1, &m_quadVbv);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->IASetIndexBuffer(&m_quadIbv);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pBundles[static_cast<UINT>(ePass::lightingPass)]->Close();

			m_pBundles[static_cast<UINT>(ePass::postProcess)]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pBundles[static_cast<UINT>(ePass::postProcess)]->SetPipelineState(m_pPostProcessPso.Get());
			m_pBundles[static_cast<UINT>(ePass::postProcess)]->SetComputeRootSignature(m_pShader->GetPostProcessShaderRootSingnature());
			m_pBundles[static_cast<UINT>(ePass::postProcess)]->SetComputeRootDescriptorTable(static_cast<UINT>(ePostProcessRP::csExposure), m_exposureCbv);
			m_pBundles[static_cast<UINT>(ePass::postProcess)]->SetComputeRootDescriptorTable(static_cast<UINT>(ePostProcessRP::csTex), m_sceneSrv);
			m_pBundles[static_cast<UINT>(ePass::postProcess)]->SetComputeRootDescriptorTable(static_cast<UINT>(ePostProcessRP::csUav), m_viewportUav);
			m_pBundles[static_cast<UINT>(ePass::postProcess)]->SetComputeRootDescriptorTable(static_cast<UINT>(ePostProcessRP::csSampler), m_wrapSsv);
			m_pBundles[static_cast<UINT>(ePass::postProcess)]->Dispatch(ceil(m_clientWidth / static_cast<float>(8)), ceil(m_clientHeight / static_cast<float>(8)), 1);
			m_pBundles[static_cast<UINT>(ePass::postProcess)]->Close();
		}
	}
	void D3D12::UpdateDrawLists()
	{
		UINT nMesh = 0;
		for (int i = 0; i < m_pObjects.size(); ++i)
		{
			std::vector<std::unique_ptr<Mesh12>>& pMeshes = m_pObjects[i]->GetMeshes();
			nMesh += pMeshes.size();
		}
		std::vector<Mesh12*> meshes;
		meshes.reserve(nMesh);
		for (int i = 0; i < m_pObjects.size(); ++i)
		{
			std::vector<std::unique_ptr<Mesh12>>& pMeshes = m_pObjects[i]->GetMeshes();
			for (int j = 0; j < pMeshes.size(); ++j)
			{
				meshes.push_back(pMeshes[j].get());
			}
		}
		m_pTotalMeshes = meshes;
	}
	void D3D12::AddObject(Object12* const pObject)
	{
		std::unique_ptr<Object12> uptr;
		uptr.reset(pObject);
		m_pObjects.push_back(std::move(uptr));
		UpdateDrawLists();
	}
	void D3D12::RemoveObject(const int i)
	{
		m_pObjects.erase(m_pObjects.begin() + i);
		UpdateDrawLists();
	}
	void D3D12::RemoveMesh(const int i, const int j)
	{
		std::vector<std::unique_ptr<Mesh12>>& pMeshes = m_pObjects[i]->GetMeshes();
		pMeshes.erase(pMeshes.begin() + j);
		UpdateDrawLists();
	}
	UINT D3D12::GetNumMesh(const int i)
	{
		std::vector<std::unique_ptr<Mesh12>>& pMeshes = m_pObjects[i]->GetMeshes();
		return pMeshes.size();
	}
	UINT D3D12::GetNumLight(const eLIGHT_TYPE eLType)
	{
		UINT size = 0;
		switch (eLType)
		{
		case eLIGHT_TYPE::DIR:
			size = m_pLightCb->GetDirLightsSize();
			break;
		case eLIGHT_TYPE::CUBE:
			size = m_pLightCb->GetCubeLightsSize();
			break;
		case eLIGHT_TYPE::SPT:
			size = m_pLightCb->GetSpotLightsSize();
		}
		return size;
	}
	void D3D12::AddLight(Light12* const pLight)
	{
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			m_pLightCb->PushDirLight(reinterpret_cast<DirectionalLight12*>(pLight));
			break;
		case eLIGHT_TYPE::CUBE:
			m_pLightCb->PushCubeLight(reinterpret_cast<CubeLight12*>(pLight));
			break;
		case eLIGHT_TYPE::SPT:
			m_pLightCb->PushSpotLight(reinterpret_cast<SpotLight12*>(pLight));
		}
	}
	void D3D12::RemoveLight(const int i, Light12* const pLight)
	{
		std::vector<std::unique_ptr<DirectionalLight12>>& pDirLights = m_pLightCb->GetDirLights();;
		std::vector<std::unique_ptr<CubeLight12>>& pCubeLights = m_pLightCb->GetCubeLights();
		std::vector<std::unique_ptr<SpotLight12>>& pSpotLights = m_pLightCb->GetSpotLights();
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			pDirLights.erase(pDirLights.begin() + i);
			break;
		case eLIGHT_TYPE::CUBE:
			pCubeLights.erase(pCubeLights.begin() + i);
			break;
		case eLIGHT_TYPE::SPT:
			pSpotLights.erase(pSpotLights.begin() + i);
			break;
		}
	}
	void D3D12::UpdateScene()
	{
		//WaitForGpu();
		HRESULT hr;
		float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		D3D12_RANGE readRange = { 0, };

		PIXBeginEvent(m_pMainCommandList.Get(), 0, L"Init Textures and Set Barriers");
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pMainCommandList->SetPipelineState(m_pZpassPso.Get());


		ID3D12Resource* pTextures[] = { m_pScreenTexs[m_curFrame].Get(), m_pViewportTex.Get(), m_pSceneTex.Get(), m_pSsaoTex.Get(), m_pSsaoBlurTex.Get(), m_pSsaoBlurDebugTex.Get() };
		UINT nTex = sizeof(pTextures) / sizeof(ID3D12Resource*) + static_cast<UINT>(eGbuf::cnt);
		std::vector<D3D12_RESOURCE_BARRIER> barriers(nTex);
		barriers[0] = CreateResourceBarrier(m_pScreenTexs[m_curFrame].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers[1] = CreateResourceBarrier(m_pViewportTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[2] = CreateResourceBarrier(m_pSceneTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers[3] = CreateResourceBarrier(m_pSsaoTex.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[4] = CreateResourceBarrier(m_pSsaoBlurTex.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[5] = CreateResourceBarrier(m_pSsaoBlurDebugTex.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		for (int i = nTex - static_cast<UINT>(eGbuf::cnt), j = 0; i < nTex; ++i, ++j)
		{
			barriers[i] = CreateResourceBarrier(m_pGBufTexs[j].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		
		m_pMainCommandList->ResourceBarrier(nTex, &barriers[0]);
		

		m_pMainCommandList->ClearRenderTargetView(m_screenRtvs[m_curFrame], color, 0, nullptr);
		m_pMainCommandList->ClearRenderTargetView(m_sceneRtv, color, 0, nullptr);

		for (int i = 0; i < static_cast<UINT>(eGbuf::cnt); ++i)
		{
			m_pMainCommandList->ClearRenderTargetView(m_GBufRtvs[i], color, 0, nullptr);
		}

		m_pMainCommandList->ClearDepthStencilView(m_screenDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		m_pMainCommandList->ClearDepthStencilView(m_sceneDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		PIXEndEvent(m_pMainCommandList.Get());


		PIXBeginEvent(m_pMainCommandList.Get(),0,L"Init Shadow Textures and Set Barriers");
		UINT nLights[] = { m_pLightCb->GetDirLightsSize(), m_pLightCb->GetCubeLightsSize(), m_pLightCb->GetSpotLightsSize() };
		UINT nLightsTotal = nLights[static_cast<UINT>(eLIGHT_TYPE::DIR)] 
			+ nLights[static_cast<UINT>(eLIGHT_TYPE::CUBE)] 
			+ nLights[static_cast<UINT>(eLIGHT_TYPE::SPT)];
		//Clear ShadowMap	
		if (nLightsTotal)
		{
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList.Get(), nLights,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList.Get(), nLights,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, false);

			m_pShadowMap->ClearRtv(m_pMainCommandList.Get(), nLights);
			m_pShadowMap->ClearDsv(m_pMainCommandList.Get(), nLights);
		}
		PIXEndEvent(m_pMainCommandList.Get());
		

		m_pMainCommandList->Close();
		m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pMainCommandList.GetAddressOf()));

		UINT fenceValue = m_fenceValue++;
		m_pMainCommandQueue->Signal(m_pFence.Get(), fenceValue);
		
		m_nDrawCall = 0;
		UINT nfrustumCull=0;
		UINT nPassed = 0;
		
		Frustum12* pFrustum =m_pCam->GetFrustum();
		XMVECTOR* pPlanes = pFrustum->GetPlanes();
		UINT totalMeshlet = 0;
		for (int i = 0; i < m_pTotalMeshes.size(); ++i)
		{
			std::vector<Meshlet*>& pMeshlets = m_pTotalMeshes[i]->GetMeshelets();
			UINT nMeshlet = pMeshlets.size();
			totalMeshlet += nMeshlet;
			m_pMeshletQueue[static_cast<UINT>(ePass::zPass)].reserve(nMeshlet);
			m_pMeshletQueue[static_cast<UINT>(ePass::hi_zPass)].reserve(nMeshlet);
			for (int j = 0; j < pMeshlets.size(); ++j)
			{
				Meshlet* pMeshlet = pMeshlets[j];
				AABB* aabb = pMeshlet->GetAABB();

				bool result = aabb->IsOnFrustum(pPlanes);
				if (result)
				{
					m_pMeshletQueue[static_cast<UINT>(ePass::zPass)].push_back(pMeshlet);
					m_pMeshletQueue[static_cast<UINT>(ePass::hi_zPass)].push_back(pMeshlet);
				}
			}
		
		}
		pFrustum->SetSubMeshesInScene(totalMeshlet);
		pFrustum->SetSubMeshesInFrustum(m_pMeshletQueue[static_cast<UINT>(ePass::zPass)].size());
		
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		m_pMainCommandAllocator->Reset();
		m_pMainCommandList->Reset(m_pMainCommandAllocator.Get(), nullptr);

		//Zpass

		HANDLE m_handles[_WORKER_THREAD_COUNT];
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			PIXBeginEvent(m_pWorkerCommandLists[i].Get(), 0, L"ZPass");
			SetEvent(m_workerBeginFrame[i]);
			m_handles[i] = m_workerFinishZpass[i];
		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_workerFinishZpass, TRUE, INFINITE);
		{
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				PIXEndEvent(m_pWorkerCommandLists[i].Get());
				m_pWorkerCommandLists[i]->Close();
			}

			m_pMainCommandQueue->ExecuteCommandLists(_WORKER_THREAD_COUNT,
				reinterpret_cast<ID3D12CommandList**>(m_pWorkerCommandLists[0].GetAddressOf()));

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				UINT fenceValue = m_workerFenceValues[i]++;
				m_pMainCommandQueue->Signal(m_pWorkerFences[i].Get(), fenceValue);
				if (m_pWorkerFences[i]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFences[i]->SetEventOnCompletion(fenceValue, m_workerFenceEvents[i]);
					WaitForSingleObject(m_workerFenceEvents[i], INFINITE);
				}

				HRESULT hr = m_pWorkerCommandAllocators[i]->Reset();
				assert(SUCCEEDED(hr));
				m_pWorkerCommandLists[i]->Reset(m_pWorkerCommandAllocators[i].Get(), nullptr);
			}
		}

		m_pMeshletQueue[static_cast<UINT>(ePass::zPass)].clear();
		m_lastMeshletIdx = 0;
		//ShadowPass 
		
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			PIXBeginEvent(m_pWorkerCommandLists[i].Get(), 0, L"ShadowPass");
			SetEvent(m_workerBeginShadowPass[i]);
			m_handles[i] = m_workerFinishShadowPass[i];

		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);
		{
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				PIXEndEvent(m_pWorkerCommandLists[i].Get());
				m_pWorkerCommandLists[i]->Close();
			}

			m_pMainCommandQueue->ExecuteCommandLists(_WORKER_THREAD_COUNT,
				reinterpret_cast<ID3D12CommandList**>(m_pWorkerCommandLists[0].GetAddressOf()));

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				UINT fenceValue = m_workerFenceValues[i]++;
				m_pMainCommandQueue->Signal(m_pWorkerFences[i].Get(), fenceValue);
				if (m_pWorkerFences[i]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFences[i]->SetEventOnCompletion(fenceValue, m_workerFenceEvents[i]);
					WaitForSingleObject(m_workerFenceEvents[i], INFINITE);
				}

				HRESULT hr = m_pWorkerCommandAllocators[i]->Reset();
				assert(SUCCEEDED(hr));
				m_pWorkerCommandLists[i]->Reset(m_pWorkerCommandAllocators[i].Get(), nullptr);
			}
		}
		



		m_lastMeshletIdx = 0;
		if (nLightsTotal)
		{
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList.Get(), nLights,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList.Get(), nLights,
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, false);
		}

		//Set-Up HiZPass
		UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE/2)));
		{
			PIXBeginEvent(m_pMainCommandList.Get(), 0, L"Gen Hi-Z");

			D3D12_VIEWPORT viewPort = {};
			viewPort.Width = _SHADOWMAP_SIZE;
			viewPort.Height = _SHADOWMAP_SIZE/2;

			D3D12_RECT scissorRect = {};
			scissorRect.right = _SHADOWMAP_SIZE;
			scissorRect.bottom = _SHADOWMAP_SIZE/2;
			//DownSample DepthMap
			D3D12_RESOURCE_BARRIER prepDownSampleZ[] =
			{
				CreateResourceBarrier(m_pSceneDepthTex.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			};
			m_pMainCommandList->ResourceBarrier(sizeof(prepDownSampleZ) / sizeof(D3D12_RESOURCE_BARRIER), prepDownSampleZ);

			m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pMainCommandList->SetPipelineState(m_pDownSamplePso.Get());
			m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetDownSampleRootSignature());
			m_pMainCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eDownSampleRP::psSampler), m_wrapSsv);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eDownSampleRP::depthMap), m_sceneDepthSrv);
			m_pMainCommandList->IASetVertexBuffers(0, 1, &m_quadVbv);
			m_pMainCommandList->IASetIndexBuffer(&m_quadIbv);
			m_pMainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pMainCommandList->RSSetViewports(1, &viewPort);
			m_pMainCommandList->RSSetScissorRects(1, &scissorRect);
			m_pMainCommandList->OMSetRenderTargets(1, &m_hiZTempRtvs[0], FALSE, nullptr);
			m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);

			D3D12_RESOURCE_BARRIER endDownSampleZ[] =
			{
				CreateResourceBarrier(m_pSceneDepthTex.Get(),D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE),
				CreateResourceBarrier(m_pHiZTempTex.Get(),D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,0),
			};
			m_pMainCommandList->ResourceBarrier(sizeof(endDownSampleZ) / sizeof(D3D12_RESOURCE_BARRIER), endDownSampleZ);
			//SetUp
			{
				m_pMainCommandList->SetPipelineState(m_pGenHiZPassPso.Get());
				m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetGenHiZpassRootSignature());
				m_pMainCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eGenHiZRP::psSampler), m_borderSsv);
				for (int i = 1; i < mipLevels; ++i)
				{
					m_pMainCommandList->RSSetViewports(1, &m_hiZViewPorts[i]);
					m_pMainCommandList->RSSetScissorRects(1, &m_hiZScissorRects[i]);
					m_pMainCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eGenHiZRP::psLastMip), m_hiZTempSrvs[i - 1]);
					m_pMainCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(eGenHiZRP::psLastResoltion), m_hi_zCbv[i - 1]);
					m_pMainCommandList->OMSetRenderTargets(1, &m_hiZTempRtvs[i], FALSE, nullptr);
					m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);
					D3D12_RESOURCE_BARRIER prepMip[] =
					{
						CreateResourceBarrier(m_pHiZTempTex.Get(),D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,i - 1),
						CreateResourceBarrier(m_pHiZTempTex.Get(),D3D12_RESOURCE_STATE_RENDER_TARGET,
							i + 1 == mipLevels ? D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,i),
					};

					m_pMainCommandList->ResourceBarrier(sizeof(prepMip) / sizeof(D3D12_RESOURCE_BARRIER), prepMip);
				}

				m_pMainCommandList->Close();
				m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pMainCommandList.GetAddressOf()));
				UINT fenceValue = m_fenceValue++;
				m_pMainCommandQueue->Signal(m_pFence.Get(), fenceValue);
				if (m_pFence->GetCompletedValue() < fenceValue)
				{
					m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
					WaitForSingleObject(m_fenceEvent, INFINITE);
				}
				m_pMainCommandAllocator->Reset();
				m_pMainCommandList->Reset(m_pMainCommandAllocator.Get(), nullptr);

			}
			m_nHiZPassed = 0;
			PIXEndEvent(m_pMainCommandList.Get());
		
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			PIXBeginEvent(m_pWorkerCommandLists[i].Get(), 0, L"Hi_ZTest");
			SetEvent(m_workerBeginHiZpass[i]);
			m_handles[i] = m_workerFinishHiZpass[i];
		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);
		{
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				PIXEndEvent(m_pWorkerCommandLists[i].Get());
				m_pWorkerCommandLists[i]->Close();
			}

			m_pMainCommandQueue->ExecuteCommandLists(_WORKER_THREAD_COUNT,
				reinterpret_cast<ID3D12CommandList**>(m_pWorkerCommandLists[0].GetAddressOf()));

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				UINT fenceValue = m_workerFenceValues[i]++;
				m_pMainCommandQueue->Signal(m_pWorkerFences[i].Get(), fenceValue);
				if (m_pWorkerFences[i]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFences[i]->SetEventOnCompletion(fenceValue, m_workerFenceEvents[i]);
					WaitForSingleObject(m_workerFenceEvents[i], INFINITE);
				}

				HRESULT hr = m_pWorkerCommandAllocators[i]->Reset();
				assert(SUCCEEDED(hr));
				m_pWorkerCommandLists[i]->Reset(m_pWorkerCommandAllocators[i].Get(), nullptr);
			}
		}
		//Set-Up HWOcclusion Pass
		{
			m_lastMeshletIdx = 0;
			std::vector<D3D12_RESOURCE_BARRIER> finishHiZtest;
			finishHiZtest.reserve(mipLevels);
			for (int i = 0; i < mipLevels; ++i)
			{
				finishHiZtest.push_back(
					CreateResourceBarrier(
						m_pHiZTempTex.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
						D3D12_RESOURCE_STATE_RENDER_TARGET, i));
			}

			m_pMainCommandList->ResourceBarrier(finishHiZtest.size(), &finishHiZtest[0]);
			

			m_pMainCommandList->Close();
			m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pMainCommandList.GetAddressOf()));

			UINT fenceValue = m_fenceValue++;
			m_pMainCommandQueue->Signal(m_pFence.Get(), fenceValue);
			
			UINT hwOcclusionPass = static_cast<UINT>(ePass::HWOcclusionPass);
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				UINT nHiZPassed= m_pHiZQueue[i].size();
				m_nHiZPassed+= nHiZPassed;
				m_pMeshletQueue[hwOcclusionPass].reserve(nHiZPassed);
				for (int j = 0; j < m_pHiZQueue[i].size(); ++j)
				{
					Meshlet* pMesh = m_pHiZQueue[i][j];
					m_pMeshletQueue[hwOcclusionPass].push_back(pMesh);
				}
				m_pHiZQueue[i].clear();
			}


			if (m_pFence->GetCompletedValue() < fenceValue)
			{
				m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}
			m_pMainCommandAllocator->Reset();
			m_pMainCommandList->Reset(m_pMainCommandAllocator.Get(), nullptr);

			m_pMeshletQueue[static_cast<UINT>(ePass::hi_zPass)].clear();
			m_lastMeshletIdx = 0;
		}
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			PIXBeginEvent(m_pWorkerCommandLists[i].Get(), 0, L"HWOcclusion");
			SetEvent(m_workerBeginHWOcclusionTestPass[i]);
			m_handles[i] = m_workerFinishHWOcclusionTestPass[i];

		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);
		{
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				PIXEndEvent(m_pWorkerCommandLists[i].Get());
				m_pWorkerCommandLists[i]->Close();
			}

			m_pMainCommandQueue->ExecuteCommandLists(_WORKER_THREAD_COUNT,
				reinterpret_cast<ID3D12CommandList**>(m_pWorkerCommandLists[0].GetAddressOf()));

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				UINT fenceValue = m_workerFenceValues[i]++;
				m_pMainCommandQueue->Signal(m_pWorkerFences[i].Get(), fenceValue);
				if (m_pWorkerFences[i]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFences[i]->SetEventOnCompletion(fenceValue, m_workerFenceEvents[i]);
					WaitForSingleObject(m_workerFenceEvents[i], INFINITE);
				}

				HRESULT hr = m_pWorkerCommandAllocators[i]->Reset();
				assert(SUCCEEDED(hr));
				m_pWorkerCommandLists[i]->Reset(m_pWorkerCommandAllocators[i].Get(), nullptr);
			}
		}

		m_pMeshletQueue[static_cast<UINT>(ePass::HWOcclusionPass)].clear();
		m_nNotOccluded = 0;
		UINT geoPass = static_cast<UINT>(ePass::geoPass);
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			UINT nMeshlets = m_pHWOcclusionQueue[i].size();
			m_nNotOccluded += nMeshlets;
			m_pMeshletQueue[geoPass].reserve(nMeshlets);
			for (int j = 0; j < m_pHWOcclusionQueue[i].size(); ++j)
			{
				Meshlet* pMesh = m_pHWOcclusionQueue[i][j];
				m_pMeshletQueue[geoPass].push_back(pMesh);
			}
			m_pHWOcclusionQueue[i].clear();

		}
		m_lastMeshletIdx = 0;

		//Draw PbrGeo
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			PIXBeginEvent(m_pWorkerCommandLists[i].Get(), 0, L"PBRGeo");
			SetEvent(m_workerBeginDeferredGeoPass[i]);
		}

		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_workerEndFrame, TRUE, INFINITE);
		{
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				PIXEndEvent(m_pWorkerCommandLists[i].Get());
				m_pWorkerCommandLists[i]->Close();
			}

			m_pMainCommandQueue->ExecuteCommandLists(_WORKER_THREAD_COUNT,
				reinterpret_cast<ID3D12CommandList**>(m_pWorkerCommandLists[0].GetAddressOf()));

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				UINT fenceValue = m_workerFenceValues[i]++;
				m_pMainCommandQueue->Signal(m_pWorkerFences[i].Get(), fenceValue);
				if (m_pWorkerFences[i]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFences[i]->SetEventOnCompletion(fenceValue, m_workerFenceEvents[i]);
					WaitForSingleObject(m_workerFenceEvents[i], INFINITE);
				}

				HRESULT hr = m_pWorkerCommandAllocators[i]->Reset();
				assert(SUCCEEDED(hr));
				m_pWorkerCommandLists[i]->Reset(m_pWorkerCommandAllocators[i].Get(), nullptr);
			}
		}
		m_pMeshletQueue[static_cast<UINT>(ePass::geoPass)].clear();
		m_lastMeshletIdx = 0;

		D3D12_RESOURCE_BARRIER PbrGeoPass_barriers[static_cast<UINT>(eGbuf::cnt)];
		for (int i = 0; i < static_cast<UINT>(eGbuf::cnt); ++i)
		{
			PbrGeoPass_barriers[i] = i > static_cast<UINT>(eGbuf::emissive) ?
				CreateResourceBarrier(m_pGBufTexs[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) :
				CreateResourceBarrier(m_pGBufTexs[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		m_pMainCommandList->ResourceBarrier(sizeof(PbrGeoPass_barriers)/sizeof(D3D12_RESOURCE_BARRIER), PbrGeoPass_barriers);
		
		m_pMainCommandList->Close();
		m_pMainCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pMainCommandList.GetAddressOf()));
		fenceValue = m_fenceValue++;
		m_pMainCommandQueue->Signal(m_pFence.Get(), fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		m_pMainCommandAllocator->Reset();
		m_pMainCommandList->Reset(m_pMainCommandAllocator.Get(), nullptr);


		SetEvent(m_ssaoBeginFrame);
		WaitForSingleObject(m_ssaoEndFrame, INFINITE);
		D3D12_RESOURCE_BARRIER ssaoEndBarriers[] = {
			CreateResourceBarrier(m_pSsaoBlurTex.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CreateResourceBarrier(m_pGBufTexs[static_cast<UINT>(eGbuf::vNormal)].Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CreateResourceBarrier(m_pGBufTexs[static_cast<UINT>(eGbuf::vPos)].Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		};

		m_pMainCommandList->ResourceBarrier(sizeof(ssaoEndBarriers) / sizeof(D3D12_RESOURCE_BARRIER) , ssaoEndBarriers);
		m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pMainCommandList->RSSetViewports(1, &m_viewport);
		m_pMainCommandList->RSSetScissorRects(1, &m_scissorRect);
		//Upload HeightScale and bHeightOnOff
		if (!m_pObjects.empty())
		{	
		
			//LightingPass
			PIXBeginEvent(m_pMainCommandList.Get(), 0, L"PbrLighting Pass");
			m_pMainCommandList->ExecuteBundle(m_pBundles[static_cast<UINT>(ePass::lightingPass)].Get());
			m_pCam->UploadCamPos(m_pMainCommandList.Get(), false);
			m_pCam->UploadCascadeLevels(m_pMainCommandList.Get());
			m_pMatricesCb->UploadProjMat(m_pMainCommandList.Get(), false);
			m_pMatricesCb->UploadViewMat(m_pMainCommandList.Get());
			m_pLightCb->UpdateLightBuffer(m_pMainCommandList.Get());
			m_pLightCb->UpdateDirLightMatrices(m_pMainCommandList.Get());
			m_pLightCb->UpdateSpotLightMatrices(m_pMainCommandList.Get());
			m_pMainCommandList->OMSetRenderTargets(1, &m_sceneRtv, TRUE, nullptr);
			m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);
			PIXEndEvent(m_pMainCommandList.Get());
		}
		

		//Draw SkyBox
		PIXBeginEvent(m_pMainCommandList.Get(), 0, L"DrawSkyBox");
		m_pMainCommandList->OMSetRenderTargets(1, &m_sceneRtv, TRUE, &m_sceneDsv);
		m_pMainCommandList->ExecuteBundle(m_pBundles[static_cast<UINT>(ePass::skyBoxPass)].Get());
		m_pCam->SetDirty(false);

		D3D12_RESOURCE_BARRIER postProcessPassBarriers[] = {
			CreateResourceBarrier(m_pSceneTex.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
		};
		m_pMainCommandList->ResourceBarrier(1, postProcessPassBarriers);
		PIXEndEvent(m_pMainCommandList.Get());
		//PostProcess
		PIXBeginEvent(m_pMainCommandList.Get(), 0, L"PostProcess");

		m_pMainCommandList->ExecuteBundle(m_pBundles[static_cast<UINT>(ePass::postProcess)].Get());
		PIXEndEvent(m_pMainCommandList.Get());
		//DrawUI
		
		D3D12_RESOURCE_BARRIER uiPassBarriers[] = {
				CreateResourceBarrier(m_pSceneTex.Get(),  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				CreateResourceBarrier(m_pViewportTex.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		};

		m_pMainCommandList->ResourceBarrier(2, uiPassBarriers);
		m_pMainCommandList->OMSetRenderTargets(1, &m_screenRtvs[m_curFrame], FALSE, &m_screenDsv);
		return;
}

void D3D12::UpdateSSAOParemeters()
{
	//SSAO Pass
	UINT8* pSSAOParametersCbBegin = m_pSsaoParametersCbBegin;
	memcpy(pSSAOParametersCbBegin, &m_ssaoRadius, sizeof(FLOAT));
	pSSAOParametersCbBegin += sizeof(FLOAT);
	memcpy(pSSAOParametersCbBegin, &m_ssaoBias, sizeof(FLOAT));
	pSSAOParametersCbBegin += sizeof(FLOAT);
	memcpy(pSSAOParametersCbBegin, &m_nSsaoSample, sizeof(UINT));
}

void D3D12::UpdateParllexMappingParemeters()
{
	memcpy(m_pHeightScaleCbBegin, &m_heightScale, sizeof(float));
}

void D3D12::UpdateExposureParemeters()
{
	memcpy(m_pExposureCbBegin, &m_exposure, sizeof(float));
}

	void D3D12::CreateUploadBuffer(ID3D12Resource** const ppUploadCB, const UINT64 uploadPitch, const UINT64 uploadSize)
{
	if ((*ppUploadCB)!=nullptr)
	{
		(*ppUploadCB)->Release();
	}
	HRESULT hr;
	D3D12_RESOURCE_DESC uploadCbDesc = {};
	uploadCbDesc.Alignment = 0;
	uploadCbDesc.Width = CUSTUM_ALIGN(uploadSize, _64KB);
	uploadCbDesc.Height = 1;
	uploadCbDesc.DepthOrArraySize = 1;
	uploadCbDesc.MipLevels = 1;
	uploadCbDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadCbDesc.SampleDesc.Count = 1;
	uploadCbDesc.SampleDesc.Quality = 0;
	uploadCbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadCbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	uploadCbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &uploadCbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(ppUploadCB));
	assert(SUCCEEDED(hr));
	
}

	void D3D12::UploadTexThroughCB(const D3D12_RESOURCE_DESC texDesc, const UINT rp,
	const UINT8* pData, ID3D12Resource* const pDst, ID3D12Resource** const ppUploadCB, ID3D12GraphicsCommandList* const pCommandList)
{
	HRESULT hr;
	D3D12_RESOURCE_BARRIER readOnlyToCopyDst = {};
	readOnlyToCopyDst.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	readOnlyToCopyDst.Transition.pResource = nullptr;
	readOnlyToCopyDst.Transition.Subresource= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	readOnlyToCopyDst.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	readOnlyToCopyDst.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

	readOnlyToCopyDst.Transition.pResource = pDst;
	pCommandList->ResourceBarrier(1, &readOnlyToCopyDst);
	
	UINT rowPitch = rp;
	UINT height = texDesc.Height;
	//4x4´ÜÀ§¶ó¼­ °¨¾ÈÇØÁà¾ßÇÔ. +3Àº 4ÀÇ¹è¼ö·Î ³ª´²¶³¾îÁö°Ô ÇÏ±â À§ÇÔ.
	if (DirectX::IsCompressed(texDesc.Format))
	{	
		height = (height + 3) / 4;
		if (texDesc.Format == DXGI_FORMAT_BC1_UNORM ||
			texDesc.Format == DXGI_FORMAT_BC1_UNORM_SRGB ||
			texDesc.Format == DXGI_FORMAT_BC1_TYPELESS)
		{
			rowPitch = (texDesc.Width + 3) / 4 * 8;
		}
		else
		{
			rowPitch = (texDesc.Width + 3) / 4 * 16;
		}
	}
	UINT64 uploadPitch = (rp + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
	UINT64 uploadSize = uploadPitch * height;
	CreateUploadBuffer(ppUploadCB, uploadPitch, uploadSize);


	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	D3D12_RESOURCE_DESC copytexDesc= texDesc;
	UINT numRows;
	UINT64 rowSizeInByte;
	UINT64 totalBytes;
	m_pDevice->GetCopyableFootprints(&copytexDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInByte, &totalBytes);
	UINT8* pUploadCbPtr;
	D3D12_RANGE readRange = { 0, };
	hr = (*ppUploadCB)->Map(0, &readRange, reinterpret_cast<void**>(&pUploadCbPtr));
	assert(SUCCEEDED(hr));
	//Padding ‹š¹®¿¡ memcpyÇÑÁÙ·Î Ã³¸® ºÒ°¡
	for (UINT64 y = 0; y < height; ++y)
	{
		memcpy((void*)(pUploadCbPtr + y * uploadPitch),
			(UINT8*)pData + y * rp, rp);
	}

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.pResource = pDst;

	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = (*ppUploadCB);
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint = footprint;

	pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	(*ppUploadCB)->Unmap(0, 0);
	return;
}

	void D3D12::DrawScene()
	{	
		ID3D12CommandList* ppCommandList[] = { m_pMainCommandList.Get() };
		ImGuiIO& io = ImGui::GetIO();
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pMainCommandList.Get());
		D3D12_RESOURCE_BARRIER rtvToPresent = 
			CreateResourceBarrier(m_pScreenTexs[m_curFrame].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_pMainCommandList->ResourceBarrier(1, &rtvToPresent);
		m_pMainCommandList->Close();
		m_pMainCommandQueue->ExecuteCommandLists(sizeof(ppCommandList) / sizeof(ID3D12CommandList*), ppCommandList);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, reinterpret_cast<void*>(m_pMainCommandList.Get()));
		}

		UINT fenceValue = m_fenceValue++;
		m_pMainCommandQueue->Signal(m_pFence.Get(), fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		m_pMainCommandAllocator->Reset();
		m_pMainCommandList->Reset(m_pMainCommandAllocator.Get(), nullptr);

		if (m_bVsyncOn)
		{
			m_pSwapChain->Present(1, 0);
		}
		else
		{
			m_pSwapChain->Present(0, 0);
		}
		
		m_curFrame = ++m_curFrame%_BUFFER_COUNT;
	}

	D3D12::D3D12(const UINT screenWidth, const UINT screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
		float fScreenFar, float fScreenNear) :m_selectedObject(-1), m_exposure(1.0f), m_nSsaoSample(64), m_ssaoBias(0.025f), m_ssaoRadius(0.5f),
		m_lastMeshletIdx(0)
	{  
		g_pD3D12 = this;

		m_pHeapManager = nullptr;
		m_pCam = nullptr;
		m_pLightCb = nullptr;
		m_pMatricesCb = nullptr;
		m_pShader = nullptr;
		m_pShadowMap = nullptr;

		m_pHeightScaleCbBegin = nullptr;
		m_pExposureCbBegin = nullptr;

		m_bVsyncOn = false;

		
		HRESULT hr;
		bool result;
#ifdef _DEBUG
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(m_pDebugController.GetAddressOf()));
		assert(SUCCEEDED(hr));
		m_pDebugController->EnableDebugLayer();
#endif
		IDXGIFactory* pFactory;
		hr = CreateDXGIFactory(IID_PPV_ARGS(&pFactory));
		assert(SUCCEEDED(hr));
		pFactory->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::pFactory") - 1, "D3D12::pFactory");

		IDXGIAdapter* pAdapter;
		hr = pFactory->EnumAdapters(0, &pAdapter);
		assert(SUCCEEDED(hr));
		pAdapter->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::pAdapter") - 1, "D3D12::pAdaper");

		DXGI_ADAPTER_DESC adapterDesc = {};
		hr = pAdapter->GetDesc(&adapterDesc);
		assert(SUCCEEDED(hr));

		IDXGIOutput* pAdapterOutput;
		hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
		assert(SUCCEEDED(hr));
		pAdapterOutput->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::pAdapterOutput") - 1, "D3D12::pAdapterOutput");

		
		UINT numModes = {};
		hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
		assert(SUCCEEDED(hr));

		DXGI_MODE_DESC* pDisplayModeList = new DXGI_MODE_DESC[numModes];
		assert(SUCCEEDED(hr));

		hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, pDisplayModeList);
		assert(SUCCEEDED(hr));

		m_clientWidth = screenWidth;
		m_clientHeight = screenHeight;


		hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_pDevice.GetAddressOf()));
		assert(SUCCEEDED(hr));
		m_pDevice->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pDevice") - 1, "D3D12::m_pDevice");
		

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_pMainCommandQueue.GetAddressOf()));
		assert(SUCCEEDED(hr));
		m_pMainCommandQueue->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pMainCommandQueue") - 1, "D3D12::m_pMainCommandQueue");


		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_pComputeCommandQueue.GetAddressOf()));
		assert(SUCCEEDED(hr));
		m_pComputeCommandQueue->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pComputeCommandQueue") - 1, "D3D12::m_pComputeCommandQueue");

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		swapChainDesc = {};
		swapChainDesc.BufferCount = _BUFFER_COUNT;
		swapChainDesc.BufferDesc.Width = m_clientWidth;
		swapChainDesc.BufferDesc.Height = m_clientHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (m_bVsyncOn)
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = _REFRESH_RATE;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}
		else
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.OutputWindow = hWnd;

		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		swapChainDesc.Windowed = bFullscreen ? FALSE : TRUE;

		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = 0;

		hr = pFactory->CreateSwapChain(m_pMainCommandQueue.Get(), &swapChainDesc, m_pSwapChain.GetAddressOf());
		assert(SUCCEEDED(hr));
		m_pSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pSwapChain") - 1, "D3D12::m_pSwapChain");
		

		delete[] pDisplayModeList;
		pDisplayModeList = nullptr;

		pAdapterOutput->Release();
		pAdapterOutput = nullptr;

		pAdapter->Release();
		pAdapter = nullptr;

		pFactory->Release();
		pFactory = nullptr;

		//Gen DescriptorHeap
		m_pHeapManager = std::make_unique<HeapManager>(m_pDevice.Get());

		ImGui_ImplDX12_Init(m_pDevice.Get(), _BUFFER_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM, *(m_pHeapManager->GetCbvSrvUavHeap()),
			m_pHeapManager->GetCurCbvSrvCpuHandle(),
			m_pHeapManager->GetCurCbvSrvGpuHandle());
		m_pHeapManager->IncreaseCbvSrvHandleOffset();

		//Gen Rtvs;
		{

			for (UINT i = 0; i < _BUFFER_COUNT; ++i)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pHeapManager->GetCurRtvHandle();
				hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pScreenTexs[i].GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pDevice->CreateRenderTargetView(m_pScreenTexs[i].Get(), nullptr, rtvHandle);
				m_screenRtvs[i] = rtvHandle;
				m_pHeapManager->IncreaseRtvHandleOffset();

				m_pScreenTexs[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pScreenTexs") - 1, "D3D12::m_pScreenTexs");

			}


		}
		
		//Create CommandAllocator and Commandlist
		{
			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_pMainCommandAllocator.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pMainCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pMainCommandAllocator") - 1, "D3D12::m_pMainCommandAllocator");

			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
				IID_PPV_ARGS(m_pSsaoCommandAllocator.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pSsaoCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSsaoCommandAllocator") - 1, "D3D12::m_pSsaoCommandAllocator");

			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_pPbrSetupCommandAllocator.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pPbrSetupCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pPbrSetupCommandAllocator") - 1, "D3D12::m_pPbrSetupCommandAllocator");

			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_pCopyCommandAllocator.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pCopyCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pCopyCommandAllocator") - 1, "D3D12::m_pCopyCommandAllocator");


			hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pMainCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_pMainCommandList.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pMainCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pMainCommandList") - 1, "D3D12::m_pMainCommandList");


			hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_pSsaoCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_pSsaoCommandList.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pSsaoCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSsaoCommandList") - 1, "D3D12::m_pSsaoCommandList");

			hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pPbrSetupCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_pPbrSetupCommandList.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pPbrSetupCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pPbrSetupCommandList") - 1, "D3D12::m_pPbrSetupCommandList");

			hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCopyCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_pCopyCommandList.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pCopyCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pCopyCommandList") - 1, "D3D12::m_pCopyCommandList");

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pWorkerCommandAllocators[i]));
				assert(SUCCEEDED(hr));
				m_pWorkerCommandAllocators[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pWorkerCommandAllocators[i]") - 1, "D3D12::m_pWorkerCommandAllocators[i]");

				hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pWorkerCommandAllocators[i].Get(), nullptr, IID_PPV_ARGS(m_pWorkerCommandLists[i].GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pWorkerCommandLists[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pWorkerCommandLists[i]") - 1, "D3D12::m_pWorkerCommandLists[i]");
			}

			for (int i = 0; i < static_cast<UINT>(ePass::cnt); ++i)
			{
				hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&m_pBundleAllocators[i]));
				assert(SUCCEEDED(hr));
				m_pBundleAllocators[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pBundleAllocators") - 1, "D3D12::m_pBundleAllocators");
				hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_pBundleAllocators[i].Get(), nullptr, IID_PPV_ARGS(m_pBundles[i].GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pBundles[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pBundles") - 1, "D3D12::m_pBundles");
			}


		}
		
		//Gen User-Defined Class
		{
			m_pShader = std::make_unique<Shader12>(m_pDevice.Get());
			m_pCam = std::make_unique<Camera12>(m_pDevice.Get(), m_pMainCommandList.Get(), m_pHeapManager.get(), screenWidth, screenHeight, fScreenFar, fScreenNear);
			XMMATRIX* m_projMat = m_pCam->GetProjectionMatrix();
			XMMATRIX* m_viewMat = m_pCam->GetViewMatrix();
			m_pMatricesCb = std::make_unique<MatrixHandler12>(m_pDevice.Get(), m_pMainCommandList.Get(), m_pHeapManager.get(), m_viewMat, m_projMat);
	
			m_pLightCb = std::make_unique<LightBuffer12>(m_pDevice.Get(), m_pMainCommandList.Get(), m_pHeapManager.get());

			m_pShadowMap = std::make_unique<ShadowMap12>(m_pDevice.Get(), m_pMainCommandList.Get(),
				m_pHeapManager.get(),
				_SHADOWMAP_SIZE, _SHADOWMAP_SIZE, m_pCam->GetCascadeLevels().size(),
				m_pLightCb->GetDirLightsCapacity(), m_pLightCb->GetCubeLightsCapacity(), m_pLightCb->GetSpotLightsCapacity());
		}

		//Init Viewports and Rects
		{
			m_viewport.Width = static_cast<float>(screenWidth);
			m_viewport.Height = static_cast<float>(screenHeight);
			m_viewport.MinDepth = 0.0f;
			m_viewport.MaxDepth = 1.0f;
			m_viewport.TopLeftX = 0.0f;
			m_viewport.TopLeftY = 0.0f;

			m_scissorRect.left = 0;
			m_scissorRect.top = 0;
			m_scissorRect.right = m_viewport.Width;
			m_scissorRect.bottom = m_viewport.Height;

			m_diffIrradViewport = m_viewport;
			m_diffIrradViewport.Width = _DIFFIRRAD_WIDTH;
			m_diffIrradViewport.Height = _DIFFIRRAD_HEIGHT;

			m_diffIrradRect = m_scissorRect;
			m_diffIrradRect.right = m_diffIrradViewport.Width;
			m_diffIrradRect.bottom = m_diffIrradViewport.Height;

			m_prefilterViewport = m_viewport;
			m_prefilterViewport.Width = _PREFILTER_WIDTH;
			m_prefilterViewport.Height = _PREFILTER_HEIGHT;

			m_prefilterRect = m_scissorRect;
			m_prefilterRect.right = m_prefilterViewport.Width;
			m_prefilterRect.bottom = m_prefilterViewport.Height;

			UINT curWidth = _SHADOWMAP_SIZE;
			UINT curHeight = _SHADOWMAP_SIZE/2;
			UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE/2)));
			m_hiZViewPorts.resize(mipLevels + 1);
			m_hiZScissorRects.resize(mipLevels + 1);
			for (int i = 0; i < mipLevels; ++i)
			{
				D3D12_VIEWPORT viewPort = {};
				viewPort.Width = curWidth;
				viewPort.Height = curHeight;

				D3D12_RECT scissorRect = {};
				scissorRect.right = curWidth;
				scissorRect.bottom = curHeight;

				m_hiZViewPorts[i] = viewPort;
				m_hiZScissorRects[i] = scissorRect;

				curWidth /= 2;
				curHeight/=2;
				curWidth = max(1, curWidth);
				curHeight = max(1, curHeight);

			}
		}

		D3D12_RASTERIZER_DESC geoRDesc = {};
		D3D12_RASTERIZER_DESC quadRDesc = {};
		D3D12_RASTERIZER_DESC AABBRDesc = {}; 
		D3D12_RASTERIZER_DESC skyboxRD = {};
		//Gen RS
		{
			geoRDesc.AntialiasedLineEnable = false;
			geoRDesc.CullMode = D3D12_CULL_MODE_BACK;
			geoRDesc.DepthBias = 0.0f;
			geoRDesc.DepthBiasClamp = 0.0f;
			geoRDesc.DepthClipEnable = true;
			geoRDesc.FillMode = D3D12_FILL_MODE_SOLID;
			geoRDesc.FrontCounterClockwise = true;
			geoRDesc.MultisampleEnable = false;
			geoRDesc.SlopeScaledDepthBias = 0.0f;
			
			quadRDesc = geoRDesc;
			quadRDesc.FrontCounterClockwise = false;

			AABBRDesc = geoRDesc;
			AABBRDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

			skyboxRD.FillMode = D3D12_FILL_MODE_SOLID;
			skyboxRD.CullMode = D3D12_CULL_MODE_BACK;

		}

		D3D12_RENDER_TARGET_BLEND_DESC GBufferWriteRTBlendDesc = {};
		D3D12_RENDER_TARGET_BLEND_DESC lightingPassBlendDesc = {};
		D3D12_RENDER_TARGET_BLEND_DESC defaultBlendDesc = {};
		D3D12_BLEND_DESC GBufferBS = {};
		D3D12_BLEND_DESC lightingBS = {};
		D3D12_BLEND_DESC defaultBS = {};
		//Gen BS
		{

			GBufferWriteRTBlendDesc.BlendEnable = TRUE;
			GBufferWriteRTBlendDesc.SrcBlend = D3D12_BLEND_ONE;
			GBufferWriteRTBlendDesc.DestBlend = D3D12_BLEND_ZERO;
			GBufferWriteRTBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
			GBufferWriteRTBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			GBufferWriteRTBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
			GBufferWriteRTBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			GBufferWriteRTBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			GBufferBS = { FALSE, FALSE, GBufferWriteRTBlendDesc };

			lightingPassBlendDesc = GBufferWriteRTBlendDesc;
			lightingPassBlendDesc.BlendEnable = TRUE;
			lightingPassBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;

			lightingBS = { FALSE, FALSE, lightingPassBlendDesc };

			defaultBlendDesc = GBufferWriteRTBlendDesc;
			defaultBlendDesc.BlendEnable = FALSE;

			defaultBS = { FALSE, FALSE, defaultBlendDesc };
		}

		D3D12_DEPTH_STENCIL_DESC defaultDss = {};
		D3D12_DEPTH_STENCIL_DESC offDss = {};
		D3D12_DEPTH_STENCIL_DESC DepthTestOnlyDss = {};
		D3D12_DEPTH_STENCIL_DESC outlinerSetupDss = {};
		D3D12_DEPTH_STENCIL_DESC outlinerTestDss = {};
		//Gen DSS
		{	

			defaultDss.DepthEnable = TRUE;
			defaultDss.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			defaultDss.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			defaultDss.StencilEnable = FALSE;
			defaultDss.StencilReadMask = 0xFF;
			defaultDss.StencilWriteMask = 0xFF;
			defaultDss.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			defaultDss.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			defaultDss.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			defaultDss.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
			defaultDss.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			defaultDss.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			defaultDss.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			defaultDss.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;

			DepthTestOnlyDss.DepthEnable = TRUE;
			DepthTestOnlyDss.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			DepthTestOnlyDss.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			//pso¿¡ ¼³Á¤, com °³Ã¼°¡ ¾Æ´Ô.

			outlinerSetupDss.DepthEnable = true;
			outlinerSetupDss.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			outlinerSetupDss.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			outlinerSetupDss.StencilEnable = true;
			outlinerSetupDss.StencilReadMask = 0xFF;
			outlinerSetupDss.StencilWriteMask = 0xFF;
			outlinerSetupDss.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
			outlinerSetupDss.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			outlinerSetupDss.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			outlinerSetupDss.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			outlinerSetupDss.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
			outlinerSetupDss.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
			outlinerSetupDss.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			outlinerSetupDss.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

			outlinerTestDss = outlinerSetupDss;
			outlinerTestDss.DepthEnable = true;
			outlinerTestDss.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			outlinerTestDss.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
			outlinerTestDss.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
			outlinerTestDss.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		}

		//Gen PSO
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = defaultDss;
			psoDesc.SampleMask = UINT_MAX; //BlendDescÀÇ ±×°Í,  °íÁ¤
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] =DXGI_FORMAT_R32_FLOAT;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			psoDesc.SampleDesc.Count = 1; //AA±â´É Ãß°¡ Àü±îÁø °íÁ¤

			psoDesc.pRootSignature = m_pShader->GetZpassRootSignature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetZpassShader(&psoDesc);
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pZpassPso));
			assert(SUCCEEDED(hr));
			m_pZpassPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pZpassPso") - 1, "D3D12::m_pZpassPso");

			psoDesc.DepthStencilState = offDss;
			psoDesc.pRootSignature = m_pShader->GetDownSampleRootSignature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetDownSampleShader(&psoDesc);
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pDownSamplePso));
			assert(SUCCEEDED(hr));
			m_pDownSamplePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pDownSamplePso") - 1, "D3D12::m_pDownSamplePso");

			psoDesc.pRootSignature = m_pShader->GetZpassRootSignature();
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DepthStencilState = DepthTestOnlyDss;
			m_pShader->SetHWOcclusionQueryShader(&psoDesc);
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pHWOcclusionQueryPso));
			assert(SUCCEEDED(hr));
			m_pHWOcclusionQueryPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pHWOcclusionQueryPso") - 1, "D3D12::m_pHWOcclusionQueryPso");


			psoDesc.DepthStencilState = offDss;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
			psoDesc.pRootSignature = m_pShader->GetGenHiZpassRootSignature();
			m_pShader->SetGenHiZpassShader(&psoDesc);
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pGenHiZPassPso));
			assert(SUCCEEDED(hr));
			m_pGenHiZPassPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pGenHiZPassPso") - 1, "D3D12::m_pGenHiZPassPso");



			psoDesc.DepthStencilState = defaultDss;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.pRootSignature = m_pShader->GetCascadeDirShadowRootSingnature();
			m_pShader->SetPosOnlyInputLayout(&psoDesc);
			m_pShader->SetCascadeDirShadowShader(&psoDesc);
			hr= m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pCascadeDirShadowPso));
			assert(SUCCEEDED(hr));
			m_pCascadeDirShadowPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pCascadeDirShadowPso") - 1, "D3D12::m_pCascadeDirShadowPso");



			psoDesc.pRootSignature = m_pShader->GetSpotShadowRootSingnature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetSpotShadowShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pSpotShadowPso));
			assert(SUCCEEDED(hr));
			m_pSpotShadowPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSpotShadowPso") - 1, "D3D12::m_pSpotShadowPso");


			psoDesc.RasterizerState = skyboxRD;
			psoDesc.pRootSignature= m_pShader->GetCubeShadowRootSingnature();
			m_pShader->SetCubeShadowShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pCubeShadowPso));
			assert(SUCCEEDED(hr));
			m_pCubeShadowPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pCubeShadowPso") - 1, "D3D12::m_pCubeShadowPso");


			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.pRootSignature = m_pShader->GetSkyBoxRootSingnature();
			psoDesc.DepthStencilState = DepthTestOnlyDss;
			m_pShader->SetSkyBoxShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pSkyBoxPso));
			assert(SUCCEEDED(hr));
			m_pSkyBoxPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSkyBoxPso") - 1, "D3D12::m_pSkyBoxPso");


			for (int i = 0; i < static_cast<UINT>(eGbuf::cnt); ++i)
			{	
				if (i > static_cast<UINT>(eGbuf::normal) && i < static_cast<UINT>(eGbuf::vPos))
				{
					psoDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
				}
				else
				{
					psoDesc.RTVFormats[i] = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}
			}
			
			psoDesc.pRootSignature = m_pShader->GetPbrDeferredGeoShaderRootSingnature();
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.DepthStencilState =DepthTestOnlyDss;
			psoDesc.BlendState = defaultBS;//GBufferBS;
			psoDesc.NumRenderTargets = static_cast<UINT>(eGbuf::cnt);
			m_pShader->SetDeferredGeoLayout(&psoDesc);
			m_pShader->SetPbrDeferredGeoShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoPso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoPso") - 1, "D3D12:::m_pPbrDeferredGeoPso");
			m_pShader->SetPbrDeferredGeoEmissiveShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoEmissivePso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoEmissivePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoEmissivePso") - 1, "D3D12:::m_pPbrDeferredGeoEmissivePso");
			m_pShader->SetPbrDeferredGeoNormalShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalPso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoNormalPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalPso") - 1, "D3D12:::m_pPbrDeferredGeoNormalPso");
			m_pShader->SetPbrDeferredGeoNormalEmissiveShader(&psoDesc);
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalEmissivePso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoNormalEmissivePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalEmissivePso") - 1, "D3D12:::m_pPbrDeferredGeoNormalEmissivePso");
			m_pShader->SetPbrDeferredGeoNormalHeightShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalHeightPso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoNormalHeightPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalHeightPso") - 1, "D3D12:::m_pPbrDeferredGeoNormalHeightPso");
			m_pShader->SetPbrDeferredGeoNormalHeightEmissiveShader(&psoDesc);
			hr =m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalHeightEmissivePso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoNormalHeightEmissivePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalHeightEmissivePso") - 1, "D3D12:::m_pPbrDeferredGeoNormalHeightEmissivePso");
			
			//GeoPass¿¡¼­ picking ÇÑ ¸ðµ¨¸¸  Stencil°ªÀ» Ã³¸®ÇØÁÖ±â ‹š¹®¿¡ pso¿¡¼­ DSS¸¸ ¹Ù²ãÁÜ. 
			psoDesc.DepthStencilState = outlinerSetupDss;
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pOutlinerSetupPso));
			assert(SUCCEEDED(hr));

			for (int i = 1; i < static_cast<UINT>(eGbuf::cnt); ++i)
			{
				psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			}

			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DepthStencilState = offDss;
			psoDesc.BlendState = lightingBS;
			psoDesc.NumRenderTargets = 1;
			psoDesc.pRootSignature = m_pShader->GetPbrDeferredLightingShaderRootSingnature();
			m_pShader->SetPbrDeferredLightingShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredLightingPso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredLightingPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pPbrDeferredLightingPso") - 1, "D3D12::m_pPbrDeferredLightingPso");

			psoDesc.pRootSignature = m_pShader->GetAabbShaderRootSingnature();
			psoDesc.RasterizerState = AABBRDesc;
			m_pShader->SetPosOnlyInputLayout(&psoDesc);
			m_pShader->SetAabbShader(&psoDesc);
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pAabbPso));
			assert(SUCCEEDED(hr));
			m_pAabbPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pAabbPso") - 1, "D3D12::m_pAabbPso");
			//OutlinerTest
			psoDesc.NumRenderTargets = 1;
			psoDesc.BlendState = defaultBS;
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.DepthStencilState = outlinerTestDss;
			psoDesc.pRootSignature = m_pShader->GetOutlinerTestShaderRootSingnature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetOutlinerTestShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pOutlinerTestPso));
			assert(SUCCEEDED(hr));
			m_pOutlinerTestPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pOutlinerTestPso") - 1, "D3D12::m_pOutlinerTestPso");



			//BRDF
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
			psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.pRootSignature = m_pShader->GetBrdfRootSingnature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetBrdfShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pBrdfPso));
			assert(SUCCEEDED(hr));
			m_pBrdfPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pBrdfPso") - 1, "D3D12::m_pBrdfPso");
			//Prefilter
			psoDesc.pRootSignature = m_pShader->GetPrefilterRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetPosOnlyInputLayout(&psoDesc);
			m_pShader->SetPrefilterShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPrefilterPso));
			assert(SUCCEEDED(hr));
			m_pPrefilterPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pPrefilterPso") - 1, "D3D12::m_pPrefilterPso");
			//DiffuseIrradiance
			psoDesc.pRootSignature = m_pShader->GetDiffuseIrradianceRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetDiffuseIrradianceShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pDiffuseIrradiancePso));
			assert(SUCCEEDED(hr));
			m_pDiffuseIrradiancePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pDiffuseIrradiancePso") - 1, "D3D12::m_pDiffuseIrradiancePso");
			//Equirect2Cube
			psoDesc.pRootSignature = m_pShader->GetEquirect2CubeRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetEquirect2CubeShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pEquirect2CubePso));
			assert(SUCCEEDED(hr));
			m_pEquirect2CubePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pEquirect2CubePso") - 1, "D3D12::m_pEquirect2CubePso");


			D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
			computePsoDesc.pRootSignature = m_pShader->GetHiZCullPassRootSignature();
			m_pShader->SetHiZCullPassShader(&computePsoDesc);
			hr = m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pHiZCullPassPso));
			assert(SUCCEEDED(hr));
			m_pHiZCullPassPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pHiZCullPassPso") - 1, "D3D12::m_pHiZCullPassPso");


			computePsoDesc.pRootSignature = m_pShader->GetGenMipShaderRootSingnature();
			m_pShader->SetGenMipShader(&computePsoDesc);
			hr=m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pGenMipmapPso));
			assert(SUCCEEDED(hr));
			m_pGenMipmapPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pGenMipmapPso") - 1, "D3D12::m_pGenMipmapPso");


			computePsoDesc.pRootSignature = m_pShader->GetSsaoShaderRootSingnature();
			m_pShader->SetSsaoShader(&computePsoDesc);
			hr = m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pSsaoPso));
			assert(SUCCEEDED(hr));
			m_pSsaoPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSsaoPso") - 1, "D3D12::m_pSsaoPso");


			computePsoDesc.pRootSignature = m_pShader->GetSsaoBlurShaderRootSingnature();
			m_pShader->SetSsaoBlurShader(&computePsoDesc);
			hr = m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pSsaoBlurPso));
			assert(SUCCEEDED(hr));
			m_pSsaoBlurPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSsaoBlurPso") - 1, "D3D12::m_pSsaoBlurPso");

			computePsoDesc.pRootSignature = m_pShader->GetPostProcessShaderRootSingnature();
			m_pShader->SetPostProcessShader(&computePsoDesc);
			hr = m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pPostProcessPso));
			assert(SUCCEEDED(hr));
			m_pPostProcessPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pPostProcessPso") - 1, "D3D12::m_pPostProcessPso");

		}
		//Gen Fence
		{
			hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pFence->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pFence") - 1, "D3D12::m_pFence");
			m_fenceValue = 1;

			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(m_fenceEvent!=nullptr);

			hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pPbrSetupFence.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pPbrSetupFence->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pPbrSetupFence") - 1, "D3D12::m_pPbrSetupFence");
			m_pbrSetupFenceValue = 1;

			m_pbrSetupEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(m_pbrSetupEvent != nullptr);

			hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pSsaoFence.GetAddressOf()));
			assert(SUCCEEDED(hr));
			m_pSsaoFence->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSsaoFence") - 1, "D3D12::m_pSsaoFence");
			m_ssaoFenceValue = 1;

			m_ssaoFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(m_ssaoFenceEvent != nullptr);

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pWorkerFences[i].GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pWorkerFences[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pWorkerFences") - 1, "D3D12::m_pWorkerFences");
				m_workerFenceValues[i] = 1;

				m_workerFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				assert(m_workerFenceEvents[i] != nullptr);

			}
		}

		//Gen Samplers
		{
			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = m_pHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = m_pHeapManager->GetCurSamplerGpuHandle();

				D3D12_SAMPLER_DESC wrapSamplerDesc = {};
				wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				m_pDevice->CreateSampler(&wrapSamplerDesc, samplerCpuHandle);

				m_wrapSsv = samplerGpuHandle;
				m_pHeapManager->IncreaseSamplerHandleOffset();
			}
			

			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = m_pHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = m_pHeapManager->GetCurSamplerGpuHandle();

				D3D12_SAMPLER_DESC clampSamplerDesc = {};
				clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				m_pDevice->CreateSampler(&clampSamplerDesc, samplerCpuHandle);

				m_clampSsv = samplerGpuHandle;
				m_pHeapManager->IncreaseSamplerHandleOffset();
			}

			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = m_pHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = m_pHeapManager->GetCurSamplerGpuHandle();

				D3D12_SAMPLER_DESC borderSamplerDesc = {};
				borderSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				borderSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
				borderSamplerDesc.AddressV = borderSamplerDesc.AddressU;
				borderSamplerDesc.AddressW = borderSamplerDesc.AddressU;
				borderSamplerDesc.MaxLOD = 16.0f;
				m_pDevice->CreateSampler(&borderSamplerDesc, samplerCpuHandle);

				m_borderSsv = samplerGpuHandle;
				m_pHeapManager->IncreaseSamplerHandleOffset();
			}

		}

		//Gen ConstantBuffers
		{
			D3D12_RANGE readRange = {};
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC cbufferDesc = {};
			cbufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			cbufferDesc.Alignment = 0;
			cbufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			cbufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			cbufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			cbufferDesc.Height = 1;
			cbufferDesc.DepthOrArraySize = 1;
			cbufferDesc.MipLevels = 1;
			cbufferDesc.SampleDesc.Count = 1;
			cbufferDesc.SampleDesc.Quality = 0;		
		

			UINT cbSize = 0;
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				cbSize = (sizeof(DirectX::XMVECTOR)) * _HI_Z_CULL_COUNT * 8;
				m_pAabbCbBegin[i] = m_pHeapManager->GetCbMappedPtr(cbSize);
				m_aabbCbv[i] = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());


				cbSize = sizeof(XMMATRIX) * _HI_Z_CULL_COUNT;
				m_pHiZCullMatrixCbBegin[i] = m_pHeapManager->GetCbMappedPtr(cbSize);
				m_hiZCullMatrixCbv[i] = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());
			}
			
			cbSize = sizeof(DirectX::XMVECTOR);
			m_pRoughnessCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_roughnessCbv = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());
			
			cbSize = sizeof(SamplePoints);
			m_pSsaoKernalCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_ssaoKernelCbv = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());
			
			cbSize = sizeof(FLOAT) + sizeof(UINT) * 2;
			m_pSsaoParametersCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_ssaoParameterCbv = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());

			cbSize = sizeof(XMFLOAT4);
			m_pExposureCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_exposureCbv = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());

			cbSize = sizeof(XMFLOAT4);
			m_pHeightScaleCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_heightScaleCbv = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());
			
			cbSize = sizeof(XMMATRIX) * 6;
			m_pEquirect2CubeCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_equirect2CubeCbv = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());
		
			cbSize = sizeof(UINT) * 2;
			m_pResolutionCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_resolutionCbv = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());

			cbSize = sizeof(UINT) * 4;
			UINT lastWidth = _SHADOWMAP_SIZE;
			UINT lastHeight = _SHADOWMAP_SIZE/2;
			for (int i = 0; i < _HI_Z_TEX_COUNT; ++i)
			{
				m_pHiZResolutionCbBegin[i] = m_pHeapManager->GetCbMappedPtr(cbSize);
				m_hi_zCbv[i] = m_pHeapManager->GetCbv(cbSize, m_pDevice.Get());
				
				UINT lastMipInfo[4] = { lastWidth,lastHeight,i,0 };
				memcpy(m_pHiZResolutionCbBegin[i], lastMipInfo, cbSize);
				lastWidth /= 2;
				lastHeight /= 2;
				lastWidth = max(1, lastWidth);
				lastHeight = max(1, lastHeight);
			}

			D3D12_RESOURCE_BARRIER readTocopyDst =
				CreateResourceBarrier(m_pHeapManager->GetQueryBlock(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
			m_pMainCommandList->ResourceBarrier(1, &readTocopyDst);
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{					
				{
					UINT resultSize = _4KB;
					UINT64 offset = m_pHeapManager->GetQueryHeapOffset();
					m_queryResultOffsets[i] = offset;
					m_pHeapManager->SetQueryHeapOffset(offset + _4KB);
				}

				{
					cbufferDesc.Width = _64KB;
					heapProps.Type = D3D12_HEAP_TYPE_READBACK;
					hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
						&cbufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(m_pQueryReadBuffers[i].GetAddressOf()));
					assert(SUCCEEDED(hr));
					m_pQueryReadBuffers[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("D3D12::m_pQueryReadBuffers[i]") - 1, "D3D12::m_pQueryReadBuffers[i]");

					m_pHeapManager->IncreaseCbvSrvHandleOffset();

					readRange.End = cbufferDesc.Width;
					hr = m_pQueryReadBuffers[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pQueryReadCbBegins[i]));
					assert(SUCCEEDED(hr));
				}
				
			}
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				cbufferDesc.Width = CUSTUM_ALIGN(_HI_Z_CULL_COUNT * sizeof(UINT), _64KB);

				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pHeapManager->GetCurCbvSrvGpuHandle();

				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(m_pHiZCullReadCb[i].GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pHiZCullReadCb[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pHiZCullReadCb") - 1, "D3D12::m_pHiZCullReadCb");

				m_pHeapManager->IncreaseCbvSrvHandleOffset();

				readRange.End = cbufferDesc.Width;
				hr = m_pHiZCullReadCb[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pHiZCullReadCbBegin[i]));
				assert(SUCCEEDED(hr));
			}
			
		}
		//GenSSAOResources
		{	

			//³ëÀÌÁî¸¸ CRV_SRV_HEAP µû·Î °ü¸®
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;
			
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
				sampleV = DirectX::XMVector3Normalize(sampleV);
				sampleV = XMVectorScale(sampleV, randomFloats(gen));
				float scale = i /static_cast<float>( _KERNEL_COUNT);
				scale = 0.1f + (1.0f - 0.1f) * scale * scale;
				sampleV = XMVectorScale(sampleV, scale);

				kernels.coord[i] = sampleV;
			}

			memcpy(m_pSsaoKernalCbBegin, &kernels, sizeof(kernels));
			

			XMFLOAT3 noiseVecs[_NOISE_VEC_COUNT];
			//Gen noise texture
			for (int i = 0; i < _NOISE_VEC_COUNT; ++i)
			{
				XMFLOAT3 rot(
					randomFloats(gen) * 2.0f - 1.0f,
					randomFloats(gen) * 2.0f - 1.0f,
					0);
				noiseVecs[i]=rot;
			}
			D3D12_RESOURCE_DESC	texDesc = {};
			texDesc.Width = _NOISE_TEX_SIZE;
			texDesc.Height = _NOISE_TEX_SIZE;
			texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			texDesc.Alignment = 0;
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
				&m_pNoiseTex, m_pDevice.Get());
			m_pNoiseTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pNoiseTex") - 1, "D3D12:::m_pNoiseTex");

			
			UploadTexThroughCB(texDesc, sizeof(XMVECTOR)*_NOISE_TEX_SIZE, reinterpret_cast<UINT8*>(noiseVecs), 
				m_pNoiseTex.Get(), m_pNoiseUploadCb.GetAddressOf(), m_pMainCommandList.Get());
				
			D3D12_RESOURCE_BARRIER copyDstToSrvBarrier =
				CreateResourceBarrier(m_pNoiseTex.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			m_pMainCommandList->ResourceBarrier(1, &copyDstToSrvBarrier);


			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			m_noiseSrv = m_pHeapManager->GetSrv(srvDesc, m_pNoiseTex.Get(), m_pDevice.Get());
		}

		//GenQuad
		{
			XMFLOAT3 vertices[] = {
				{XMFLOAT3(-1.0f, -1.0f, 0.0f)},//left-down
				{XMFLOAT3(-1.0f, 1.0f, 0.0f)},//left-up
				{XMFLOAT3(1.0f, 1.0f, 0.0f)},//right-up
				{XMFLOAT3(1.0f, -1.0f, 0.0f)}//right-down
			};
			XMFLOAT2 texCoords[] = {
				{XMFLOAT2(0.0f, 1.0f)},
				{XMFLOAT2(0.0f, 0.0f)},
				{XMFLOAT2(1.0f, 0.0f)},
				{XMFLOAT2(1.0f, 1.0f)}
			};

			QUAD quads[4];
			for (int i = 0; i < 4; ++i)
			{
				quads[i].pos = vertices[i];
				quads[i].tex = texCoords[i];
			}

			UINT quadsVbSize = sizeof(quads);
			m_pHeapManager->AllocateVertexData(reinterpret_cast<UINT8*>(quads), quadsVbSize);
			m_quadVbv = m_pHeapManager->GetVbv(quadsVbSize, sizeof(QUAD));

			unsigned long indices[6] = { 0,1,2, 2,3,0 };
			UINT quadsIbSize = sizeof(indices);
			m_pHeapManager->AllocateIndexData(reinterpret_cast<UINT8*>(indices), quadsIbSize);
			m_quadIbv = m_pHeapManager->GetIbv(quadsIbSize, 0);
			UINT idx = m_pHeapManager->GetIbHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = m_pHeapManager->GetIbBlockOffset(idx);
			m_pHeapManager->SetIbBlockOffset(idx, curBlockOffset + m_quadIbv.SizeInBytes);
		}


		XMFLOAT3 vertices[] = {
			 {XMFLOAT3(-1.0f,  1.0f, -1.0f)},//front-upper-left  0
			 {XMFLOAT3(1.0f,   1.0f, -1.0f)},//front-upper-right 1
			 {XMFLOAT3(1.0f,  -1.0f, -1.0f)},//front-down-right  2
			 {XMFLOAT3(-1.0f, -1.0f, -1.0f)},//front-down-left   3

			 {XMFLOAT3(-1.0f,  1.0f, 1.0f)},//back-upper-left   4
			 {XMFLOAT3(1.0f,   1.0f, 1.0f)},//back-upper-right  5
			 {XMFLOAT3(1.0f,  -1.0f, 1.0f)},//back-down-right   6
			 {XMFLOAT3(-1.0f, -1.0f, 1.0f)} };//back-down-left   7
		//Gen Cube Buffer
		{

			UINT skyVbSize = sizeof(vertices);


			m_pHeapManager->AllocateVertexData(reinterpret_cast<UINT8*>(vertices), skyVbSize);
			m_skyBoxVbv = m_pHeapManager->GetVbv(skyVbSize, sizeof(XMFLOAT3));

			//½Ã°è¹æÇâ °¨Àº ¸éÀÌ  frontface
			unsigned long indices[] = {
				//front
				0,2,1,
				0,3,2,
				//back
				4,5,6,
				4,6,7,
				//left
				0,4,7,
				0,7,3,
				//right
				5,1,2,
				5,2,6,
				//top
				5,4,0,
				5,0,1,
				//bottom
				7,6,2,
				7,2,3
			};
			UINT skyIbSize = sizeof(indices);
			m_pHeapManager->AllocateIndexData(reinterpret_cast<UINT8*>(indices), skyIbSize);
			m_skyBoxIbv = m_pHeapManager->GetIbv(skyIbSize, 0);
			UINT idx = m_pHeapManager->GetIbHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = m_pHeapManager->GetIbBlockOffset(idx);
			m_pHeapManager->SetIbBlockOffset(idx,curBlockOffset + m_skyBoxIbv.SizeInBytes);
		}

		//GenAABB Debug 
		{	
			unsigned long cubeIndices[24] = { 0, 1,   1, 2,   2, 3,  3,0 ,
					   0, 4,   1, 5,   2, 6,  3,7,
					   4, 5,   5, 6,   6, 7,  7,0 };

			UINT aabbIbSize = sizeof(cubeIndices);
			m_pHeapManager->AllocateIndexData(reinterpret_cast<UINT8*>(cubeIndices), aabbIbSize);
			m_aabbIbv = m_pHeapManager->GetIbv(aabbIbSize, 0);
			UINT idx = m_pHeapManager->GetIbHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = m_pHeapManager->GetIbBlockOffset(idx);
			m_pHeapManager->SetIbBlockOffset(idx, curBlockOffset + m_aabbIbv.SizeInBytes);
		}

		if (!CreateRtvSrv(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D12::CreateRtvSrv()Failed");
		}

		if (!CreateDsv(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D12::CreateDsv()onScreenFailed");
		}

		//Gen QueryHeap
		{
			D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
			queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
			queryHeapDesc.Count = _4KB;

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				hr = m_pDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(m_pQueryHeap[i].GetAddressOf()));
				assert(SUCCEEDED(hr));
				m_pQueryHeap[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pQueryHeap[i]") - 1, "D3D12::m_pQueryHeap[i]");
			}
			
		}

		//GenPBR Resources	
	    {
			bool result =
				CreateEquirentangularMap(".\\Assets\\Models\\FBX\\Bistro_v5_2\\Bistro_v5_2\\san_giuseppe_bridge_4k.hdr");
			if (result)
			{
				ConvertEquirectagular2Cube();
				CreateDiffuseIrradianceMap();
				CreatePrefileterMap();
				CreateMipMap();
				CreateBrdfMap();

			}
			UINT resolution[] = { _SHADOWMAP_SIZE,  _SHADOWMAP_SIZE/2 };
			memcpy(m_pResolutionCbBegin, resolution, sizeof(UINT) * 2);
		}


		{
			m_ssaoBeginFrame = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);
			m_ssaoEndFrame = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_ssaoThreadHandle = reinterpret_cast<HANDLE>(_beginthreadex(
				nullptr,
				0,
				&WrapperSsaoThreadFun,		 //»ý¼º½Ã È£ÃâÇÔ¼ö
				nullptr,                 //Àü´Þ ÀÎÀÚ
				0,							//Áï½Ã ½ÇÇà
				nullptr));
		}
		
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			m_workerBeginFrame[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);
			m_workerFinishZpass[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerBeginHiZpass[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerFinishHiZpass[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerBeginHWOcclusionTestPass[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerFinishHWOcclusionTestPass[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerBeginShadowPass[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);
			
			m_workerFinishShadowPass[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerBeginDeferredGeoPass[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerEndFrame[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);
		
			m_workerThreadIdx[i] = i;
			m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
				nullptr,
				0,
				&WrapperWorkerThreadFun,		 //»ý¼º½Ã È£ÃâÇÔ¼ö
				reinterpret_cast<LPVOID>(&m_workerThreadIdx[i]),//Àü´Þ ÀÎÀÚ
				0,							//Áï½Ã ½ÇÇà
				nullptr ));
		}
		
		m_workerMutex = CreateMutex(NULL, FALSE, NULL);

		for (int i = 0; i < static_cast<UINT>(ePass::cnt); ++i)
		{
			m_pBundles[i]->Close();
		}
		PopulateBundle();
	}
	D3D12::~D3D12()
	{	
		WaitForGpu();
		ImGui_ImplDX12_Shutdown();

		CloseHandle(m_fenceEvent);
		CloseHandle(m_ssaoThreadHandle);
		CloseHandle(m_ssaoBeginFrame);
		CloseHandle(m_ssaoEndFrame);
		CloseHandle(m_workerMutex);
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			CloseHandle(m_threadHandles[i]);
			CloseHandle(m_workerBeginFrame[i]);
			CloseHandle(m_workerFinishZpass[i]);
			CloseHandle(m_workerBeginHWOcclusionTestPass[i]);
			CloseHandle(m_workerFinishHWOcclusionTestPass[i]);
			CloseHandle(m_workerBeginShadowPass[i]);
			CloseHandle(m_workerFinishShadowPass[i]);
			CloseHandle(m_workerBeginDeferredGeoPass[i]);
			CloseHandle(m_workerEndFrame[i]);
			CloseHandle(m_workerFenceEvents[i]);
		}

	}
}

