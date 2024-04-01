#include <random>
#include <process.h>
#include "D3D12.h"
#include "stb_image.h"
#include "../ImGui/imgui_impl_dx12.h"

namespace wilson
{	
#ifdef _DEBUG

	#include <dxgidebug.h>
	DEFINE_GUID(DXGI_DEBUG_D3D12, 0x4b99317b, 0xac39, 0x4aa6, 0xbb, 0xb, 0xba, 0xa0, 0x47, 0x84, 0x79, 0x8f);

	#pragma comment(lib, "d3d12.lib")
	#pragma comment(lib, "dxguid.lib")

	void D3D12::D3DMemoryLeakCheck()
	{
		HMODULE dxgidebugDLL = GetModuleHandleW(L"dxgidebug.dll");
		decltype(&DXGIGetDebugInterface) GetDebugInterface =
			reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugDLL, "DXGIGetDebugInterface"));

		IDXGIDebug* pDebug;
		GetDebugInterface(IID_PPV_ARGS(&pDebug));

		OutputDebugStringW(L"!!!D3D 메모리 누수 체크!!!\r\n");
		pDebug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
		OutputDebugStringW(L"!!!반환되지 않은 IUnKnown 객체!!!\r\n");

		pDebug->Release();
	}
#endif // _DEBUG
	D3D12* D3D12::g_pD3D12 = nullptr;
	void D3D12::WorkerThread(UINT threadIndex)
	{

		while (true)
		{
			//Zpass
			WaitForSingleObject(m_workerBeginFrame[threadIndex], INFINITE);
			DrawObject(eZpass, threadIndex, -1);
			SetEvent(m_workerFinishZpass[threadIndex]);

			//ShadowPass
			WaitForSingleObject(m_workerBeginShadowPass[threadIndex], INFINITE);

			std::vector<DirectionalLight12*>& dirLights = m_pLightBuffer->GetDirLights();
			std::vector<CubeLight12*>& CubeLights = m_pLightBuffer->GetCubeLights();
			std::vector<SpotLight12*>& spotLights = m_pLightBuffer->GetSpotLights();
			UINT litCounts[3] = { dirLights.size(), CubeLights.size(), spotLights.size() };
			UINT litCountSum = litCounts[0] + litCounts[1] + litCounts[2];
			if (litCountSum)
			{
				for (int i = 0; i < dirLights.size(); ++i)
				{
					DrawObject(eCascadeDirShadowPass, threadIndex, i);
				}

				
				for (int i = 0; i < spotLights.size(); ++i)
				{
					DrawObject(eSpotShadowPass, threadIndex, i);
				}
				
				for (int i = 0; i <CubeLights.size(); ++i)
				{
					DrawObject(eCubeShadowPass, threadIndex, i);
				}

			}
			SetEvent(m_workerFinishShadowPass[threadIndex]);

			//HWOcclusionTestPass
			WaitForSingleObject(m_workerBeginHWOcclusionTestPass[threadIndex], INFINITE);
			HWQueryForOcclusion(threadIndex);
			SetEvent(m_workerFinishHWOcclusionTestPass[threadIndex]);

			//PbrGeoPass
			WaitForSingleObject(m_workerBeginDeferredGeoPass[threadIndex], INFINITE);
			DrawObject(eGeoPass, threadIndex, -1);
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
	void D3D12::SSAOThread()
	{
		while (true)
		{	
			WaitForSingleObject(m_ssaoBeginFrame, INFINITE);

			ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
			//SSAO Pass
		
			m_pSSAOCommandList->SetPipelineState(m_pSSAOPso);
			m_pSSAOCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pSSAOCommandList->SetComputeRootSignature(m_pShader->GetSSAOShaderRootSingnature());
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsao_eCsNoise, m_NoiseSrv);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsao_eCsVpos, m_GBufSRV[eGbuf_vPos]);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsao_eCsVnomral, m_GBufSRV[eGbuf_vNormal]);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsao_eCsUAV, m_SSAOUAV);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsao_eCsWrap, m_WrapSSV);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsao_eCsClamp, m_ClampSSV);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsao_eCsSamplePoints, m_SSAOKernelCBV);
			m_pMatBuffer->UploadProjMat(m_pSSAOCommandList, true);
			//8보다 낮을 경우 감안
			m_pSSAOCommandList->Dispatch(ceil(m_clientWidth / (float)8), ceil(m_clientHeight / (float)8), 1);

			//Blur SSAOTex
			D3D12_RESOURCE_BARRIER blurBarriers[] =
			{
				CreateResourceBarrier(m_pSSAOTex,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
				CreateResourceBarrier(m_pSSAOBlurDebugTex,
					 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			};

			m_pSSAOCommandList->ResourceBarrier(2, blurBarriers);
			m_pSSAOCommandList->SetPipelineState(m_pSSAOBlurPso);
			m_pSSAOCommandList->SetComputeRootSignature(m_pShader->GetSSAOBlurShaderRootSingnature());
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsaoBlur_eCsDst, m_SSAOBlurUAV);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsaoBlur_eCsDebug, m_SSAOBlurDebugUAV);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsaoBlur_eCsSsao, m_SSAOSRV);
			m_pSSAOCommandList->SetComputeRootDescriptorTable(eSsaoBlur_eCsWrap, m_WrapSSV);
			m_pSSAOCommandList->Dispatch(ceil(m_clientWidth / (float)8), ceil(m_clientHeight / (float)8), 1);

			
			m_pSSAOCommandList->Close();
			m_pComputeCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pSSAOCommandList);
			
			UINT fenceValue = m_SsaoFenceValue++;
			m_pComputeCommandQueue->Signal(m_pSsaoFence, fenceValue);
			if (m_pSsaoFence->GetCompletedValue() < fenceValue)
			{
				m_pSsaoFence->SetEventOnCompletion(fenceValue, m_SsaoFenceEvent);
				WaitForSingleObject(m_SsaoFenceEvent, INFINITE);
			}
			m_pSSAOCommandAllocator->Reset();
			m_pSSAOCommandList->Reset(m_pSSAOCommandAllocator, nullptr);
			SetEvent(m_ssaoEndFrame);
		}
		return;
	}

	UINT __stdcall D3D12::WrapperSSAOThreadFun(LPVOID pParameter)
	{
		g_pD3D12->SSAOThread();
		return 0;
	}


	D3D12_RESOURCE_BARRIER D3D12::CreateResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
	{	
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = pResource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = beforeState;
		barrier.Transition.StateAfter = afterState;
		return barrier;
	}
	D3D12_RESOURCE_BARRIER D3D12::CreateResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, UINT mipLevel)
	{
		D3D12_RESOURCE_BARRIER barrier = CreateResourceBarrier(pResource, beforeState, afterState);
		barrier.Transition.Subresource = mipLevel;
		return barrier;
	}
	void D3D12::ExecuteCommandLists(ID3D12GraphicsCommandList** ppCmdLists, UINT cnt)
	{	 
		UINT fenceValue = m_fenceValue++;
		m_pMainCommandQueue->ExecuteCommandLists(cnt, (ID3D12CommandList**)(ppCmdLists));
		m_pMainCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void D3D12::ResizeBackBuffer(int newWidth, int newHeight)
	{
		UINT fenceValue = m_fenceValue;
		m_pMainCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		m_clientWidth = newWidth;
		m_clientHeight = newHeight;

		DestroyTexture();
		DestroySceneDepthTex();
		DestroyHDR();
		DestroyBackBuffer();

		m_pSwapChain->ResizeBuffers(_BUFFER_COUNT, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_EFFECT_FLIP_DISCARD);

		HRESULT hr;
		for (UINT i = 0; i < _BUFFER_COUNT; ++i)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pHeapManager->GetCurRtvHandle();
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pScreenTex[i]));
			assert(SUCCEEDED(hr));
			m_pScreenTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pScreenTex") - 1, "D3D12::m_pScreenTex");

			m_pDevice->CreateRenderTargetView(m_pScreenTex[i], nullptr, rtvHandle);
			m_ScreenRTV[i] = rtvHandle;
			m_pHeapManager->IncreaseRtvHandleOffset();

		}


		if (!CreateRTVandSRV(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D11::CreateRTVandSRV()Failed");
		}

		if (!CreateDSV(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D11::CreateDSV()onScreenFailed");
		}

		m_viewport.Width = static_cast<float>(m_clientWidth);
		m_viewport.Height = static_cast<float>(m_clientHeight);
		m_scissorRect.right = m_clientWidth;
		m_scissorRect.bottom = m_clientHeight;

		bool result =
			CreateEquirentangularMap(".\\Assets\\Models\\FBX\\Bistro_v5_2\\Bistro_v5_2\\san_giuseppe_bridge_4k.hdr");
		if (result)
		{
			ConvertEquirectagular2Cube();
			CreateDiffuseIrradianceMap();
			CreatePrefileterMap();
			CreateMipMap();
			CreateBRDFMap();
		}
	}
	void D3D12::WaitForGpu()
	{
		UINT fenceVal = m_fenceValue++;
		m_pMainCommandQueue->Signal(m_pFence, fenceVal);
		if (m_pFence->GetCompletedValue() < fenceVal)
		{
			m_pFence->SetEventOnCompletion(fenceVal, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		UINT ssaofenceVal = m_SsaoFenceValue++;
		m_pComputeCommandQueue->Signal(m_pSsaoFence, ssaofenceVal);
		if (m_pSsaoFence->GetCompletedValue() < ssaofenceVal)
		{
			m_pSsaoFence->SetEventOnCompletion(fenceVal, m_SsaoFenceEvent);
			WaitForSingleObject(m_SsaoFenceEvent, INFINITE);
		}
	}
	bool D3D12::CreateRTVandSRV(UINT width, UINT height)
	{
		//SSAO, 위치, 법선, PBR 텍스쳐들은 DXGI_FORMAT_R16G16B16A16_FLOAT;
		//Hi-Z는 깊이라서 d32
		//그외는 DXGI_FORMAT_R8G8B8A8_UNORM;

		DestroyTexture();

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
			&m_pSceneTex, m_pDevice);
		m_pSceneTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pSceneTex") - 1, "D3D12:::m_pSceneTex");
		m_SceneRTV = m_pHeapManager->GetRTV(rtvDesc, m_pSceneTex, m_pDevice);
		m_SceneSRV = m_pHeapManager->GetSRV(srvDesc, m_pSceneTex, m_pDevice);
	
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ,
			&m_pViewportTex, m_pDevice);
		m_pViewportTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pViewportTex") - 1, "D3D12:::m_pViewportTex");

		m_ViewportUAV = m_pHeapManager->GetUAV(uavDesc, m_pViewportTex, m_pDevice);
		m_ViewportSRV = m_pHeapManager->GetSRV(srvDesc, m_pViewportTex, m_pDevice);


		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ,
			&m_pBrightTex, m_pDevice);
		m_pBrightTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pBrightTex") - 1, "D3D12:::m_pBrightTex");
		m_BrightRTV = m_pHeapManager->GetRTV(rtvDesc, m_pBrightTex, m_pDevice);
		m_BrightSRV = m_pHeapManager->GetSRV(srvDesc, m_pBrightTex, m_pDevice);

		for (int i = 0; i < 2; ++i)
		{
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ,
				&m_pPingPongTex[i], m_pDevice);
			m_pPingPongTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPingPongTex[i]") - 1, "D3D12:::m_pPingPongTex[i]");
			m_PingPongRTV[i] = m_pHeapManager->GetRTV(rtvDesc, m_pPingPongTex[i], m_pDevice);
			m_PingPongSRV[i] = m_pHeapManager->GetSRV(srvDesc, m_pPingPongTex[i], m_pDevice);
		}

		
		for (int i = 0; i < eGbuf_cnt; ++i)
		{	
			if (i <eGbuf_albedo || i >eGbuf_emissive)
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
				&m_pGBufTex[i], m_pDevice);
			m_pGBufTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pGBufTex[i]") - 1, "D3D12:::m_pGBufTex[i]");
			
			m_GBufRTV[i] = m_pHeapManager->GetRTV(rtvDesc, m_pGBufTex[i], m_pDevice);
			m_GBufSRV[i] = m_pHeapManager->GetSRV(srvDesc, m_pGBufTex[i], m_pDevice);
		}


		//HiZ
		//fmaxf : 원래 밉맵은 가장 큰 변(side) 기준
		//log2f : 텍스쳐 크기가 절반씩 줄어들어서
		//+1    : 원본 텍스쳐까지 반영. ex)64, 32, 16, 8, 4, 2, 1

		{
			UINT mipLevels = 1 + (int)floorf(log2f(fmaxf(width, height)));
			m_hiZTempRtvs = std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>(mipLevels);
			m_hiZTempSrvs = std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>(mipLevels);

			texDesc.MipLevels = mipLevels;
			texDesc.Format = DXGI_FORMAT_R32_FLOAT;
			rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ,
				&m_pHiZTempTex, m_pDevice);
			m_pHiZTempTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pHiZTempTex") - 1, "D3D12:::m_pHiZTempTex");

			//밉맵생성용 
			for (int i = 0; i < mipLevels; ++i)
			{
				rtvDesc.Texture2D.MipSlice = i;
				srvDesc.Texture2D.MostDetailedMip = i;
				m_hiZTempRtvs[i] = m_pHeapManager->GetRTV(rtvDesc, m_pHiZTempTex, m_pDevice);
				m_hiZTempSrvs[i] = m_pHeapManager->GetSRV(srvDesc, m_pHiZTempTex, m_pDevice);
			}

			//Culling용
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			srvDesc.Texture2D.MipLevels = mipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			m_hiZTempSrvs.push_back(m_pHeapManager->GetSRV(srvDesc, m_pHiZTempTex, m_pDevice));


			texDesc.Width = sqrt(_HI_Z_CULL_COUNT);
			texDesc.Height = texDesc.Width;
			texDesc.MipLevels = 1;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				&m_pHiZCullListTex, m_pDevice);
			m_pHiZCullListTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pHiZCullListTex") - 1, "D3D12:::m_pHiZCullListTex");

			uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
			m_HiZCullListUAV = m_pHeapManager->GetUAV(uavDesc, m_pHiZCullListTex, m_pDevice);
		}
		

		//SSAO & Post process and UAV..
		{
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.MipLevels = 1;
			texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = 1;
			uavDesc.Format = texDesc.Format;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				&m_pSSAOTex, m_pDevice);
			m_pSSAOTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pSSAOTex") - 1, "D3D12:::m_pSSAOTex");
			m_SSAOSRV = m_pHeapManager->GetSRV(srvDesc, m_pSSAOTex, m_pDevice);
			m_SSAOUAV = m_pHeapManager->GetUAV(uavDesc, m_pSSAOTex, m_pDevice);

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&m_pSSAOBlurTex, m_pDevice);
			m_pSSAOBlurTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pSSAOBlurTex") - 1, "D3D12:::m_pSSAOBlurTex");
			m_SSAOBlurSRV = m_pHeapManager->GetSRV(srvDesc, m_pSSAOBlurTex, m_pDevice);
			m_SSAOBlurUAV = m_pHeapManager->GetUAV(uavDesc, m_pSSAOBlurTex, m_pDevice);

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS ,
				&m_pSSAOBlurDebugTex, m_pDevice);
			m_pSSAOBlurDebugTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pSSAOBlurDebugTex") - 1, "D3D12:::m_pSSAOBlurDebugTex");
			m_SSAOBlurDebugSRV = m_pHeapManager->GetSRV(srvDesc, m_pSSAOBlurDebugTex, m_pDevice);
			m_SSAOBlurDebugUAV = m_pHeapManager->GetUAV(uavDesc, m_pSSAOBlurDebugTex, m_pDevice);
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
				&m_pBRDFTex, m_pDevice);
			m_pBRDFTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pBRDFTex") - 1, "D3D12:::m_pBRDFTex");
			m_BRDFRTV = m_pHeapManager->GetRTV(rtvDesc, m_pBRDFTex, m_pDevice);
			m_BRDFSRV = m_pHeapManager->GetSRV(srvDesc, m_pBRDFTex, m_pDevice);
			
			texDesc.DepthOrArraySize = 6;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = 6;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = 1;
			srvDesc.TextureCube.MostDetailedMip = 0;

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&m_pSkyBoxTex, m_pDevice);
			m_pSkyBoxTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pSkyBoxTex") - 1, "D3D12:::m_pSkyBoxTex");
			m_SkyBoxRTV = m_pHeapManager->GetRTV(rtvDesc, m_pSkyBoxTex, m_pDevice);
			m_SkyBoxSRV = m_pHeapManager->GetSRV(srvDesc, m_pSkyBoxTex, m_pDevice);

			texDesc.Width = _DIFFIRRAD_WIDTH;
			texDesc.Height = _DIFFIRRAD_HEIGHT;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&m_pDiffIrradianceTex, m_pDevice);
			m_pDiffIrradianceTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pDiffIrradianceTex") - 1, "D3D12:::m_pDiffIrradianceTex");
			m_DiffIrradianceRTV = m_pHeapManager->GetRTV(rtvDesc, m_pDiffIrradianceTex, m_pDevice);
			m_DiffIrradianceSRV = m_pHeapManager->GetSRV(srvDesc, m_pDiffIrradianceTex, m_pDevice);
			

			texDesc.Width = _PREFILTER_WIDTH;
			texDesc.Height = _PREFILTER_HEIGHT;
			texDesc.MipLevels = _PREFILTER_MIP_LEVELS;
			srvDesc.TextureCube.MipLevels = _PREFILTER_MIP_LEVELS;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&m_pPrefilterTex, m_pDevice);
			m_pPrefilterTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPrefilterTex") - 1, "D3D12:::m_pPrefilterTex");
			m_PrefilterRTV = m_pHeapManager->GetRTV(rtvDesc, m_pPrefilterTex, m_pDevice);
			m_PrefilterSRV = m_pHeapManager->GetSRV(srvDesc, m_pPrefilterTex, m_pDevice);


			texDesc.Width = _PREFILTER_WIDTH;
			texDesc.Height = _PREFILTER_HEIGHT;
			texDesc.MipLevels = _PREFILTER_MIP_LEVELS;
			texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				&m_pGenMipUAVTex, m_pDevice);
			m_pGenMipUAVTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pGenMipUAVTex") - 1, "D3D12:::m_pGenMipUAVTex");
		}
		return true;
	}

	bool D3D12::CreateDSV(UINT width, UINT height)
	{	
		DestroySceneDepthTex();

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
			&m_pScreenDepthTex, m_pDevice);
		m_pScreenDepthTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pScreenDepthTex") - 1, "D3D12::m_pScreenDepthTex");
		m_ScreenDSV = m_pHeapManager->GetDSV(dsvDesc, m_pScreenDepthTex, m_pDevice);

		m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE ,
			&m_pSceneDepthTex, m_pDevice);
		m_pSceneDepthTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pSceneDepthTex") - 1, "D3D12::m_pSceneDepthTex");
		m_SceneDSV = m_pHeapManager->GetDSV(dsvDesc, m_pSceneDepthTex, m_pDevice);
	
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		m_SceneDepthSRV = m_pHeapManager->GetSRV(srvDesc, m_pSceneDepthTex, m_pDevice);
	
		return true;
	}

	void D3D12::DestroyTexture()
	{
		if (m_pViewportTex != nullptr)
		{
			m_pViewportTex->Release();
			m_pViewportTex = nullptr;
		}


		if (m_pSceneTex != nullptr)
		{
			m_pSceneTex->Release();
			m_pSceneTex = nullptr;
		}

		if (m_pSSAOTex != nullptr)
		{
			m_pSSAOTex->Release();
			m_pSSAOTex = nullptr;
		}

		if (m_pHiZTempTex != nullptr)
		{
			m_pHiZTempTex->Release();
			m_pHiZTempTex = nullptr;
		}

		if (m_pHiZCullListTex != nullptr)
		{
			m_pHiZCullListTex->Release();
			m_pHiZCullListTex = nullptr;
		}

		if (m_pSSAOBlurTex != nullptr)
		{
			m_pSSAOBlurTex->Release();
			m_pSSAOBlurTex = nullptr;
		}

		if (m_pSSAOBlurDebugTex != nullptr)
		{	
			m_pSSAOBlurDebugTex->Release();
			m_pSSAOBlurDebugTex = nullptr;
		}

		if (m_pBrightTex != nullptr)
		{
			m_pBrightTex->Release();
			m_pBrightTex = nullptr;
		}

		if (m_pBRDFTex != nullptr)
		{
			m_pBRDFTex->Release();
			m_pBRDFTex = nullptr;
		}

		if (m_pSkyBoxTex != nullptr)
		{
			m_pSkyBoxTex->Release();
			m_pSkyBoxTex = nullptr;
		}

		if (m_pDiffIrradianceTex != nullptr)
		{
			m_pDiffIrradianceTex->Release();
			m_pDiffIrradianceTex = nullptr;
		}

		if (m_pPrefilterTex != nullptr)
		{
			m_pPrefilterTex->Release();
			m_pPrefilterTex = nullptr;
		}

		if (m_pGenMipUAVTex != nullptr)
		{
			m_pGenMipUAVTex->Release();
			m_pGenMipUAVTex = nullptr;
		}

		for (int i = 0; i < 2; ++i)
		{	
			if (m_pPingPongTex[i] != nullptr)
			{	
				m_pPingPongTex[i]->Release();
				m_pPingPongTex[i] = nullptr;
			}

			
		}

		for (int i = 0; i < eGbuf_cnt; ++i)
		{
			if (m_pGBufTex[i] != nullptr)
			{
				m_pGBufTex[i]->Release();
				m_pGBufTex[i] = nullptr;
			}

		}
	}

	void D3D12::DestroySceneDepthTex()
	{
		if (m_pSceneDepthTex != nullptr)
		{
			m_pSceneDepthTex->Release();
			m_pSceneDepthTex = nullptr;
		}

	}

	bool D3D12::CreateEquirentangularMap(const char* pPath)
	{
		HRESULT hr;
		int width, height, nrComponents;
		//8perChannel->32perChannel
		float* data = stbi_loadf(pPath,
			&width, &height, &nrComponents, STBI_rgb_alpha);
		if (data)
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

			DestroyHDR();

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				&m_pHDRTex, m_pDevice);
			m_pHDRTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pHDRTex") - 1, "D3D12::m_pHDRTex");

			UploadTexThroughCB(texDesc, sizeof(XMVECTOR)*width, (UINT8*)data, m_pHDRTex, &m_pHdrUploadCb, m_pPbrSetupCommandList);
			
			ID3D12CommandList* ppCommandList[1] = { m_pPbrSetupCommandList };
			ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };

			m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_HDRSRV = m_pHeapManager->GetSRV(srvDesc, m_pHDRTex, m_pDevice);
			
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
		{	//변환된 텍스쳐가 뒤집혀져있음에 유의
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
			CreateResourceBarrier(m_pSkyBoxTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
			CreateResourceBarrier(m_pHDRTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		};
		m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
		m_pPbrSetupCommandList->SetPipelineState(m_pEquirect2CubePso);
		m_pPbrSetupCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pPbrSetupCommandList->IASetVertexBuffers(0, 1, &m_SkyBoxVBV);
		m_pPbrSetupCommandList->IASetIndexBuffer(&m_SkyBoxIBV);
		m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pPbrSetupCommandList->SetGraphicsRootSignature(m_pShader->GetEquirect2CubeRootSingnature());
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(eEquirect2CubeRP::eEquirect2Cube_eGsCb, m_Equirect2CubeCBV);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(eEquirect2CubeRP::eEquirect2Cube_ePsTex, m_HDRSRV);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(eEquirect2CubeRP::eEquirect2Cube_ePsSampler, m_WrapSSV);
		m_pPbrSetupCommandList->RSSetViewports(1, m_pShadowMap->GetViewport12());
		m_pPbrSetupCommandList->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
		m_pPbrSetupCommandList->OMSetRenderTargets(1, &m_SkyBoxRTV, TRUE, nullptr);
		m_pPbrSetupCommandList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);
		
	}

	void D3D12::CreateDiffuseIrradianceMap()
	{		
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		D3D12_RESOURCE_BARRIER barriers[] = {
			CreateResourceBarrier(m_pSkyBoxTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CreateResourceBarrier(m_pDiffIrradianceTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
		};

		m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers)/sizeof(D3D12_RESOURCE_BARRIER), barriers);
		m_pPbrSetupCommandList->SetPipelineState(m_pDiffuseIrradiancePso);
		m_pPbrSetupCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pPbrSetupCommandList->IASetVertexBuffers(0, 1, &m_SkyBoxVBV);
		m_pPbrSetupCommandList->IASetIndexBuffer(&m_SkyBoxIBV);
		m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pPbrSetupCommandList->SetGraphicsRootSignature(m_pShader->GetDiffuseIrradianceRootSingnature());
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(eDiffuseIrraidianceRP::eDiffuseIrraidiance_eGsCb, m_Equirect2CubeCBV);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsTex, m_SkyBoxSRV);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(eDiffuseIrraidianceRP::eDiffuseIrraidiance_ePsSampler, m_ClampSSV);
		m_pPbrSetupCommandList->RSSetViewports(1, &m_diffIrradViewport);
		m_pPbrSetupCommandList->RSSetScissorRects(1, &m_diffIrradRect);
		m_pPbrSetupCommandList->OMSetRenderTargets(1, &m_DiffIrradianceRTV, TRUE, nullptr);
		m_pPbrSetupCommandList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);
		
	}

	void D3D12::CreatePrefileterMap()
	{	
		FLOAT r = 0;
		memcpy(m_pRoughnessCbBegin, &r, sizeof(FLOAT));

		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		D3D12_RESOURCE_BARRIER barriers[] = {
			CreateResourceBarrier(m_pDiffIrradianceTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CreateResourceBarrier(m_pPrefilterTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
		};

		m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
		m_pPbrSetupCommandList->SetPipelineState(m_pPrefilterPso);

		m_pPbrSetupCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pPbrSetupCommandList->IASetVertexBuffers(0, 1, &m_SkyBoxVBV);
		m_pPbrSetupCommandList->IASetIndexBuffer(&m_SkyBoxIBV);
		m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pPbrSetupCommandList->SetGraphicsRootSignature(m_pShader->GetPrefilterRootSingnature());
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(ePrefilterRP::ePrefilter_eGsCb, m_Equirect2CubeCBV);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(ePrefilterRP::ePrefilter_ePsCb, m_RoughnessCBV);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(ePrefilterRP::ePrefilter_ePsTex, m_SkyBoxSRV);
		m_pPbrSetupCommandList->SetGraphicsRootDescriptorTable(ePrefilterRP::ePrefilter_ePsSampler, m_ClampSSV);
		m_pPbrSetupCommandList->RSSetViewports(1, &m_prefilterViewport);
		m_pPbrSetupCommandList->RSSetScissorRects(1, &m_prefilterRect);
		m_pPbrSetupCommandList->OMSetRenderTargets(1, &m_PrefilterRTV, TRUE, nullptr);
		m_pPbrSetupCommandList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);

		D3D12_RESOURCE_BARRIER rtvToSrvBarrier =
			CreateResourceBarrier(m_pPrefilterTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
		dst.pResource = m_pPrefilterTex;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.pResource = m_pGenMipUAVTex;


		UINT8* pMipCB;
		D3D12_RANGE readRange = {};
		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_pHeapManager->GetCurCbvSrvCpuHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_pHeapManager->GetCurCbvSrvGpuHandle();
	

		for (int i = 1; i < _PREFILTER_MIP_LEVELS; ++i)
		{
			{
				D3D12_RESOURCE_BARRIER srvToNsrvBarrier =
					CreateResourceBarrier(m_pPrefilterTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				m_pPbrSetupCommandList->ResourceBarrier(1, &srvToNsrvBarrier);

				uavDesc.Texture2DArray.MipSlice = i;

				cbvSrvCpuHandle = m_pHeapManager->GetCurCbvSrvCpuHandle();
				cbvSrvGpuHandle = m_pHeapManager->GetCurCbvSrvGpuHandle();
				m_pDevice->CreateUnorderedAccessView(m_pGenMipUAVTex, nullptr, &uavDesc, cbvSrvCpuHandle);
				m_pHeapManager->IncreaseCbvSrvHandleOffset();

				FLOAT texelSize[2] = { _PREFILTER_WIDTH >> i, _PREFILTER_HEIGHT >> i };

				memcpy(m_pResolutionCbBegin, texelSize, sizeof(texelSize));

				m_pPbrSetupCommandList->SetPipelineState(m_pGenMipmapPso);
				m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
				m_pPbrSetupCommandList->SetComputeRootSignature(m_pShader->GetGenMipShaderRootSingnature());
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(eGenMipRP::eGenMipRP_eCsTex, m_PrefilterSRV);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(eGenMipRP::eGenMipRP_eCsUAV, cbvSrvGpuHandle);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(eGenMipRP::eGenMipRP_eCsSampler, m_ClampSSV);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(eGenMipRP::eGenMipRP_eCsCb, m_ResolutionCBV);
				m_pPbrSetupCommandList->Dispatch(texelSize[0], texelSize[1], 6);//tex Dimesion


				D3D12_RESOURCE_BARRIER barriers[] = {
					CreateResourceBarrier(m_pPrefilterTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,  D3D12_RESOURCE_STATE_COPY_DEST),
					CreateResourceBarrier(m_pGenMipUAVTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
				};
				m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
				m_pPbrSetupCommandList->Close();


				ID3D12CommandList* pCommandList[] = { m_pPbrSetupCommandList };
				UINT fenceValue = m_fenceValue++;
				m_pMainCommandQueue->ExecuteCommandLists(1, pCommandList);
				m_pMainCommandQueue->Signal(m_pFence, fenceValue);
				if (m_pFence->GetCompletedValue() < fenceValue)
				{
					m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
					WaitForSingleObject(m_fenceEvent, INFINITE);
				}
				m_pPbrSetupCommandAllocator->Reset();
				m_pPbrSetupCommandList->Reset(m_pPbrSetupCommandAllocator, nullptr);
			}
			
			{
				for (int j = 0; j < 6; ++j)
				{
					dst.SubresourceIndex = i + (_PREFILTER_MIP_LEVELS * j);
					src.SubresourceIndex = i + (_PREFILTER_MIP_LEVELS * j);
					m_pPbrSetupCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
				}

				D3D12_RESOURCE_BARRIER barriers[] = {
					CreateResourceBarrier(m_pPrefilterTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
					CreateResourceBarrier(m_pGenMipUAVTex, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
				};

				m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
				m_pPbrSetupCommandList->Close();

				ID3D12CommandList* pCommandList[] = { m_pPbrSetupCommandList };
				UINT fenceValue = m_fenceValue++;
				m_pMainCommandQueue->ExecuteCommandLists(1, pCommandList);
				m_pMainCommandQueue->Signal(m_pFence, fenceValue);
				if (m_pFence->GetCompletedValue() < fenceValue)
				{
					m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
					WaitForSingleObject(m_fenceEvent, INFINITE);
				}

				m_pPbrSetupCommandAllocator->Reset();
				m_pPbrSetupCommandList->Reset(m_pPbrSetupCommandAllocator, nullptr);
			}
			
			
		}


		//https://learn.microsoft.com/en-us/windows/win32/direct3d12/subresources

	}

	void D3D12::CreateBRDFMap()
	{
		//Gen BRDFMap
		ID3D12CommandList* ppCommandList[1] = { m_pPbrSetupCommandList };
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		D3D12_RESOURCE_BARRIER rtvToSrvBarrier = CreateResourceBarrier(m_pBRDFTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		D3D12_RESOURCE_BARRIER srvToRtvBarrier = CreateResourceBarrier(m_pBRDFTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

		m_pPbrSetupCommandList->ResourceBarrier(1, &srvToRtvBarrier);
		m_pPbrSetupCommandList->SetPipelineState(m_pBRDFPso);
		m_pPbrSetupCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pPbrSetupCommandList->IASetVertexBuffers(0, 1, &m_QuadVBV);
		m_pPbrSetupCommandList->IASetIndexBuffer(&m_QuadIBV);
		m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pPbrSetupCommandList->SetGraphicsRootSignature(m_pShader->GetBRDFRootSingnature());
		m_pPbrSetupCommandList->RSSetViewports(1, m_pShadowMap->GetViewport12());
		m_pPbrSetupCommandList->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
		m_pPbrSetupCommandList->OMSetRenderTargets(1, &m_BRDFRTV, TRUE, nullptr);
		m_pPbrSetupCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);

		m_pPbrSetupCommandList->ResourceBarrier(1, &rtvToSrvBarrier);

	}

	void D3D12::DestroyHDR()
	{
		if (m_pHDRTex != nullptr)
		{
			m_pHDRTex->Release();
			m_pHDRTex = nullptr;
		}
	}

	void D3D12::DestroyBackBuffer()
	{
		if (m_pScreenDepthTex != nullptr)
		{
			m_pScreenDepthTex->Release();
			m_pScreenDepthTex = nullptr;
		}

		for (int i = 0; i < _BUFFER_COUNT; ++i)
		{
			if (m_pScreenTex[i] != nullptr)
			{
				m_pScreenTex[i]->Release();
				m_pScreenTex[i] = nullptr;
			}
		}
		

	}

	void D3D12::UpdateTotalModels()
	{
		{
			UINT modelCnt = 0;
			for (int i = 0; i < m_pModelGroups.size(); ++i)
			{
				std::vector<Model12*> pModels = m_pModelGroups[i]->GetModels();
				modelCnt += pModels.size();
			}
			std::vector<Model12*> drawLists;
			drawLists.reserve(modelCnt);
			for (int i = 0; i < m_pModelGroups.size(); ++i)
			{
				std::vector<Model12*> pModels = m_pModelGroups[i]->GetModels();
				for (int j = 0; j < pModels.size(); ++j)
				{
					drawLists.push_back(pModels[j]);
				}
			}
			m_pTotalModels = drawLists;
		}
	}

	void D3D12::DrawObject(ePass curPass, UINT threadIndex, UINT lightIdx) 
	{
		XMMATRIX worldMat, invWorldMat;
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		std::vector<DirectionalLight12*>& dirLights = m_pLightBuffer->GetDirLights();
		std::vector<CubeLight12*>& cubetLights = m_pLightBuffer->GetCubeLights();
		std::vector<SpotLight12*>& spotLights = m_pLightBuffer->GetSpotLights();

		while (!m_pModelQueue.empty())
		{
			DWORD result =WaitForSingleObject(m_workerMutex, INFINITE);

			if (result==WAIT_OBJECT_0)
			{	
				Model12* pModels[_OBJECT_PER_THREAD];
				UINT qLen = m_pModelQueue.size();
				for (int i = 0; i < min(qLen, _OBJECT_PER_THREAD); ++i)
				{
					pModels[i] = m_pModelQueue.front();
					m_pModelQueue.pop();
				}
				//Set-up Status
				ReleaseMutex(m_workerMutex);
				switch (curPass)
				{
				case wilson::eZpass:
				{
					m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pZpassPso);
					m_pWorkerCommandList[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetZpassRootSignature());
					m_pWorkerCommandList[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pWorkerCommandList[threadIndex]->RSSetViewports(1, &m_viewport);
					m_pWorkerCommandList[threadIndex]->RSSetScissorRects(1, &m_scissorRect);
					m_pWorkerCommandList[threadIndex]->OMSetRenderTargets(1, &m_SceneRTV, FALSE, &m_SceneDSV);
				}
					break;
				case wilson::eCascadeDirShadowPass:
				{
					m_pWorkerCommandList[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pCascadeDirShadowPso);
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetCascadeDirShadowRootSingnature());
					m_pWorkerCommandList[threadIndex]->RSSetViewports(1, m_pShadowMap->GetViewport12());
					m_pWorkerCommandList[threadIndex]->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
					m_pWorkerCommandList[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					dirLights[lightIdx]->UpdateLightSpaceMatrices();
					dirLights[lightIdx]->UploadShadowMatrices(m_pWorkerCommandList[threadIndex]);
					m_pShadowMap->BindDirDSV(m_pWorkerCommandList[threadIndex], lightIdx);
					
				}
					break;
				case wilson::eSpotShadowPass:
				{
					m_pWorkerCommandList[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pSpotShadowPso);
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetSpotShadowRootSingnature());
					m_pWorkerCommandList[threadIndex]->RSSetViewports(1, m_pShadowMap->GetViewport12());
					m_pWorkerCommandList[threadIndex]->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
					m_pWorkerCommandList[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					spotLights[lightIdx]->UpdateLitMat();
					m_pLitMat = spotLights[lightIdx]->GetLightSpaceMat();
					m_pShadowMap->BindSpotDSV(m_pWorkerCommandList[threadIndex], lightIdx);
				}
					break;
				case wilson::eCubeShadowPass:
				{	
					m_pWorkerCommandList[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pCubeShadowPso);
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetCubeShadowRootSingnature());
					m_pWorkerCommandList[threadIndex]->RSSetViewports(1, m_pShadowMap->GetViewport12());
					m_pWorkerCommandList[threadIndex]->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
					m_pWorkerCommandList[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootDescriptorTable(eCubeShadow_ePsSampler, m_WrapSSV);
					m_pShadowMap->BindCubeDSV(m_pWorkerCommandList[threadIndex], lightIdx);
					cubetLights[lightIdx]->UploadShadowMatrices(m_pWorkerCommandList[threadIndex]);
					cubetLights[lightIdx]->UploadLightPos(m_pWorkerCommandList[threadIndex]);
				}
					break;
				case wilson::eGeoPass:
				{

					m_pWorkerCommandList[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetPBRDeferredGeoShaderRootSingnature());
					m_pWorkerCommandList[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pWorkerCommandList[threadIndex]->RSSetViewports(1, &m_viewport);
					m_pWorkerCommandList[threadIndex]->RSSetScissorRects(1, &m_scissorRect);

					m_pCam->UploadCamPos(m_pWorkerCommandList[threadIndex], true);
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootDescriptorTable(ePbrGeoRP::ePbrGeo_ePsSampler, m_WrapSSV);
					m_pWorkerCommandList[threadIndex]->OMSetRenderTargets(eGbuf_cnt, m_GBufRTV, FALSE, &m_SceneDSV);
					memcpy(m_pHeightScaleCbBegin, &m_heightScale, sizeof(float));
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootDescriptorTable(ePbrGeoRP::ePbrGeo_ePsHeightScale, m_HeightScaleCBV);
				}
					break;
				case wilson::eDefault:
				default:
					break;
				}
				
				for (int i = 0; i < min(qLen, _OBJECT_PER_THREAD); ++i)
				{
					worldMat = pModels[i]->GetTransformMatrix(false);
					invWorldMat = pModels[i]->GetInverseWorldMatrix();
					MatBuffer12* pMatBuffer = pModels[i]->GetMatBuffer();
					pMatBuffer->SetWorldMatrix(&worldMat);
					pMatBuffer->SetInvWorldMatrix(&invWorldMat);
					pMatBuffer->SetViewMatrix(m_pCam->GetViewMatrix());
					pMatBuffer->SetProjMatrix(m_pCam->GetProjectionMatrix());

					switch (curPass)
					{
					case wilson::eZpass:
						pMatBuffer->UpdateCombinedMat(false);
						pMatBuffer->UploadCombinedMat(m_pWorkerCommandList[threadIndex], false);
						break;
					case wilson::eSpotShadowPass:
						pMatBuffer->SetLightSpaceMatrix(m_pLitMat);
						pMatBuffer->UpdateCombinedMat(true);
						pMatBuffer->UploadCombinedMat(m_pWorkerCommandList[threadIndex], true);
						break;
					case wilson::eGeoPass:
						pMatBuffer->UpdateCombinedMat(false);
						pMatBuffer->UploadMatBuffer(m_pWorkerCommandList[threadIndex]);
						break;
					case wilson::eCascadeDirShadowPass:
					case wilson::eCubeShadowPass:
					case wilson::eDefault:
					default:
						pMatBuffer->UploadMatBuffer(m_pWorkerCommandList[threadIndex]);
						break;
					}

					if (curPass == eGeoPass)
					{
						for (int j = 0; j < pModels[i]->GetMatCount(); ++j)
						{

							{
								PerModel perModel = *(pModels[i]->GetPerModel(j));
								if (perModel.hasNormal)
								{
									if (m_bHeightOnOff)
									{
										if (perModel.hasEmissive)
										{
											m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalHeightEmissivePso);
										}
										else
										{
											m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalHeightPso);
										}
									}
									else
									{
										m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalPso);
									}
								}
								else if (perModel.hasEmissive)
								{
									m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoEmissivePso);
								}
								else
								{
									m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoPso);
								}
							}

							pModels[i]->UploadBuffers(m_pWorkerCommandList[threadIndex], j, curPass);
							m_pWorkerCommandList[threadIndex]->DrawIndexedInstanced(pModels[i]->GetIndexCount(j), 1,
								0, 0, 0);
						}
					}
					else
					{
						pModels[i]->UploadBuffers(m_pWorkerCommandList[threadIndex], 0, curPass);
						m_pWorkerCommandList[threadIndex]->DrawIndexedInstanced(pModels[i]->GetTotalIndexCount(), 1,
							0, 0, 0);
					}
					

				}
				m_pWorkerCommandList[threadIndex]->Close();
				m_pMainCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pWorkerCommandList[threadIndex]);
				UINT fenceValue = m_workerFenceValue[threadIndex]++;
				m_pMainCommandQueue->Signal(m_pWorkerFence[threadIndex], fenceValue);
				if (m_pWorkerFence[threadIndex]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFence[threadIndex]->SetEventOnCompletion(fenceValue, m_workerFenceEvent[threadIndex]);
					WaitForSingleObject(m_workerFenceEvent[threadIndex], INFINITE);
				}
				
				HRESULT hr = m_pWorkerCommandAllocator[threadIndex]->Reset();
				assert(SUCCEEDED(hr));
				m_pWorkerCommandList[threadIndex]->Reset(m_pWorkerCommandAllocator[threadIndex], nullptr);
			}
		}
		

	}

	void D3D12::HWQueryForOcclusion(UINT threadIndex)
	{	
		ID3D12DescriptorHeap* ppHeaps[] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		while (!m_pModelQueue.empty())
		{
			DWORD result = WaitForSingleObject(m_workerMutex, INFINITE);

			if (result == WAIT_OBJECT_0)
			{
				Model12* pModels[_OBJECT_PER_THREAD];
				UINT qLen = m_pModelQueue.size();
				for (int i = 0; i < min(qLen, _OBJECT_PER_THREAD); ++i)
				{
					pModels[i] = m_pModelQueue.front();
					m_pModelQueue.pop();
				}
				//Set-up Status
				ReleaseMutex(m_workerMutex);
				UINT queryCnt = 0;
				for (int i = 0; i < min(qLen, _OBJECT_PER_THREAD); ++i)
				{
					
					XMMATRIX worldMat = pModels[i]->GetTransformMatrix(false);
					XMMATRIX invWorldMat = pModels[i]->GetInverseWorldMatrix();
					MatBuffer12* pMatBuffer = pModels[i]->GetMatBuffer();
					//m_pHWOcclusionQueryPso는 DSS만 Zpass랑 다름
					m_pWorkerCommandList[threadIndex]->SetPipelineState(m_pHWOcclusionQueryPso);
					m_pWorkerCommandList[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pWorkerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetZpassRootSignature());
					m_pWorkerCommandList[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_pWorkerCommandList[threadIndex]->RSSetViewports(1, &m_viewport);
					m_pWorkerCommandList[threadIndex]->RSSetScissorRects(1, &m_scissorRect);
					m_pWorkerCommandList[threadIndex]->OMSetRenderTargets(1, &m_SceneRTV, FALSE, &m_SceneDSV);

					pMatBuffer->SetWorldMatrix(&worldMat);
					pMatBuffer->SetInvWorldMatrix(&invWorldMat);
					pMatBuffer->SetViewMatrix(m_pCam->GetViewMatrix());
					pMatBuffer->SetProjMatrix(m_pCam->GetProjectionMatrix());

					pMatBuffer->UpdateCombinedMat(false);
					pMatBuffer->UploadCombinedMat(m_pWorkerCommandList[threadIndex], false);

					pModels[i]->UploadBuffers(m_pWorkerCommandList[threadIndex], 0, eOcclusionTestPass);
					m_pWorkerCommandList[threadIndex]->BeginQuery(m_pQueryHeap[threadIndex], D3D12_QUERY_TYPE_OCCLUSION, queryCnt);
					m_pWorkerCommandList[threadIndex]->DrawIndexedInstanced(pModels[i]->GetTotalIndexCount(), 1,
						0, 0, 0);
					m_pWorkerCommandList[threadIndex]->EndQuery(m_pQueryHeap[threadIndex], D3D12_QUERY_TYPE_OCCLUSION, queryCnt++);
				}

				ID3D12Resource* pQueryBlock= m_pHeapManager->GetQueryBlock();
				m_pWorkerCommandList[threadIndex]->ResolveQueryData(m_pQueryHeap[threadIndex], D3D12_QUERY_TYPE_OCCLUSION, 0, queryCnt,
					pQueryBlock, m_QueryResultOffset[threadIndex]);

				D3D12_RESOURCE_BARRIER copyDstToSrc =
				{
					CreateResourceBarrier(pQueryBlock, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE)
				};
				m_pWorkerCommandList[threadIndex]->ResourceBarrier(1, &copyDstToSrc);
				m_pWorkerCommandList[threadIndex]->CopyBufferRegion(m_pQueryReadBuffer[threadIndex], 0, pQueryBlock, m_QueryResultOffset[threadIndex],
					sizeof(UINT64)* min(qLen, _OBJECT_PER_THREAD));

				D3D12_RESOURCE_BARRIER copySrcToDst =
				{
					CreateResourceBarrier(pQueryBlock, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST)
				};
				m_pWorkerCommandList[threadIndex]->ResourceBarrier(1, &copySrcToDst);

				m_pWorkerCommandList[threadIndex]->Close();
				m_pMainCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pWorkerCommandList[threadIndex]);
				UINT fenceValue = m_workerFenceValue[threadIndex]++;
				m_pMainCommandQueue->Signal(m_pWorkerFence[threadIndex], fenceValue);
				if (m_pWorkerFence[threadIndex]->GetCompletedValue() < fenceValue)
				{
					m_pWorkerFence[threadIndex]->SetEventOnCompletion(fenceValue, m_workerFenceEvent[threadIndex]);
					WaitForSingleObject(m_workerFenceEvent[threadIndex], INFINITE);
				}
				m_pWorkerCommandAllocator[threadIndex]->Reset();
				m_pWorkerCommandList[threadIndex]->Reset(m_pWorkerCommandAllocator[threadIndex], nullptr);


				for (int i = 0; i < min(qLen, _OBJECT_PER_THREAD); ++i)
				{
					UINT64 result = *((UINT64*)m_pQueryReadCbBegin[threadIndex] + i);

					if (result)
					{
						m_pHWOcclusionQueue[threadIndex].push(pModels[i]);
					}
				}
				
			}
		}
		
	}

	void D3D12::AddModelGroup(ModelGroup12* pModelGroup)
	{
		m_pModelGroups.push_back(pModelGroup);
		UpdateTotalModels();
	}

	void D3D12::RemoveModelGroup(int i)
	{
		delete m_pModelGroups[i];
		m_pModelGroups.erase(m_pModelGroups.begin() + i);
		UpdateTotalModels();
	}

	void D3D12::RemoveModel(int i, int j)
	{
		std::vector<Model12*>& pModels = m_pModelGroups[i]->GetModels();
		delete pModels[j];
		pModels.erase(pModels.begin() + j);
		UpdateTotalModels();
	}

	UINT D3D12::GetModelSize(int i)
	{
		std::vector<Model12*>& pModels = m_pModelGroups[i]->GetModels();
		return pModels.size();
	}

	UINT D3D12::GetLightSize(eLIGHT_TYPE eLType)
	{
		UINT size = 0;
		switch (eLType)
		{
		case eLIGHT_TYPE::DIR:
			size = m_pLightBuffer->GetDirLightSize();
			break;
		case eLIGHT_TYPE::CUBE:
			size = m_pLightBuffer->GetCubeLightSize();
			break;
		case eLIGHT_TYPE::SPT:
			size = m_pLightBuffer->GetSpotLightSize();
		}
		return size;
	}

	void D3D12::AddLight(Light12* pLight)
	{
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			m_pLightBuffer->PushDirLight((DirectionalLight12*)pLight);
			break;
		case eLIGHT_TYPE::CUBE:
			m_pLightBuffer->PushCubeLight((CubeLight12*)pLight);
			break;
		case eLIGHT_TYPE::SPT:
			m_pLightBuffer->PushSpotLight((SpotLight12*)pLight);
		}
	}

	void D3D12::RemoveLight(int i, Light12* pLight)
	{
		std::vector<DirectionalLight12*>& pDirLights = m_pLightBuffer->GetDirLights();;
		std::vector<CubeLight12*>& pCubeLights = m_pLightBuffer->GetCubeLights();
		std::vector<SpotLight12*>& pSpotLights = m_pLightBuffer->GetSpotLights();
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			delete pDirLights[i];
			pDirLights.erase(pDirLights.begin() + i);
			break;
		case eLIGHT_TYPE::CUBE:
			delete pCubeLights[i];
			pCubeLights.erase(pCubeLights.begin() + i);
			break;
		case eLIGHT_TYPE::SPT:
			delete pSpotLights[i];
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

		PIXBeginEvent(m_pMainCommandList, 0, L"Init Textures and Set Barriers");
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };
		m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pMainCommandList->SetPipelineState(m_pZpassPso);


		ID3D12Resource* pTextures[] = { m_pScreenTex[m_curFrame], m_pViewportTex, m_pSceneTex, m_pSSAOTex, m_pSSAOBlurTex, m_pSSAOBlurDebugTex };
		UINT textureCount = sizeof(pTextures) / sizeof(ID3D12Resource*) + eGbuf_cnt;
		std::vector<D3D12_RESOURCE_BARRIER> barriers(textureCount);
		barriers[0] = CreateResourceBarrier(m_pScreenTex[m_curFrame], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers[1] = CreateResourceBarrier(m_pViewportTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[2] = CreateResourceBarrier(m_pSceneTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers[3] = CreateResourceBarrier(m_pSSAOTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[4] = CreateResourceBarrier(m_pSSAOBlurTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[5] = CreateResourceBarrier(m_pSSAOBlurDebugTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		for (int i = textureCount - eGbuf_cnt, j = 0; i < textureCount; ++i, ++j)
		{
			barriers[i] = CreateResourceBarrier(m_pGBufTex[j], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		m_pMainCommandList->ResourceBarrier(textureCount, &barriers[0]);

		m_pMainCommandList->ClearRenderTargetView(m_ScreenRTV[m_curFrame], color, 0, nullptr);
		m_pMainCommandList->ClearRenderTargetView(m_SceneRTV, color, 0, nullptr);

		for (int i = 0; i < eGbuf_cnt; ++i)
		{
			m_pMainCommandList->ClearRenderTargetView(m_GBufRTV[i], color, 0, nullptr);
		}

		m_pMainCommandList->ClearDepthStencilView(m_ScreenDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		m_pMainCommandList->ClearDepthStencilView(m_SceneDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		PIXEndEvent(m_pMainCommandList);


		PIXBeginEvent(m_pMainCommandList, 0, L"Init Shadow Textures and Set Barriers");
		UINT litCounts[] = { m_pLightBuffer->GetDirLightSize(), m_pLightBuffer->GetCubeLightSize(), m_pLightBuffer->GetSpotLightSize() };
		UINT litCountSum = litCounts[0] + litCounts[1] + litCounts[2];
		//Clear ShadowMap	
		if (litCountSum)
		{
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList, litCounts,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList, litCounts,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, false);

			m_pShadowMap->ClearRTV(m_pMainCommandList, litCounts);
			m_pShadowMap->ClearDSV(m_pMainCommandList, litCounts);
		}
		PIXEndEvent(m_pMainCommandList);
		m_pMainCommandList->Close();

		m_pMainCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pMainCommandList);
		UINT fenceValue = m_fenceValue++;
		m_pMainCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		m_pMainCommandAllocator->Reset();
		m_pMainCommandList->Reset(m_pMainCommandAllocator, nullptr);
		
		m_pCam->Update();

		//전체 개체 수 갱신 및 프러스텀 컬링
		
		XMVECTOR* pPlanes = m_pFrustum->GetPlanes();
		for (int i = 0; i < m_pTotalModels.size(); ++i)
		{
			XMMATRIX w = m_pTotalModels[i]->GetTransformMatrix(false);
			w = XMMatrixTranspose(w);

			Sphere* pSphere = m_pTotalModels[i]->GetSphere();
			XMFLOAT3 center = pSphere->GetCenter();
			XMFLOAT4 center4 = XMFLOAT4(center.x, center.y, center.z, 1.0f);
			XMVECTOR centerV = XMLoadFloat4(&center4);
			centerV = XMVector4Transform(centerV, w);
			XMStoreFloat3(&center, centerV);

			float r = pSphere->GetRadius();

			bool result= true;
			for (int j = 0; j < 6; ++j)
			{
				float dot =XMVectorGetX(
					XMPlaneDotCoord(pPlanes[j],
						XMVectorSet(center.x, center.y, center.z, 1.0f)));
				if (dot < -r/2)
				{
					result = false;
					break;
				}
			}
			if (result)
			{
				m_pModelQueue.push(m_pTotalModels[i]);
			}
		}
		m_pFrustum->SetENTTsInTotal(m_pTotalModels.size());
		m_pFrustum->SetENTTsInFrustum(m_pModelQueue.size());
		std::queue<Model12*> pModelQueue = m_pModelQueue;

		PIXBeginEvent(m_pMainCommandList, 0, L"Pre-Z Pass");
		//Zpass
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginFrame[i]);
		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_workerFinishZpass, TRUE, INFINITE);
		PIXEndEvent(m_pMainCommandList);

		//ShadowPass 
		PIXBeginEvent(m_pMainCommandList, 0, L"ShadowPass");
		m_pModelQueue=pModelQueue;
		HANDLE m_handles[_WORKER_THREAD_COUNT] ;
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginShadowPass[i]);
			m_handles[i] = m_workerFinishShadowPass[i];

		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);


		if (litCountSum) 
		{
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList, litCounts,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList, litCounts,
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, false);
		}
		PIXEndEvent(m_pMainCommandList);
		m_pModelQueue = pModelQueue;
		if (0)
		{	
			
			//Hi-Z Occlusion Pass
			{
				PIXBeginEvent(m_pMainCommandList, 0, L"Gen Hi-Z");
				UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(m_clientWidth, m_clientHeight)));
				//SetUp
				{
					D3D12_RESOURCE_BARRIER copyZ[] =
					{
						CreateResourceBarrier(m_pSceneDepthTex, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE),
						CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, 0)
					};
					m_pMainCommandList->ResourceBarrier(sizeof(copyZ) / sizeof(D3D12_RESOURCE_BARRIER), copyZ);

					UINT curWidth = m_clientWidth;
					UINT curHeight = m_clientHeight;

					D3D12_TEXTURE_COPY_LOCATION dst = {};
					dst.pResource = m_pHiZTempTex;
					dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					dst.SubresourceIndex = 0;

					D3D12_TEXTURE_COPY_LOCATION src = {};
					src.pResource = m_pSceneDepthTex;
					src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					src.SubresourceIndex = 0;

					m_pMainCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

					D3D12_RESOURCE_BARRIER afterCopyZ[] =
					{
						CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET,0),
						CreateResourceBarrier(m_pSceneDepthTex, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE)
					};

					m_pMainCommandList->ResourceBarrier(sizeof(afterCopyZ) / sizeof(D3D12_RESOURCE_BARRIER), afterCopyZ);

					for (int i = 1; i < mipLevels; ++i)
					{
						m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
						m_pMainCommandList->SetPipelineState(m_pGenHiZPassPso);
						m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetGenHiZpassRootSignature());
						m_pMainCommandList->SetGraphicsRootDescriptorTable(eGenHiZRP_ePs_sampler, m_borderSSV);
						m_pMainCommandList->SetGraphicsRootDescriptorTable(eGenHiZRP_ePs_lastResoltion, m_ResolutionCBV);
						D3D12_RESOURCE_BARRIER mipPrep[] =
						{
							CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, i - 1),
							CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, i)
						};
						m_pMainCommandList->ResourceBarrier(sizeof(mipPrep) / sizeof(D3D12_RESOURCE_BARRIER), mipPrep);
						m_pMainCommandList->SetGraphicsRootDescriptorTable(eGenHiZRP_ePs_lastMip, m_hiZTempSrvs[i - 1]);

						UINT resolution[] = { curWidth, curHeight };
						memcpy(m_pResolutionCbBegin, resolution, sizeof(UINT) * 2);

						D3D12_VIEWPORT viewPort = {};
						viewPort.Width = max(curWidth / 2, 1);
						viewPort.Height = max(curHeight / 2, 1);
			
						D3D12_RECT scissorRect = {};
						scissorRect.right = max(curWidth / 2, 1);
						scissorRect.bottom = max(curHeight / 2, 1);

						m_pMainCommandList->IASetVertexBuffers(0, 1, &m_QuadVBV);
						m_pMainCommandList->IASetIndexBuffer(&m_QuadIBV);
						m_pMainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						m_pMainCommandList->RSSetViewports(1, &viewPort);
						m_pMainCommandList->RSSetScissorRects(1, &scissorRect);
						m_pMainCommandList->OMSetRenderTargets(1, &m_hiZTempRtvs[i], FALSE, nullptr);
						m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);


						m_pMainCommandList->Close();
						m_pMainCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pMainCommandList);
						UINT fenceValue = m_fenceValue++;
						m_pMainCommandQueue->Signal(m_pFence, fenceValue);
						if (m_pFence->GetCompletedValue() < fenceValue)
						{
							m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
							WaitForSingleObject(m_fenceEvent, INFINITE);
						}
						m_pMainCommandAllocator->Reset();
						m_pMainCommandList->Reset(m_pMainCommandAllocator, nullptr);
						curWidth /= 2;
						curHeight /= 2;
					}
					D3D12_RESOURCE_BARRIER mipPrep[] =
					{
						CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, mipLevels - 1),
						CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
					};
					m_pMainCommandList->ResourceBarrier(sizeof(mipPrep) / sizeof(D3D12_RESOURCE_BARRIER), mipPrep);
				}
				PIXEndEvent(m_pMainCommandList);
				//Test
				std::queue<Model12*> q;
				UINT nModelsTotal = 0;
				PIXBeginEvent(m_pMainCommandList, 0, L"Hi-Z Culling");
				while (!m_pModelQueue.empty())
				{	
					UINT nModels = min(m_pModelQueue.size(),4096);
					UINT8* pSphereCbBegin = m_pSphereCbBegin;
					UINT8* pHiZCullMatrixCbBegin = m_pHiZCullMatrixCbBegin;

					UINT resolution[] = { m_clientWidth, m_clientHeight };
					memcpy(m_pResolutionCbBegin, resolution, sizeof(UINT) * 2);
					XMMATRIX matrices[3] = {
						*(m_pCam->GetViewMatrix()),
						*(m_pCam->GetProjectionMatrix()),
						*(m_pCam->GetViewProjectionMatrix())
					};
					memcpy(pHiZCullMatrixCbBegin, matrices, sizeof(XMMATRIX)*3);
					pHiZCullMatrixCbBegin += sizeof(XMMATRIX) * 3;
					memcpy(pHiZCullMatrixCbBegin, m_pFrustum->GetPlanes(), sizeof(XMVECTOR)*6);
					pHiZCullMatrixCbBegin += sizeof(XMVECTOR) * 6;

					XMVECTOR camInfo[3] =
					{
						*(m_pCam->GetPosition()),
						m_pCam->GetUp(),
						m_pCam->GetDir()
					};
					memcpy(pHiZCullMatrixCbBegin, camInfo, sizeof(XMVECTOR) * 3);



					for (int i=0;i<nModels;++i)
					{
						Model12* pModel = m_pModelQueue.front();
						m_pModelQueue.pop();
						
						MatBuffer12* pMatBuffer = pModel->GetMatBuffer();
						XMMATRIX w = pModel->GetTransformMatrix(false);
						w = XMMatrixTranspose(w);

						Sphere* pSphere = pModel->GetSphere();
						XMFLOAT3 center= pSphere->GetCenter();

						FLOAT r = pSphere->GetRadius();
						XMFLOAT4 center4 = XMFLOAT4(center.x, center.y, center.z, 1.0f);
						XMVECTOR centerV = XMLoadFloat4(&center4);
						centerV = XMVector4Transform(centerV, w);
						XMStoreFloat4(&center4, centerV);
						center4.w = r*0.5f;
						centerV = XMLoadFloat4(&center4);
					
						memcpy(pSphereCbBegin, &centerV, sizeof(XMVECTOR));
						pSphereCbBegin += sizeof(XMVECTOR);

					}

					m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
					m_pMainCommandList->SetPipelineState(m_pHiZCullPassPso);
					m_pMainCommandList->SetComputeRootSignature(m_pShader->GetHiZCullPassRootSignature());
					m_pMainCommandList->SetComputeRootDescriptorTable(eHiZCullRP_eCs_hiZ, m_hiZTempSrvs[mipLevels]);
					m_pMainCommandList->SetComputeRootDescriptorTable(eHiZCullRP_eCs_dst, m_HiZCullListUAV);
					m_pMainCommandList->SetComputeRootDescriptorTable(eHiZCullRP_eCs_resolution, m_ResolutionCBV);
					m_pMainCommandList->SetComputeRootDescriptorTable(eHiZCullRP_eCs_sphere, m_SphereCBV);
					m_pMainCommandList->SetComputeRootDescriptorTable(eHiZCullRP_eCs_matrix, m_HiZCullMatrixCBV);
					m_pMainCommandList->SetComputeRootDescriptorTable(eHiZCullRP_eCs_border, m_borderSSV);
					m_pMainCommandList->Dispatch(8, 8, 1);

					D3D12_RESOURCE_BARRIER fininshHiZ[] =
					{
						CreateResourceBarrier(m_pHiZCullListTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE )
					};
					m_pMainCommandList->ResourceBarrier(sizeof(fininshHiZ) / sizeof(D3D12_RESOURCE_BARRIER), fininshHiZ);
					
					//ReadBackResult;
					{  
						UINT len =sqrt(_HI_Z_CULL_COUNT);
						D3D12_TEXTURE_COPY_LOCATION dst = {};
						dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R32_FLOAT;
						dst.PlacedFootprint.Footprint.Width = sqrt(_HI_Z_CULL_COUNT);
						dst.PlacedFootprint.Footprint.RowPitch =CUSTUM_ALIGN(sizeof(FLOAT) * len, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
						dst.PlacedFootprint.Footprint.Height = len;
						dst.PlacedFootprint.Footprint.Depth = 1;

						dst.pResource = m_pHiZCullReadCb;
						dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
						
						D3D12_TEXTURE_COPY_LOCATION src = {};
						src.pResource = m_pHiZCullListTex;
						src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
						src.SubresourceIndex = 0;
						
						m_pMainCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
					}

					D3D12_RESOURCE_BARRIER finishReadBack[] =
					{
						CreateResourceBarrier(m_pHiZCullListTex, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
					};
					m_pMainCommandList->ResourceBarrier(sizeof(finishReadBack) / sizeof(D3D12_RESOURCE_BARRIER), finishReadBack);

					m_pMainCommandList->Close();
					m_pMainCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pMainCommandList);
					UINT fenceValue = m_fenceValue++;
					m_pMainCommandQueue->Signal(m_pFence, fenceValue);
					if (m_pFence->GetCompletedValue() < fenceValue)
					{
						m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
						WaitForSingleObject(m_fenceEvent, INFINITE);
					}
					m_pMainCommandAllocator->Reset();
					m_pMainCommandList->Reset(m_pMainCommandAllocator, nullptr);
					
					float* pHiZCullReadCbBegin = (float*)m_pHiZCullReadCbBegin;
					//Refresh queue
					for (int i = 0; i < nModels; ++i)
					{
						float result = *(pHiZCullReadCbBegin + i);
						Model12* pModel = pModelQueue.front();
						if (result)
						{
							q.push(pModel);
						}
						pModelQueue.pop();
						
					}
					nModelsTotal += nModels;
				}
				pModelQueue   = q;
				m_pModelQueue = pModelQueue;

				D3D12_RESOURCE_BARRIER finishHiZtest[] =
				{
					CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
				};
				m_pMainCommandList->ResourceBarrier(sizeof(finishHiZtest) / sizeof(D3D12_RESOURCE_BARRIER), finishHiZtest);
				PIXEndEvent(m_pMainCommandList);
			}
		}
		m_numOfModelsHiZPassed = m_pModelQueue.size();
		
		//HW오클루전 테스트
		PIXBeginEvent(m_pMainCommandList, 0, L"HW Culling");
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginHWOcclusionTestPass[i]);
			m_handles[i] = m_workerFinishHWOcclusionTestPass[i];

		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);
		PIXEndEvent(m_pMainCommandList);
		std::queue<Model12*> q;
		m_numOfModelsNotOccluded = 0;
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			m_numOfModelsNotOccluded += m_pHWOcclusionQueue[i].size();
			while (!m_pHWOcclusionQueue[i].empty())
			{
				Model12* pModel = m_pHWOcclusionQueue[i].front();
				m_pHWOcclusionQueue[i].pop();
				q.push(pModel);
			}
			
		}
		m_pModelQueue = q;

		//Draw PbrGeo
		PIXBeginEvent(m_pMainCommandList, 0, L"PbrGeo Pass");
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginDeferredGeoPass[i]);
		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_workerEndFrame, TRUE, INFINITE);

		D3D12_RESOURCE_BARRIER PbrGeoPass_barriers[eGbuf_cnt];
		for (int i = 0; i < eGbuf_cnt; ++i)
		{
			PbrGeoPass_barriers[i] = i > eGbuf_emissive ?
				CreateResourceBarrier(m_pGBufTex[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) :
				CreateResourceBarrier(m_pGBufTex[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		m_pMainCommandList->ResourceBarrier(sizeof(PbrGeoPass_barriers)/sizeof(D3D12_RESOURCE_BARRIER), PbrGeoPass_barriers);
		PIXEndEvent(m_pMainCommandList);
		m_pMainCommandList->Close();
		m_pMainCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pMainCommandList);
		fenceValue = m_fenceValue++;
		m_pMainCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		m_pMainCommandAllocator->Reset();
		m_pMainCommandList->Reset(m_pMainCommandAllocator, nullptr);

		SetEvent(m_ssaoBeginFrame);
		WaitForSingleObject(m_ssaoEndFrame, INFINITE);
		D3D12_RESOURCE_BARRIER ssaoEndBarriers[] = {
			CreateResourceBarrier(m_pSSAOBlurTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CreateResourceBarrier(m_pGBufTex[eGbuf_vNormal], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CreateResourceBarrier(m_pGBufTex[eGbuf_vPos], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		};

		m_pMainCommandList->ResourceBarrier(sizeof(ssaoEndBarriers) / sizeof(D3D12_RESOURCE_BARRIER) , ssaoEndBarriers);
		//Upload HeightScale and bHeightOnOff
		if (!m_pModelGroups.empty())
		{	
		
			//LightingPass
			PIXBeginEvent(m_pMainCommandList, 0, L"PbrLighting Pass");
			m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pMainCommandList->SetPipelineState(m_pPbrDeferredLightingPso);
			m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetPBRDeferredLightingShaderRootSingnature());
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsPos, m_GBufSRV[eGbuf_pos]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsNormal, m_GBufSRV[eGbuf_normal]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsAlbedo, m_GBufSRV[eGbuf_albedo]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsSpecular, m_GBufSRV[eGbuf_specular]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsEmissive, m_GBufSRV[eGbuf_emissive]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsAO, m_SSAOBlurSRV);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsIrradiance, m_DiffIrradianceSRV);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsPrefilter, m_PrefilterSRV);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsBrdf, m_BRDFSRV);
			m_pShadowMap->BindDirSRV(m_pMainCommandList);
			m_pShadowMap->BindSpotSRV(m_pMainCommandList);
			m_pShadowMap->BindCubeSRV(m_pMainCommandList);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsWrap, m_WrapSSV);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsCubeShadowSampler, m_pShadowMap->GetCubeShadowSamplerView());
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsShadowSampler, m_pShadowMap->GetDirShadowSamplerView());
			m_pCam->UploadCamPos(m_pMainCommandList, false);
			m_pMatBuffer->UploadProjMat(m_pMainCommandList, false);
			m_pMatBuffer->UploadViewMat(m_pMainCommandList);
			m_pCam->UploadCascadeLevels(m_pMainCommandList);
			m_pLightBuffer->UpdateLightBuffer(m_pMainCommandList);
			m_pLightBuffer->UpdateDirLightMatrices(m_pMainCommandList);
			m_pLightBuffer->UpdateSpotLightMatrices(m_pMainCommandList);
			m_pMainCommandList->IASetVertexBuffers(0, 1, &m_QuadVBV);
			m_pMainCommandList->IASetIndexBuffer(&m_QuadIBV);
			m_pMainCommandList->RSSetViewports(1, &m_viewport);
			m_pMainCommandList->RSSetScissorRects(1, &m_scissorRect);
			m_pMainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pMainCommandList->OMSetRenderTargets(1, &m_SceneRTV, TRUE, nullptr);
			m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);
			PIXEndEvent(m_pMainCommandList);
		}
		

		//Draw SkyBox
		PIXBeginEvent(m_pMainCommandList, 0, L"DrawSkyBox");
		m_pMainCommandList->SetPipelineState(m_pSkyBoxPso);
		m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetSkyBoxRootSingnature());
		m_pMainCommandList->SetGraphicsRootDescriptorTable(eSkybox_ePsDiffuseMap, m_SkyBoxSRV);
		m_pMainCommandList->SetGraphicsRootDescriptorTable(eSkybox_ePsSampler, m_WrapSSV);
		m_pMainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pMainCommandList->IASetVertexBuffers(0, 1, &m_SkyBoxVBV);
		m_pMainCommandList->IASetIndexBuffer(&m_SkyBoxIBV);
		m_pMainCommandList->RSSetViewports(1, &m_viewport);
		m_pMainCommandList->RSSetScissorRects(1, &m_scissorRect);
		m_pMainCommandList->OMSetRenderTargets(1, &m_SceneRTV, TRUE, &m_SceneDSV);

		m_pMatBuffer->SetWorldMatrix(&m_idMat);
		m_pMatBuffer->SetViewMatrix(m_pCam->GetViewMatrix());
		m_pMatBuffer->SetProjMatrix(m_pCam->GetProjectionMatrix());
		m_pMatBuffer->UpdateCombinedMat(false);
		m_pMatBuffer->UploadCombinedMat(m_pMainCommandList, false);
		m_pMainCommandList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);

		D3D12_RESOURCE_BARRIER postProcessPassBarriers[] = {
			CreateResourceBarrier(m_pSceneTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
		};
		m_pMainCommandList->ResourceBarrier(1, postProcessPassBarriers);
		PIXEndEvent(m_pMainCommandList);
		//PostProcess
		PIXBeginEvent(m_pMainCommandList, 0, L"PostProcess");

		m_pMainCommandList->SetPipelineState(m_pPostProcessPso);
		m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pMainCommandList->SetComputeRootSignature(m_pShader->GetPostProcessShaderRootSingnature());
		
		//Exporsure CBV 바인딩
		{
			memcpy(m_pExposureCbBegin, &m_exposure, sizeof(float));
			m_pMainCommandList->SetComputeRootDescriptorTable(ePostProcessRP::ePostProcess_eCsExposure, m_ExposureCBV);
		}

		m_pMainCommandList->SetComputeRootDescriptorTable(ePostProcessRP::ePostProcess_eCsTex, m_SceneSRV);
		m_pMainCommandList->SetComputeRootDescriptorTable(ePostProcess_eCsUav, m_ViewportUAV);
		m_pMainCommandList->SetComputeRootDescriptorTable(ePostProcessRP::ePostProcess_eCsSampler, m_WrapSSV);
		m_pMainCommandList->Dispatch(ceil(m_clientWidth / (float)8), ceil(m_clientHeight / (float)8), 1);
		PIXEndEvent(m_pMainCommandList);
		//DrawUI
		
		D3D12_RESOURCE_BARRIER uiPassBarriers[] = {
				CreateResourceBarrier(m_pSceneTex,  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				CreateResourceBarrier(m_pViewportTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		};

		m_pMainCommandList->ResourceBarrier(2, uiPassBarriers);
		m_pMainCommandList->OMSetRenderTargets(1, &m_ScreenRTV[m_curFrame], FALSE, &m_ScreenDSV);
		return;
}

	void D3D12::GenUploadBuffer(ID3D12Resource** ppUploadCB, const UINT64 uploadPitch, const UINT64 uploadSize)
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

	void D3D12::UploadTexThroughCB(D3D12_RESOURCE_DESC texDesc, const UINT rp,
	const UINT8* pData, ID3D12Resource* pDst, ID3D12Resource** ppUploadCB, ID3D12GraphicsCommandList* pCommandList)
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
	//4x4단위라서 감안해줘야함. +3은 4의배수로 나눠떨어지게 하기 위함.
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
	GenUploadBuffer(ppUploadCB, uploadPitch, uploadSize);


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
	//Padding 떄문에 memcpy한줄로 처리 불가
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
		ID3D12CommandList* ppCommandList[] = { m_pPbrSetupCommandList, m_pMainCommandList };
		ImGuiIO& io = ImGui::GetIO();
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pHeapManager->GetSamplerHeap()),
											*(m_pHeapManager->GetCbvSrvUavHeap()) };

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pMainCommandList);
		D3D12_RESOURCE_BARRIER rtvToPresent = 
			CreateResourceBarrier(m_pScreenTex[m_curFrame], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_pMainCommandList->ResourceBarrier(1, &rtvToPresent);
		m_pMainCommandList->Close();
		m_pPbrSetupCommandList->Close();
		m_pMainCommandQueue->ExecuteCommandLists(sizeof(ppCommandList) / sizeof(ID3D12CommandList*), ppCommandList);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, (void*)m_pMainCommandList);
		}

		UINT fenceValue = m_fenceValue++;
		m_pMainCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		m_pMainCommandAllocator->Reset();
		m_pMainCommandList->Reset(m_pMainCommandAllocator, nullptr);

		m_pPbrSetupCommandAllocator->Reset();
		m_pPbrSetupCommandList->Reset(m_pPbrSetupCommandAllocator, nullptr);

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

	D3D12::D3D12(int screenWidth, int screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
		float fScreenFar, float fScreenNear) :m_selectedModelGroup(-1), m_exposure(1.0f)
	{  
		g_pD3D12 = this;

		m_pDebugController = nullptr;
		m_pDevice = nullptr;
		m_pMainCommandQueue = nullptr;
		m_pComputeCommandQueue = nullptr;
		m_pSwapChain = nullptr;

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			m_pQueryHeap[i] = nullptr;
			m_pQueryReadBuffer[i] = nullptr;
			m_pQueryReadCbBegin[i] = nullptr;
		}
		for (int i = 0; i < _BUFFER_COUNT; ++i)
		{
			m_pScreenTex[i] = nullptr;
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			m_pWorkerCommandAllocator[i] = nullptr;
			m_pWorkerCommandList[i] = nullptr;
			m_pWorkerFence[i] = nullptr;
			m_workerFenceValue[i] = 0;
			m_pQueryHeap[i] = nullptr;
		}

		m_pMainCommandAllocator = nullptr;
		m_pMainCommandList = nullptr;
		m_pSSAOCommandAllocator = nullptr;
		m_pSSAOCommandList = nullptr;
		m_pPbrSetupCommandAllocator = nullptr;
		m_pPbrSetupCommandList = nullptr;
		m_pCopyCommandAllocator = nullptr;
		m_pCopyCommandList = nullptr;
		m_pFence = nullptr;
		m_pSsaoFence = nullptr;

		m_pZpassPso = nullptr;
		m_pGenHiZPassPso = nullptr;
		m_pHiZCullPassPso = nullptr;
		m_pHWOcclusionQueryPso = nullptr;
		m_pCascadeDirShadowPso = nullptr;
		m_pSpotShadowPso = nullptr;
		m_pCubeShadowPso = nullptr;
		m_pSkyBoxPso = nullptr;
		m_pPbrDeferredGeoPso = nullptr;
		m_pPbrDeferredGeoEmissivePso = nullptr;
		m_pPbrDeferredGeoNormalPso = nullptr;
		m_pPbrDeferredGeoNormalHeightPso = nullptr;
		m_pPbrDeferredGeoNormalHeightEmissivePso = nullptr;

		m_pOutlinerSetupPso = nullptr;
		m_pOutlinerTestPso = nullptr;
		m_pSSAOPso = nullptr;
		m_pSSAOBlurPso = nullptr;
		m_pPbrDeferredLightingPso = nullptr;
		m_pPostProcessPso = nullptr;
		m_pBRDFPso = nullptr;
		m_pPrefilterPso = nullptr;
		m_pDiffuseIrradiancePso = nullptr;
		m_pEquirect2CubePso = nullptr;
		m_pGenMipmapPso = nullptr;

		m_pNoiseTex = nullptr;
		m_pNoiseUploadCb = nullptr;
		m_pHiZCullReadCb = nullptr;
		m_pHdrUploadCb = nullptr;

		m_pViewportTex = nullptr;
		m_pSceneTex = nullptr;
		m_pHiZTempTex = nullptr;
		m_pHiZCullListTex = nullptr;
		m_pSSAOTex = nullptr;
		m_pSSAOBlurTex = nullptr;
		m_pSSAOBlurDebugTex = nullptr;
		m_pBrightTex = nullptr;
		m_pBRDFTex = nullptr;
		m_pSkyBoxTex = nullptr;
		m_pDiffIrradianceTex = nullptr;
		m_pPrefilterTex = nullptr;
		m_pGenMipUAVTex = nullptr;
		m_pScreenDepthTex = nullptr;
		m_pSceneDepthTex = nullptr;

		for (int i = 0; i < 2; ++i)
		{
			m_pPingPongTex[i] = nullptr;
		}

		for (int i = 0; i < eGbuf_cnt; ++i)
		{
			m_pGBufTex[i] = nullptr;
		}


		m_pHeapManager = nullptr;
		m_pCam = nullptr;
		m_pFrustum = nullptr;
		m_pLightBuffer = nullptr;
		m_pMatBuffer = nullptr;
		m_pOutlinerMatBuffer = nullptr;
		m_pShader = nullptr;
		m_pShadowMap = nullptr;

		m_pHeightScaleCbBegin = nullptr;
		m_pExposureCbBegin = nullptr;

		m_bVsyncOn = false;

		
		HRESULT hr;
		bool result;
#ifdef _DEBUG
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebugController));
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


		hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
		assert(SUCCEEDED(hr));
		m_pDevice->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pDevice") - 1, "D3D12::m_pDevice");
		

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pMainCommandQueue));
		assert(SUCCEEDED(hr));
		m_pMainCommandQueue->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pMainCommandQueue") - 1, "D3D12::m_pMainCommandQueue");


		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pComputeCommandQueue));
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

		hr = pFactory->CreateSwapChain(m_pMainCommandQueue, &swapChainDesc, &m_pSwapChain);
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
		m_pHeapManager = new HeapManager(m_pDevice);

		ImGui_ImplDX12_Init(m_pDevice, _BUFFER_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM, *(m_pHeapManager->GetCbvSrvUavHeap()),
			m_pHeapManager->GetCurCbvSrvCpuHandle(),
			m_pHeapManager->GetCurCbvSrvGpuHandle());
		m_pHeapManager->IncreaseCbvSrvHandleOffset();

		//Gen Rtvs;
		{

			for (UINT i = 0; i < _BUFFER_COUNT; ++i)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pHeapManager->GetCurRtvHandle();
				hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pScreenTex[i]));
				assert(SUCCEEDED(hr));
				m_pDevice->CreateRenderTargetView(m_pScreenTex[i], nullptr, rtvHandle);
				m_ScreenRTV[i] = rtvHandle;
				m_pHeapManager->IncreaseRtvHandleOffset();

				m_pScreenTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pScreenTex") - 1, "D3D12::m_pScreenTex");

			}


		}
		

		hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_pMainCommandAllocator));
		assert(SUCCEEDED(hr));
		m_pMainCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pMainCommandAllocator") - 1, "D3D12::m_pMainCommandAllocator");

		hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
			IID_PPV_ARGS(&m_pSSAOCommandAllocator));
		assert(SUCCEEDED(hr));
		m_pSSAOCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pSSAOCommandAllocator") - 1, "D3D12::m_pSSAOCommandAllocator");

		hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_pPbrSetupCommandAllocator));
		assert(SUCCEEDED(hr));
		m_pPbrSetupCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pPbrSetupCommandAllocator") - 1, "D3D12::m_pPbrSetupCommandAllocator");

		hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_pCopyCommandAllocator));
		assert(SUCCEEDED(hr));
		m_pCopyCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pCopyCommandAllocator") - 1, "D3D12::m_pCopyCommandAllocator");


		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pMainCommandAllocator, nullptr, IID_PPV_ARGS(&m_pMainCommandList));
		assert(SUCCEEDED(hr));
		m_pMainCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pMainCommandList") - 1, "D3D12::m_pMainCommandList");


		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_pSSAOCommandAllocator, nullptr, IID_PPV_ARGS(&m_pSSAOCommandList));
		assert(SUCCEEDED(hr));
		m_pSSAOCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pSSAOCommandList") - 1, "D3D12::m_pSSAOCommandList");

		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pPbrSetupCommandAllocator, nullptr, IID_PPV_ARGS(&m_pPbrSetupCommandList));
		assert(SUCCEEDED(hr));
		m_pPbrSetupCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pPbrSetupCommandList") - 1, "D3D12::m_pPbrSetupCommandList");

		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCopyCommandAllocator, nullptr, IID_PPV_ARGS(&m_pCopyCommandList));
		assert(SUCCEEDED(hr));
		m_pCopyCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pCopyCommandList") - 1, "D3D12::m_pCopyCommandList");

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pWorkerCommandAllocator[i]));
			assert(SUCCEEDED(hr));
			m_pWorkerCommandAllocator[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pWorkerCommandAllocator[i]") - 1, "D3D12::m_pWorkerCommandAllocator[i]");

			hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pWorkerCommandAllocator[i], nullptr, IID_PPV_ARGS(&m_pWorkerCommandList[i]));
			assert(SUCCEEDED(hr));
			m_pWorkerCommandList[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pWorkerCommandList[i]") - 1, "D3D12::m_pWorkerCommandList[i]");
		}

		//Gen User-Defined Class
		{
			m_pShader = new Shader12(m_pDevice);
			m_pCam = new Camera12(m_pDevice, m_pMainCommandList, m_pHeapManager, screenWidth, screenHeight, fScreenFar, fScreenNear);
			XMMATRIX* m_projMat = m_pCam->GetProjectionMatrix();
			XMMATRIX* m_viewMat = m_pCam->GetViewMatrix();
			m_pFrustum = new Frustum12(m_pCam);
			m_pMatBuffer = new MatBuffer12(m_pDevice, m_pMainCommandList, m_pHeapManager, m_viewMat, m_projMat);
			m_pOutlinerMatBuffer = new MatBuffer12(m_pDevice, m_pMainCommandList, m_pHeapManager, m_viewMat, m_projMat);

			m_pLightBuffer = new LightBuffer12(m_pDevice, m_pMainCommandList, m_pHeapManager);

			m_pShadowMap = new ShadowMap12(m_pDevice, m_pMainCommandList,
				m_pHeapManager,
				_SHADOWMAP_SIZE, _SHADOWMAP_SIZE, m_pCam->GetCascadeLevels().size(),
				m_pLightBuffer->GetDirLightCapacity(), m_pLightBuffer->GetCubeLightCapacity(), m_pLightBuffer->GetSpotLightCapacity());
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
		}

		D3D12_RASTERIZER_DESC geoRDesc = {};
		D3D12_RASTERIZER_DESC quadRDesc = {};
		D3D12_RASTERIZER_DESC AABBRDesc = {}; 
		D3D12_RASTERIZER_DESC skyboxRD = {};
		//Gen RS
		{
			geoRDesc.AntialiasedLineEnable = false;
			geoRDesc.CullMode = D3D12_CULL_MODE_NONE;
			geoRDesc.DepthBias = 0.0f;
			geoRDesc.DepthBiasClamp = 0.0f;
			geoRDesc.DepthClipEnable = true;
			geoRDesc.FillMode = D3D12_FILL_MODE_SOLID;
			geoRDesc.FrontCounterClockwise = true;
			geoRDesc.MultisampleEnable = false;
			geoRDesc.SlopeScaledDepthBias = 1.0f;
			
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

			GBufferWriteRTBlendDesc.BlendEnable = FALSE;
			GBufferWriteRTBlendDesc.SrcBlend = D3D12_BLEND_ONE;
			GBufferWriteRTBlendDesc.DestBlend = D3D12_BLEND_ZERO;
			GBufferWriteRTBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
			GBufferWriteRTBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			GBufferWriteRTBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
			GBufferWriteRTBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			GBufferWriteRTBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			GBufferBS = { FALSE, FALSE, GBufferWriteRTBlendDesc };

			lightingPassBlendDesc = GBufferWriteRTBlendDesc;
			lightingPassBlendDesc.BlendEnable = FALSE;
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
			//pso에 설정, com 개체가 아님.

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
			psoDesc.SampleMask = UINT_MAX; //BlendDesc의 그것,  고정
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] =DXGI_FORMAT_R32_FLOAT;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			psoDesc.SampleDesc.Count = 1; //AA기능 추가 전까진 고정

			psoDesc.pRootSignature = m_pShader->GetZpassRootSignature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetZpassShader(&psoDesc);
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pZpassPso));
			assert(SUCCEEDED(hr));

			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DepthStencilState = DepthTestOnlyDss;
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pHWOcclusionQueryPso));
			assert(SUCCEEDED(hr));

			psoDesc.DepthStencilState = offDss;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
			psoDesc.pRootSignature = m_pShader->GetGenHiZpassRootSignature();
			m_pShader->SetGenHiZpassShader(&psoDesc);
			hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pGenHiZPassPso));
			assert(SUCCEEDED(hr));

			psoDesc.DepthStencilState = defaultDss;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.pRootSignature = m_pShader->GetCascadeDirShadowRootSingnature();
			m_pShader->SetPosOnlyInputLayout(&psoDesc);
			m_pShader->SetCascadeDirShadowShader(&psoDesc);
			hr= m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pCascadeDirShadowPso));
			assert(SUCCEEDED(hr));

			psoDesc.pRootSignature = m_pShader->GetSpotShadowRootSingnature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetSpotShadowShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pSpotShadowPso));
			assert(SUCCEEDED(hr));

			psoDesc.RasterizerState = skyboxRD;
			psoDesc.pRootSignature= m_pShader->GetCubeShadowRootSingnature();
			m_pShader->SetCubeShadowShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pCubeShadowPso));
			assert(SUCCEEDED(hr));

			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.pRootSignature = m_pShader->GetSkyBoxRootSingnature();
			psoDesc.DepthStencilState = DepthTestOnlyDss;
			m_pShader->SetSkyBoxShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pSkyBoxPso));
			assert(SUCCEEDED(hr));


			for (int i = 0; i < eGbuf_cnt; ++i)
			{	
				if (i > eGbuf_normal && i < eGbuf_vPos)
				{
					psoDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
				}
				else
				{
					psoDesc.RTVFormats[i] = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}
			}
			
			psoDesc.pRootSignature = m_pShader->GetPBRDeferredGeoShaderRootSingnature();
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.DepthStencilState =DepthTestOnlyDss;
			psoDesc.BlendState = defaultBS;//GBufferBS;
			psoDesc.NumRenderTargets = eGbuf_cnt;
			m_pShader->SetDeferredGeoLayout(&psoDesc);
			m_pShader->SetPBRDeferredGeoShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoPso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoPso") - 1, "D3D12:::m_pPbrDeferredGeoPso");
			m_pShader->SetPBRDeferredGeoEmissiveShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoEmissivePso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoEmissivePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoEmissivePso") - 1, "D3D12:::m_pPbrDeferredGeoEmissivePso");
			m_pShader->SetPBRDeferredGeoNormalShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalPso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoNormalPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalPso") - 1, "D3D12:::m_pPbrDeferredGeoNormalPso");

			m_pShader->SetPBRDeferredGeoNormalHeightShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalHeightPso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoNormalHeightPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalHeightPso") - 1, "D3D12:::m_pPbrDeferredGeoNormalHeightPso");
			m_pShader->SetPBRDeferredGeoNormalHeightEmissiveShader(&psoDesc);
			hr =m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalHeightEmissivePso));
			assert(SUCCEEDED(hr));
			m_pPbrDeferredGeoNormalHeightEmissivePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalHeightEmissivePso") - 1, "D3D12:::m_pPbrDeferredGeoNormalHeightEmissivePso");
			
			//GeoPass에서 picking 한 모델만  Stencil값을 처리해주기 떄문에 pso에서 DSS만 바꿔줌. 
			psoDesc.DepthStencilState = outlinerSetupDss;
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pOutlinerSetupPso));
			assert(SUCCEEDED(hr));

			for (int i = 1; i < eGbuf_cnt; ++i)
			{
				psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			}

			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DepthStencilState = offDss;
			psoDesc.BlendState = lightingBS;
			psoDesc.NumRenderTargets = 1;
			psoDesc.pRootSignature = m_pShader->GetPBRDeferredLightingShaderRootSingnature();
			m_pShader->SetPBRDeferredLightingShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredLightingPso));
			assert(SUCCEEDED(hr));


			//OutlinerTest
			psoDesc.NumRenderTargets = 1;
			psoDesc.BlendState = defaultBS;
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.DepthStencilState = outlinerTestDss;
			psoDesc.pRootSignature = m_pShader->GetOutlinerTestShaderRootSingnature();
			m_pShader->SetOutlinerTestShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pOutlinerTestPso));
			assert(SUCCEEDED(hr));

			//BRDF
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
			psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.pRootSignature = m_pShader->GetBRDFRootSingnature();
			m_pShader->SetBRDFShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pBRDFPso));
			assert(SUCCEEDED(hr));

			//Prefilter
			psoDesc.pRootSignature = m_pShader->GetPrefilterRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetPosOnlyInputLayout(&psoDesc);
			m_pShader->SetPrefilterShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPrefilterPso));
			assert(SUCCEEDED(hr));

			//DiffuseIrradiance
			psoDesc.pRootSignature = m_pShader->GetDiffuseIrradianceRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetDiffuseIrradianceShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pDiffuseIrradiancePso));
			assert(SUCCEEDED(hr));

			//Equirect2Cube
			psoDesc.pRootSignature = m_pShader->GetEquirect2CubeRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetEquirect2CubeShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pEquirect2CubePso));
			assert(SUCCEEDED(hr));

			D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
			computePsoDesc.pRootSignature = m_pShader->GetHiZCullPassRootSignature();
			m_pShader->SetHiZCullPassShader(&computePsoDesc);
			hr = m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pHiZCullPassPso));
			assert(SUCCEEDED(hr));

			computePsoDesc.pRootSignature = m_pShader->GetGenMipShaderRootSingnature();
			m_pShader->SetGenMipShader(&computePsoDesc);
			hr=m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pGenMipmapPso));
			assert(SUCCEEDED(hr));

			computePsoDesc.pRootSignature = m_pShader->GetSSAOShaderRootSingnature();
			m_pShader->SetSSAOShader(&computePsoDesc);
			hr = m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pSSAOPso));
			assert(SUCCEEDED(hr));

			computePsoDesc.pRootSignature = m_pShader->GetSSAOBlurShaderRootSingnature();
			m_pShader->SetSSAOBlurShader(&computePsoDesc);
			hr = m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pSSAOBlurPso));
			assert(SUCCEEDED(hr));

			computePsoDesc.pRootSignature = m_pShader->GetPostProcessShaderRootSingnature();
			m_pShader->SetPostProcessShader(&computePsoDesc);
			hr = m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pPostProcessPso));
			assert(SUCCEEDED(hr));
		}
		//Gen Fence
		{
			hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
			assert(SUCCEEDED(hr));
			m_pFence->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pFence") - 1, "D3D12::m_pFence");
			m_fenceValue = 1;

			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(m_fenceEvent!=nullptr);

			hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pSsaoFence));
			assert(SUCCEEDED(hr));
			m_pSsaoFence->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSsaoFence") - 1, "D3D12::m_pSsaoFence");
			m_SsaoFenceValue = 1;

			m_SsaoFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(m_SsaoFenceEvent != nullptr);

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pWorkerFence[i]));
				assert(SUCCEEDED(hr));
				m_pWorkerFence[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pSsaoFence") - 1, "D3D12::m_pSsaoFence");
				m_workerFenceValue[i] = 1;

				m_workerFenceEvent[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				assert(m_workerFenceEvent[i] != nullptr);

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

				m_WrapSSV = samplerGpuHandle;
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

				m_ClampSSV = samplerGpuHandle;
				m_pHeapManager->IncreaseSamplerHandleOffset();
			}

			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = m_pHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = m_pHeapManager->GetCurSamplerGpuHandle();

				D3D12_SAMPLER_DESC borderSamplerDesc = {};
				borderSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				borderSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				borderSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				borderSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

				borderSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
				m_pDevice->CreateSampler(&borderSamplerDesc, samplerCpuHandle);

				m_borderSSV = samplerGpuHandle;
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
		
			
			UINT cbSize = sizeof(DirectX::XMVECTOR) * _HI_Z_CULL_COUNT;
			m_pSphereCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_SphereCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);
			
			cbSize = sizeof(XMMATRIX) * 3 + sizeof(XMVECTOR) * 9;
			m_pHiZCullMatrixCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_HiZCullMatrixCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);
			
			cbSize = sizeof(DirectX::XMVECTOR);
			m_pRoughnessCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_RoughnessCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);
			
			cbSize = sizeof(SamplePoints);
			m_pSSAOKernalCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_SSAOKernelCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);
			
			cbSize = sizeof(XMFLOAT4);
			m_pExposureCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_ExposureCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);

			cbSize = sizeof(XMFLOAT4);
			m_pHeightScaleCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_HeightScaleCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);
			
			cbSize = sizeof(XMMATRIX) * 6;
			m_pEquirect2CubeCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_Equirect2CubeCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);

			cbSize = sizeof(UINT) * 2;
			m_pResolutionCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_ResolutionCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);

			cbSize = sizeof(UINT) * 2;
			m_pResolutionCbBegin = m_pHeapManager->GetCbMappedPtr(cbSize);
			m_ResolutionCBV = m_pHeapManager->GetCBV(cbSize, m_pDevice);


			D3D12_RESOURCE_BARRIER readTocopyDst =
				CreateResourceBarrier(m_pHeapManager->GetQueryBlock(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
			m_pMainCommandList->ResourceBarrier(1, &readTocopyDst);
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{					
				{
					UINT resultSize = 8;
					UINT64 offset = m_pHeapManager->GetQueryHeapOffset();
					m_QueryResultOffset[i] = offset;
					m_pHeapManager->SetQueryHeapOffset(offset + 8);
				}

				{
					cbufferDesc.Width = _64KB;
					heapProps.Type = D3D12_HEAP_TYPE_READBACK;
					hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
						&cbufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pQueryReadBuffer[i]));
					assert(SUCCEEDED(hr));
					m_pQueryReadBuffer[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("D3D12::m_pQueryReadBuffer[i]") - 1, "D3D12::m_pQueryReadBuffer[i]");

					m_pHeapManager->IncreaseCbvSrvHandleOffset();

					readRange.End = cbufferDesc.Width;
					hr = m_pQueryReadBuffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pQueryReadCbBegin[i]));
					assert(SUCCEEDED(hr));
				}
				
			}
			
			{
				cbufferDesc.Width = CUSTUM_ALIGN(_HI_Z_CULL_COUNT * sizeof(FLOAT), _64KB);

				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pHeapManager->GetCurCbvSrvGpuHandle();

				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pHiZCullReadCb));
				assert(SUCCEEDED(hr));
				m_pHiZCullReadCb->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pHiZCullReadCb") - 1, "D3D12::m_pHiZCullReadCb");

				m_pHeapManager->IncreaseCbvSrvHandleOffset();

				readRange.End = cbufferDesc.Width;
				hr = m_pHiZCullReadCb->Map(0, &readRange, reinterpret_cast<void**>(&m_pHiZCullReadCbBegin));
				assert(SUCCEEDED(hr));
			}
			
		}
		//GenSSAOResources
		{	

			//노이즈만 CRV_SRV_HEAP 따로 관리
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
				float scale = i /(float) _KERNEL_COUNT;
				scale = 0.1f + (1.0f - 0.1f) * scale * scale;
				sampleV = XMVectorScale(sampleV, scale);

				kernels.coord[i] = sampleV;
			}

			memcpy(m_pSSAOKernalCbBegin, &kernels, sizeof(kernels));
			

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
				&m_pNoiseTex, m_pDevice);
			m_pNoiseTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pNoiseTex") - 1, "D3D12:::m_pNoiseTex");

			
			UploadTexThroughCB(texDesc, sizeof(XMVECTOR)*_NOISE_TEX_SIZE, (UINT8*)noiseVecs, m_pNoiseTex, &m_pNoiseUploadCb, m_pMainCommandList);
				
			D3D12_RESOURCE_BARRIER copyDstToSrvBarrier =
				CreateResourceBarrier(m_pNoiseTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			m_pMainCommandList->ResourceBarrier(1, &copyDstToSrvBarrier);


			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			m_NoiseSrv = m_pHeapManager->GetSRV(srvDesc, m_pNoiseTex, m_pDevice);
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

			UINT quadsVBSize = sizeof(quads);
			m_pHeapManager->AllocateVertexData((UINT8*)quads, quadsVBSize);
			m_QuadVBV = m_pHeapManager->GetVBV(quadsVBSize, sizeof(QUAD));

			unsigned long indices[6] = { 0,1,2, 2,3,0 };
			UINT quadsIBSize = sizeof(indices);
			m_pHeapManager->AllocateIndexData((UINT8*)indices, quadsIBSize);
			m_QuadIBV = m_pHeapManager->GetIBV(quadsIBSize, 0);
			UINT idx = m_pHeapManager->GetIndexBufferHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = m_pHeapManager->GetIndexBufferBlockOffset(idx);
			m_pHeapManager->SetIndexBufferBlockOffset(idx, curBlockOffset + m_QuadIBV.SizeInBytes);
		}


		//Gen Cube Buffer
		{
			XMFLOAT3 vertices[] = {
			 {XMFLOAT3(-1.0f,  1.0f, -1.0f)},//front-upper-left  0
			 {XMFLOAT3(1.0f,   1.0f, -1.0f)},//front-upper-right 1
			 {XMFLOAT3(1.0f,  -1.0f, -1.0f)},//front-down-right  2
			 {XMFLOAT3(-1.0f, -1.0f, -1.0f)},//front-down-left   3

			 {XMFLOAT3(-1.0f,  1.0f, 1.0f)},//back-upper-left   4
			 {XMFLOAT3(1.0f,   1.0f, 1.0f)},//back-upper-right  5
			 {XMFLOAT3(1.0f,  -1.0f, 1.0f)},//back-down-right   6
			 {XMFLOAT3(-1.0f, -1.0f, 1.0f)} };//back-down-left   7

			UINT skyVBSize = sizeof(vertices);


			m_pHeapManager->AllocateVertexData((UINT8*)vertices, skyVBSize);
			m_SkyBoxVBV = m_pHeapManager->GetVBV(skyVBSize, sizeof(XMFLOAT3));

			//시계방향 감은 면이  frontface
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
			UINT skyIBSize = sizeof(indices);
			m_pHeapManager->AllocateIndexData((UINT8*)indices, skyIBSize);
			m_SkyBoxIBV = m_pHeapManager->GetIBV(skyIBSize, 0);
			UINT idx = m_pHeapManager->GetIndexBufferHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = m_pHeapManager->GetIndexBufferBlockOffset(idx);
			m_pHeapManager->SetIndexBufferBlockOffset(idx,curBlockOffset + m_SkyBoxIBV.SizeInBytes);
		}

		//GenAABB Debug 
		{
			m_AABBVBV = m_SkyBoxVBV;
			unsigned long cubeIndices[24] = { 0, 1,   1, 2,   2, 3,  3,0 ,
					   0, 4,   1, 5,   2, 6,  3,7,
					   4, 5,   5, 6,   6, 7,  7,0 };

			UINT AABBIBSize = sizeof(cubeIndices);
			m_pHeapManager->AllocateIndexData((UINT8*)cubeIndices, AABBIBSize);
			m_AABBIBV = m_pHeapManager->GetIBV(AABBIBSize, 0);
			UINT idx = m_pHeapManager->GetIndexBufferHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = m_pHeapManager->GetIndexBufferBlockOffset(idx);
			m_pHeapManager->SetIndexBufferBlockOffset(idx, curBlockOffset + m_AABBIBV.SizeInBytes);
		}

		if (!CreateRTVandSRV(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D12::CreateRTVandSRV()Failed");
		}

		if (!CreateDSV(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D12::CreateDSV()onScreenFailed");
		}

		//Gen QueryHeap
		{
			D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
			queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
			queryHeapDesc.Count = 1000;

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				hr = m_pDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_pQueryHeap[i]));
				assert(SUCCEEDED(hr));
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
				CreateBRDFMap();
			}

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
				&WrapperSSAOThreadFun,		 //생성시 호출함수
				nullptr,                 //전달 인자
				0,							//즉시 실행
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
				&WrapperWorkerThreadFun,		 //생성시 호출함수
				reinterpret_cast<LPVOID>(&m_workerThreadIdx[i]),//전달 인자
				0,							//즉시 실행
				nullptr ));
		}
		
		m_workerMutex = CreateMutex(NULL, FALSE, NULL);
	}
	D3D12::~D3D12()
	{	
		WaitForGpu();
		ImGui_ImplDX12_Shutdown();
		DestroyHDR();
		DestroyTexture();
		DestroySceneDepthTex();
		DestroyBackBuffer();

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
			CloseHandle(m_workerFenceEvent[i]);
		}

		if (m_pDebugController != nullptr)
		{
			m_pDebugController->Release();
			m_pDebugController = nullptr;
		}

		if (m_pDevice != nullptr)
		{
			m_pDevice->Release();
			m_pDevice = nullptr;
		}

		if (m_pMainCommandQueue != nullptr)
		{
			m_pMainCommandQueue->Release();
			m_pMainCommandQueue = nullptr;
		}

		if (m_pComputeCommandQueue != nullptr)
		{
			m_pComputeCommandQueue->Release();
			m_pComputeCommandQueue = nullptr;
		}

		if (m_pSwapChain != nullptr)
		{
			m_pSwapChain->Release();
			m_pSwapChain = nullptr;
		}

		if (m_pFence != nullptr)
		{
			m_pFence->Release();
			m_pFence = nullptr;
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			if (m_pQueryHeap[i] != nullptr)
			{
				m_pQueryHeap[i]->Release();
				m_pQueryHeap[i] = nullptr;
			}

			if (m_pQueryReadBuffer[i] != nullptr)
			{
				m_pQueryReadBuffer[i]->Release();
				m_pQueryReadBuffer[i] = nullptr;
			}
		}
		

		if (m_pSsaoFence != nullptr)
		{
			m_pSsaoFence->Release();
			m_pSsaoFence = nullptr;
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			if (m_pWorkerFence[i] != nullptr)
			{
				m_pWorkerFence[i]->Release();
				m_pWorkerFence[i] = nullptr;
			}

		}


		if (m_pMainCommandAllocator != nullptr)
		{
			m_pMainCommandAllocator->Release();
			m_pMainCommandAllocator = nullptr;
		}

		if (m_pSSAOCommandAllocator != nullptr)
		{
			m_pSSAOCommandAllocator->Release();
			m_pSSAOCommandAllocator = nullptr;
		}

		if (m_pPbrSetupCommandAllocator != nullptr)
		{
			m_pPbrSetupCommandAllocator->Release();
			m_pPbrSetupCommandAllocator = nullptr;
		}

		if (m_pCopyCommandAllocator != nullptr)
		{
			m_pCopyCommandAllocator->Release();
			m_pCopyCommandAllocator = nullptr;
		}

		if (m_pZpassPso != nullptr)
		{
			m_pZpassPso->Release();
			m_pZpassPso = nullptr;
		}
		
		if (m_pGenHiZPassPso != nullptr)
		{
			m_pGenHiZPassPso->Release();
			m_pGenHiZPassPso = nullptr;
		}

		if (m_pHiZCullPassPso != nullptr)
		{
			m_pHiZCullPassPso->Release();
			m_pHiZCullPassPso = nullptr;
		}

		if (m_pHWOcclusionQueryPso != nullptr)
		{
			m_pHWOcclusionQueryPso->Release();
			m_pHWOcclusionQueryPso = nullptr;
		}

		if (m_pCascadeDirShadowPso != nullptr)
		{
			m_pCascadeDirShadowPso->Release();
			m_pCascadeDirShadowPso = nullptr;
		}
		
		if (m_pSpotShadowPso != nullptr)
		{
			m_pSpotShadowPso->Release();
			m_pSpotShadowPso = nullptr;
		}
		
		if (m_pCubeShadowPso != nullptr)
		{
			m_pCubeShadowPso->Release();
			m_pCubeShadowPso = nullptr;
		}

		if (m_pSkyBoxPso != nullptr)
		{
			m_pSkyBoxPso->Release();
			m_pSkyBoxPso = nullptr;
		}
		
		if (m_pPbrDeferredGeoPso != nullptr)
		{
			m_pPbrDeferredGeoPso->Release();
			m_pPbrDeferredGeoPso = nullptr;
		}
		if (m_pPbrDeferredGeoEmissivePso != nullptr)
		{
			m_pPbrDeferredGeoEmissivePso->Release();
			m_pPbrDeferredGeoEmissivePso = nullptr;
		}
		if (m_pPbrDeferredGeoNormalPso != nullptr)
		{
			m_pPbrDeferredGeoNormalPso->Release();
			m_pPbrDeferredGeoNormalPso = nullptr;
		}
		if (m_pPbrDeferredGeoNormalHeightPso != nullptr)
		{
			m_pPbrDeferredGeoNormalHeightPso->Release();
			m_pPbrDeferredGeoNormalHeightPso = nullptr;
		}
		if (m_pPbrDeferredGeoNormalHeightEmissivePso != nullptr)
		{
			m_pPbrDeferredGeoNormalHeightEmissivePso->Release();
			m_pPbrDeferredGeoNormalHeightEmissivePso = nullptr;
		}
		if (m_pOutlinerSetupPso != nullptr)
		{
			m_pOutlinerSetupPso->Release();
			m_pOutlinerSetupPso = nullptr;
		}
	
		if (m_pOutlinerTestPso != nullptr)
		{
			m_pOutlinerTestPso->Release();
			m_pOutlinerTestPso = nullptr;
		}

		if (m_pSSAOPso != nullptr)
		{
			m_pSSAOPso->Release();
			m_pSSAOPso = nullptr;
		}
		
		if (m_pSSAOBlurPso != nullptr)
		{
			m_pSSAOBlurPso->Release();
			m_pSSAOBlurPso = nullptr;
		}
		
		if (m_pPbrDeferredLightingPso != nullptr)
		{
			m_pPbrDeferredLightingPso->Release();
			m_pPbrDeferredLightingPso = nullptr;
		}
		
		if (m_pPostProcessPso != nullptr)
		{
			m_pPostProcessPso->Release();
			m_pPostProcessPso = nullptr;
		}
		
		if (m_pBRDFPso != nullptr)
		{
			m_pBRDFPso->Release();
			m_pBRDFPso = nullptr;
		}
		
		if (m_pPrefilterPso != nullptr)
		{
			m_pPrefilterPso->Release();
			m_pPrefilterPso = nullptr;
		}
		
		if (m_pDiffuseIrradiancePso != nullptr)
		{
			m_pDiffuseIrradiancePso->Release();
			m_pDiffuseIrradiancePso = nullptr;
		}

		if (m_pEquirect2CubePso != nullptr)
		{
			m_pEquirect2CubePso->Release();
			m_pEquirect2CubePso = nullptr;
		}

		if (m_pGenMipmapPso != nullptr)
		{	
			m_pGenMipmapPso->Release();
			m_pGenMipmapPso = nullptr;
		}


		if (m_pNoiseTex != nullptr)
		{
			m_pNoiseTex->Release();
			m_pNoiseTex = nullptr;
		}

		if (m_pNoiseUploadCb != nullptr)
		{
			m_pNoiseUploadCb->Release();
			m_pNoiseUploadCb = nullptr;
		}

		if (m_pHiZCullReadCb != nullptr)
		{
			m_pHiZCullReadCb->Release();
			m_pHiZCullReadCb = nullptr;
		}

		if (m_pHdrUploadCb != nullptr)
		{	
			m_pHdrUploadCb->Release();
			m_pHdrUploadCb = nullptr;
		}

		if (m_pMainCommandList != nullptr)
		{
			m_pMainCommandList->Release();
			m_pMainCommandList = nullptr;
		}

		if (m_pSSAOCommandList != nullptr)
		{
			m_pSSAOCommandList->Release();
			m_pSSAOCommandList = nullptr;
		}

		if (m_pPbrSetupCommandList != nullptr)
		{
			m_pPbrSetupCommandList->Release();
			m_pPbrSetupCommandList = nullptr;
		}

		if (m_pCopyCommandList != nullptr)
		{
			m_pCopyCommandList->Release();
			m_pCopyCommandList = nullptr;
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			if (m_pWorkerCommandList[i] != nullptr)
			{
				m_pWorkerCommandList[i]->Release();
				m_pWorkerCommandList[i] = nullptr;
			}

			if (m_pWorkerCommandAllocator[i] != nullptr)
			{
				m_pWorkerCommandAllocator[i]->Release();
				m_pWorkerCommandAllocator[i] = nullptr;
			}
		}
		
		for (int i = 0; i < m_pModelGroups.size(); ++i)
		{
			if (m_pModelGroups[i] != nullptr)
			{
				delete m_pModelGroups[i];
				m_pModelGroups[i] = nullptr;
			}
		}

		if (m_pHeapManager != nullptr)
		{	
			delete m_pHeapManager;
			m_pHeapManager = nullptr;
		}

		if (m_pLightBuffer != nullptr)
		{
			delete m_pLightBuffer;
			m_pLightBuffer = nullptr;
		}

		if (m_pMatBuffer != nullptr)
		{
			delete m_pMatBuffer;
			m_pMatBuffer = nullptr;
		}

		if (m_pOutlinerMatBuffer != nullptr)
		{	
			delete m_pOutlinerMatBuffer;
			m_pOutlinerMatBuffer = nullptr;
		}

		if (m_pCam != nullptr)
		{
			delete m_pCam;
			m_pCam = nullptr;
		}

		if (m_pFrustum != nullptr)
		{
			delete m_pFrustum;
			m_pFrustum = nullptr;
		}

		if (m_pShader != nullptr)
		{
			delete m_pShader;
			m_pShader = nullptr;
		}

		if (m_pShadowMap != nullptr)
		{
			delete m_pShadowMap;
			m_pShadowMap = nullptr;
		}

#ifdef _DEBUG
		D3DMemoryLeakCheck();
#endif
	}
}

