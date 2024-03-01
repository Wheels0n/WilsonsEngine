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
		bool bSpotShadowPass = false;
		bool bGeoPass = false;
		D3D12_RANGE readRange = { 0, };

		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											*(m_pDescriptorHeapManager->GetCbvSrvHeap()) };

		std::vector<DirectionalLight12*>& dirLights = m_pLightBuffer->GetDirLights();
		std::vector<PointLight12*>& pointLights = m_pLightBuffer->GetPointLights();
		std::vector<SpotLight12*>& spotLights = m_pLightBuffer->GetSpotLights();

		while (true)
		{
			WaitForSingleObject(m_workerBeginFrame[threadIndex], INFINITE);
			m_pWorkerCommandAllocator[threadIndex]->Reset();
			m_pWokerCommandList[threadIndex]->Reset(m_pWorkerCommandAllocator[threadIndex], m_pCascadeDirShadowPso);
			m_pWokerCommandList[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			
			UINT litCounts[3] = { dirLights.size(), pointLights.size(), spotLights.size() };
			UINT litCountSum = litCounts[0] + litCounts[1] + litCounts[2];
			//Gen ShadowMap	
			if (litCountSum)
			{
				//텍스쳐 rtv,dsv떄문에 에러

				m_pWokerCommandList[threadIndex]->RSSetViewports(1, m_pShadowMap->GetViewport12());
				m_pWokerCommandList[threadIndex]->RSSetScissorRects(1, m_pShadowMap->GetScissorRect());
				m_pWokerCommandList[threadIndex]->SetPipelineState(m_pCascadeDirShadowPso);
				m_pWokerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetCascadeDirShadowRootSingnature());
				m_pWokerCommandList[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				for (int i = 0; i < dirLights.size(); ++i)
				{
					dirLights[i]->UpdateLightSpaceMatrices();
					dirLights[i]->UploadShadowMatrices(m_pWokerCommandList[threadIndex]);
					m_pShadowMap->BindDirDSV(m_pWokerCommandList[threadIndex], i);
					DrawENTT(bGeoPass, bSpotShadowPass, threadIndex);
				}

				m_pWokerCommandList[threadIndex]->SetPipelineState(m_pSpotShadowPso);
				m_pWokerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetSpotShadowRootSingnature());
				bSpotShadowPass = true;
				for (int i = 0; i < spotLights.size(); ++i)
				{
					spotLights[i]->UpdateLitMat();
					m_pLitMat = spotLights[i]->GetLightSpaceMat();
					m_pShadowMap->BindSpotDSV(m_pWokerCommandList[threadIndex], i);
					DrawENTT(bGeoPass, bSpotShadowPass, threadIndex);
				}
				bSpotShadowPass = false;


				m_pWokerCommandList[threadIndex]->SetPipelineState(m_pOmniDirShadowPso);
				m_pWokerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetOmniDirShadowRootSingnature());
				for (int i = 0; i <pointLights.size(); ++i)
				{
					m_pShadowMap->BindCubeDSV(m_pWokerCommandList[threadIndex], i);
					pointLights[i]->UploadShadowMatrices(m_pWokerCommandList[threadIndex]);
					pointLights[i]->UploadLightPos(m_pWokerCommandList[threadIndex]);
					DrawENTT(bGeoPass, bSpotShadowPass, threadIndex);
				}

			}
			m_pWokerCommandList[threadIndex]->Close();
			SetEvent(m_workerFinshShadowPass[threadIndex]);

			//PbrGeoPass
			WaitForSingleObject(m_workerBeginDeferredGeoPass[threadIndex], INFINITE);

			m_pWorkerCommandAllocator[threadIndex]->Reset();
			m_pWokerCommandList[threadIndex]->Reset(m_pWorkerCommandAllocator[threadIndex], nullptr);
	
			m_pWokerCommandList[threadIndex]->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pWokerCommandList[threadIndex]->SetGraphicsRootSignature(m_pShader->GetPBRDeferredGeoShaderRootSingnature());
			m_pWokerCommandList[threadIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pWokerCommandList[threadIndex]->RSSetViewports(1, &m_viewport);
			m_pWokerCommandList[threadIndex]->RSSetScissorRects(1, &m_scissorRect);

			m_pCam->SetCamPos(m_pWokerCommandList[threadIndex], !bGeoPass);
			m_pWokerCommandList[threadIndex]->SetGraphicsRootDescriptorTable(ePbrGeoRP::ePbrGeo_ePsSampler, m_WrapSSV);
			m_pWokerCommandList[threadIndex]->OMSetRenderTargets(_GBUF_COUNT, m_GBufRTV, FALSE, &m_pSceneDSV);
			memcpy(m_pHeightScaleCbBegin, &m_heightScale, sizeof(float));
			m_pWokerCommandList[threadIndex]->SetGraphicsRootDescriptorTable(ePbrGeoRP::ePbrGeo_ePsHeightScale, m_HeightScaleCBV);
			DrawENTT(!bGeoPass, bSpotShadowPass, threadIndex);

			m_pWokerCommandList[threadIndex]->Close();
			SetEvent(m_workerEndFrame[threadIndex]);
		}
		return;
	}
	UINT __stdcall D3D12::WrapperThreadFun(LPVOID pParameter)
	{	
		UINT threadIndex = *(reinterpret_cast<UINT*>(pParameter));
		g_pD3D12->WorkerThread(threadIndex);
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

	void D3D12::ExecuteCommandLists(ID3D12GraphicsCommandList** ppCmdLists, UINT cnt)
	{	 
		UINT fenceValue = m_fenceValue++;
		m_pCommandQueue->ExecuteCommandLists(cnt, (ID3D12CommandList**)(ppCmdLists));
		m_pCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void D3D12::ResizeBackBuffer(int newWidth, int newHeight)
	{
		UINT fenceValue = m_fenceValue;
		m_pCommandQueue->Signal(m_pFence, fenceValue);
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
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pScreenTex[i]));
			assert(SUCCEEDED(hr));
			m_pScreenTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pScreenTex") - 1, "D3D12::m_pScreenTex");

			m_pDevice->CreateRenderTargetView(m_pScreenTex[i], nullptr, rtvHandle);
			m_ScreenRTV[i] = rtvHandle;
			m_pDescriptorHeapManager->IncreaseRtvHandleOffset();

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
		
		if (m_pFence->GetCompletedValue() < m_fenceValue)
		{
			m_pFence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}
	bool D3D12::CreateRTVandSRV(UINT width, UINT height)
	{   
		//코드 중복, 함수로 만들것
		DestroyTexture();

		HRESULT hr;
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC	texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		texDesc.Alignment = 0;
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension =D3D12_RTV_DIMENSION_TEXTURE2D;


		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = texDesc.Format;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		D3D12_CLEAR_VALUE clearVal = {};
		clearVal.Format = texDesc.Format;
		clearVal.Color[3] = 1.0f;

		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pViewportTex));
		assert(SUCCEEDED(hr));
		m_pViewportTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pViewportTex") - 1, "D3D12:::m_pViewportTex");


		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pSceneTex));
		assert(SUCCEEDED(hr));
		m_pSceneTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pSceneTex") - 1, "D3D12:::m_pSceneTex");


		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pSSAOBlurDebugTex));
		assert(SUCCEEDED(hr));
		m_pSSAOBlurDebugTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pSSAOBlurDebugTex") - 1, "D3D12:::m_pSSAOBlurDebugTex");
		
		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pBrightTex));
		assert(SUCCEEDED(hr));
		m_pBrightTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pBrightTex") - 1, "D3D12:::m_pBrightTex");


		for (int i = 0; i < 2; ++i)
		{	
			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pPingPongTex[i]));
			assert(SUCCEEDED(hr));
			m_pPingPongTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPingPongTex[i]") - 1, "D3D12:::m_pPingPongTex[i]");
		}
		for (int i = 0; i < _GBUF_COUNT; ++i)
		{
			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pGBufTex[i]));
			assert(SUCCEEDED(hr));
			m_pGBufTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pGBufTex[i]") - 1, "D3D12:::m_pGBufTex[i]");
		}



		D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pViewportTex, &rtvDesc, rtvCpuHandle);
		m_ViewportRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();
		
		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pSceneTex, &rtvDesc, rtvCpuHandle);
		m_SceneRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();

		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pSSAOBlurDebugTex, &rtvDesc, rtvCpuHandle);
		m_SSAOBlurDebugRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();

		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pBrightTex, &rtvDesc, rtvCpuHandle);
		m_BrightRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();


		for (int i = 0; i < 2; ++i)
		{	
			rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
			m_pDevice->CreateRenderTargetView(m_pPingPongTex[i], &rtvDesc, rtvCpuHandle);
			m_PingPongRTV[i] = rtvCpuHandle;
			m_pDescriptorHeapManager->IncreaseRtvHandleOffset();
		}
		for (int i = 0; i < _GBUF_COUNT; ++i)
		{	
			rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
			m_pDevice->CreateRenderTargetView(m_pGBufTex[i], &rtvDesc, rtvCpuHandle);
			m_GBufRTV[i] = rtvCpuHandle;
			m_pDescriptorHeapManager->IncreaseRtvHandleOffset();

		}


		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();	
		m_pDevice->CreateShaderResourceView(m_pViewportTex, &srvDesc, cbvSrvCpuHandle);
		m_viewportSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		m_pDevice->CreateShaderResourceView(m_pSceneTex, &srvDesc, cbvSrvCpuHandle);
		m_SceneSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		m_pDevice->CreateShaderResourceView(m_pSSAOBlurDebugTex, &srvDesc, cbvSrvCpuHandle);
		m_SSAOBlurDebugSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		m_pDevice->CreateShaderResourceView(m_pBrightTex, &srvDesc, cbvSrvCpuHandle);
		m_BrightSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();


		for (int i = 0; i < 2; ++i)
		{
			cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
			m_pDevice->CreateShaderResourceView(m_pPingPongTex[i], &srvDesc, cbvSrvCpuHandle);
			m_PingPongSRV[i] = cbvSrvGpuHandle;
			m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}
		for (int i = 0; i < _GBUF_COUNT; ++i)
		{
			cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
			m_pDevice->CreateShaderResourceView(m_pGBufTex[i], &srvDesc, cbvSrvCpuHandle);
			m_GBufSRV[i] = cbvSrvGpuHandle;
			m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}

		


		

		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R32_FLOAT;
		clearVal.Format = texDesc.Format;
		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pSSAOTex));
		assert(SUCCEEDED(hr));
		m_pSSAOTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pSSAOTex") - 1, "D3D12:::m_pSSAOTex");


		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pSSAOBlurTex));
		assert(SUCCEEDED(hr));
		m_pSSAOBlurTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pSSAOBlurTex") - 1, "D3D12:::m_pSSAOBlurTex");


		rtvDesc.Format = texDesc.Format;

		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pSSAOTex, &rtvDesc, rtvCpuHandle);
		m_SSAORTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();

		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pSSAOBlurTex, &rtvDesc, rtvCpuHandle);
		m_SSAOBlurRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();


		srvDesc.Format = texDesc.Format;

		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		m_pDevice->CreateShaderResourceView(m_pSSAOTex, &srvDesc, cbvSrvCpuHandle);
		m_SSAOSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		m_pDevice->CreateShaderResourceView(m_pSSAOBlurTex, &srvDesc, cbvSrvCpuHandle);
		m_SSAOBlurSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();



		texDesc.Width = _SHADOWMAP_SIZE;
		texDesc.Height = _SHADOWMAP_SIZE;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		clearVal.Format = texDesc.Format;
		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pBRDFTex));
		assert(SUCCEEDED(hr));
		m_pBRDFTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pBRDFTex") - 1, "D3D12:::m_pBRDFTex");

		texDesc.DepthOrArraySize = 6;
		
		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pSkyBoxTex));
		assert(SUCCEEDED(hr));
		m_pSkyBoxTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pSkyBoxTex") - 1, "D3D12:::m_pSkyBoxTex");


		texDesc.Width = _DIFFIRRAD_WIDTH;
		texDesc.Height = _DIFFIRRAD_HEIGHT;
		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pDiffIrradianceTex));
		assert(SUCCEEDED(hr));
		m_pDiffIrradianceTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pDiffIrradianceTex") - 1, "D3D12:::m_pDiffIrradianceTex");



		texDesc.Width = _PREFILTER_WIDTH;
		texDesc.Height = _PREFILTER_HEIGHT;
		texDesc.MipLevels = _PREFILTER_MIP_LEVELS;
		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearVal, IID_PPV_ARGS(&m_pPrefilterTex));
		assert(SUCCEEDED(hr));
		m_pPrefilterTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pPrefilterTex") - 1, "D3D12:::m_pPrefilterTex");

		texDesc.Width = _PREFILTER_WIDTH;
		texDesc.Height = _PREFILTER_HEIGHT;
		texDesc.MipLevels = _PREFILTER_MIP_LEVELS;
		texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, &clearVal, IID_PPV_ARGS(&m_pUAVTex));
		assert(SUCCEEDED(hr));
		m_pUAVTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12:::m_pUAVTex") - 1, "D3D12:::m_pUAVTex");


		rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pBRDFTex, &rtvDesc, rtvCpuHandle);
		m_BRDFRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();


		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.ArraySize = 6;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;

		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pSkyBoxTex, &rtvDesc, rtvCpuHandle);
		m_SkyBoxRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();
	
		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pDiffIrradianceTex, &rtvDesc, rtvCpuHandle);
		m_DiffIrradianceRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();

		rtvCpuHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
		m_pDevice->CreateRenderTargetView(m_pPrefilterTex, &rtvDesc, rtvCpuHandle);
		m_PrefilterRTV = rtvCpuHandle;
		m_pDescriptorHeapManager->IncreaseRtvHandleOffset();



		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		m_pDevice->CreateShaderResourceView(m_pBRDFTex, &srvDesc, cbvSrvCpuHandle);
		m_BRDFSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;

		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		m_pDevice->CreateShaderResourceView(m_pSkyBoxTex, &srvDesc, cbvSrvCpuHandle);
		m_SkyBoxSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		m_pDevice->CreateShaderResourceView(m_pDiffIrradianceTex, &srvDesc, cbvSrvCpuHandle);
		m_DiffIrradianceSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
		srvDesc.TextureCube.MipLevels = _PREFILTER_MIP_LEVELS;
		m_pDevice->CreateShaderResourceView(m_pPrefilterTex, &srvDesc, cbvSrvCpuHandle);
		m_PrefilterSRV = cbvSrvGpuHandle;
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		return true;
	}

	bool D3D12::CreateDSV(UINT width, UINT height)
	{	
		DestroySceneDepthTex();

		HRESULT hr;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;


		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		texDesc.Alignment = 0;
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;

		D3D12_CLEAR_VALUE clearVal = {};
		clearVal.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clearVal.DepthStencil.Depth = 1.0f;

		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearVal, IID_PPV_ARGS(&m_pScreenDepthTex));
		assert(SUCCEEDED(hr));
		m_pScreenDepthTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pScreenDepthTex") - 1, "D3D12::m_pScreenDepthTex");
		
		hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearVal, IID_PPV_ARGS(&m_pSceneDepthTex));
		assert(SUCCEEDED(hr));
		m_pSceneDepthTex->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pSceneDepthTex") - 1, "D3D12::m_pSceneDepthTex");
		
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		

		D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = m_pDescriptorHeapManager->GetCurDsvHandle();
		m_pDevice->CreateDepthStencilView(m_pScreenDepthTex, &dsvDesc, dsvCpuHandle);
		m_pScreenDSV = dsvCpuHandle;
		m_pDescriptorHeapManager->IncreaseDsvHandleOffset();
		
		dsvCpuHandle = m_pDescriptorHeapManager->GetCurDsvHandle();
		m_pDevice->CreateDepthStencilView(m_pSceneDepthTex, &dsvDesc, dsvCpuHandle);
		m_pSceneDSV = dsvCpuHandle;
		m_pDescriptorHeapManager->IncreaseDsvHandleOffset();

	
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

		if (m_pUAVTex != nullptr)
		{
			m_pUAVTex->Release();
			m_pUAVTex = nullptr;
		}

		for (int i = 0; i < 2; ++i)
		{	
			if (m_pPingPongTex[i] != nullptr)
			{	
				m_pPingPongTex[i]->Release();
				m_pPingPongTex[i] = nullptr;
			}

			
		}

		for (int i = 0; i < _GBUF_COUNT; ++i)
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

			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_pHDRTex));
			assert(SUCCEEDED(hr));
			
			m_pHDRTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pHDRTex") - 1, "D3D12::m_pHDRTex");

			UploadTexThroughCB(texDesc, sizeof(XMVECTOR)*width, (UINT8*)data, m_pHDRTex, &m_pHdrUploadCB, m_pPbrSetupCommandList);
			ID3D12CommandList* ppCommandList[1] = { m_pPbrSetupCommandList };
			ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											*(m_pDescriptorHeapManager->GetCbvSrvHeap()) };

			m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);

			D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
			
			m_pDevice->CreateShaderResourceView(m_pHDRTex, &srvDesc, cbvSrvCpuHandle);
			m_HDRSRV = cbvSrvGpuHandle;
			m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			
			return true;
		}
		else
		{
			return false;
		}
	}

	void D3D12::ConvertEquirectagular2Cube()
	{
		HRESULT hr;
		UINT8* pEquirect2CubeCB;
		D3D12_RANGE readRange = { 0, };
		hr = m_pEquirect2CubeCB->Map(0, &readRange, reinterpret_cast<void**>(&pEquirect2CubeCB));
		assert(SUCCEEDED(hr));
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
		XMMATRIX* pMatrix = reinterpret_cast<XMMATRIX*>(pEquirect2CubeCB);
		for (int i = 0; i < 6; ++i)
		{
			pMatrix[i] = XMMatrixMultiplyTranspose(capView[i], capProj);

		}
		m_pEquirect2CubeCB->Unmap(0, 0);

		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											*(m_pDescriptorHeapManager->GetCbvSrvHeap()) };
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
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											*(m_pDescriptorHeapManager->GetCbvSrvHeap()) };
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
		HRESULT hr;
		UINT8* pRoughnessCB;
		D3D12_RANGE readRange = { 0, };
		hr = m_pRoughnessCB->Map(0, &readRange, reinterpret_cast<void**>(&pRoughnessCB));
		assert(SUCCEEDED(hr));
		FLOAT r = 0;
		memcpy(pRoughnessCB, &r, sizeof(FLOAT));
		m_pRoughnessCB->Unmap(0, nullptr);

		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											*(m_pDescriptorHeapManager->GetCbvSrvHeap()) };
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
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											*(m_pDescriptorHeapManager->GetCbvSrvHeap()) };
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
		src.pResource = m_pUAVTex;


		UINT8* pMipCB;
		D3D12_RANGE readRange = {};
		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
	

		for (int i = 1; i < _PREFILTER_MIP_LEVELS; ++i)
		{
			{
				D3D12_RESOURCE_BARRIER srvToNsrvBarrier =
					CreateResourceBarrier(m_pPrefilterTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				m_pPbrSetupCommandList->ResourceBarrier(1, &srvToNsrvBarrier);

				uavDesc.Texture2DArray.MipSlice = i;

				cbvSrvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				cbvSrvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				m_pDevice->CreateUnorderedAccessView(m_pUAVTex, nullptr, &uavDesc, cbvSrvCpuHandle);
				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

				hr = m_pMipCB->Map(0, &readRange, reinterpret_cast<void**>(&pMipCB));
				assert(SUCCEEDED(hr));
				FLOAT texelSize[2] = { _PREFILTER_WIDTH >> i, _PREFILTER_HEIGHT >> i };

				memcpy(pMipCB, texelSize, sizeof(texelSize));
				m_pMipCB->Unmap(0, nullptr);

				m_pPbrSetupCommandList->SetPipelineState(m_pGenMipmapPso);
				m_pPbrSetupCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
				m_pPbrSetupCommandList->SetComputeRootSignature(m_pShader->GetGenMipShaderRootSingnature());
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(eGenMipRP::eGenMipRP_eCsTex, m_PrefilterSRV);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(eGenMipRP::eGenMipRP_eCsUAV, cbvSrvGpuHandle);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(eGenMipRP::eGenMipRP_eCsSampler, m_ClampSSV);
				m_pPbrSetupCommandList->SetComputeRootDescriptorTable(eGenMipRP::eGenMipRP_eCsCb, m_MipCBV);
				m_pPbrSetupCommandList->Dispatch(texelSize[0], texelSize[1], 6);//tex Dimesion


				D3D12_RESOURCE_BARRIER barriers[] = {
					CreateResourceBarrier(m_pPrefilterTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,  D3D12_RESOURCE_STATE_COPY_DEST),
					CreateResourceBarrier(m_pUAVTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
				};
				m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
				m_pPbrSetupCommandList->Close();


				ID3D12CommandList* pCommandList[] = { m_pPbrSetupCommandList };
				UINT fenceValue = m_fenceValue++;
				m_pCommandQueue->ExecuteCommandLists(1, pCommandList);
				m_pCommandQueue->Signal(m_pFence, fenceValue);
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
					CreateResourceBarrier(m_pUAVTex, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
				};

				m_pPbrSetupCommandList->ResourceBarrier(sizeof(barriers) / sizeof(D3D12_RESOURCE_BARRIER), barriers);
				m_pPbrSetupCommandList->Close();

				ID3D12CommandList* pCommandList[] = { m_pPbrSetupCommandList };
				UINT fenceValue = m_fenceValue++;
				m_pCommandQueue->ExecuteCommandLists(1, pCommandList);
				m_pCommandQueue->Signal(m_pFence, fenceValue);
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
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											*(m_pDescriptorHeapManager->GetCbvSrvHeap()) };
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

	void D3D12::DrawENTT(bool bGeoPass, bool bSpotShadowPass, UINT threadIndex) 
	{
		XMMATRIX worldMat, invWorldMat;
		UINT ENTTCnt = 0;
		UINT ENTTDrawn = 0;
		Plane* pPlanes = m_pFrustum->GetPlanes();

		for (int i = threadIndex; i < m_pTotalModels.size(); i+=_THREAD_COUNT)
		{
			ENTTCnt++;
			worldMat = m_pTotalModels[i]->GetTransformMatrix(false);
			invWorldMat = m_pTotalModels[i]->GetInverseWorldMatrix();
			AABB* aabb = m_pTotalModels[i]->GetAABB();
			if (aabb->IsOnFrustum(pPlanes, worldMat))
			{

				ENTTDrawn++;
				MatBuffer12* pMatBuffer = m_pTotalModels[i]->GetMatBuffer();
				pMatBuffer->SetWorldMatrix(&worldMat);
				pMatBuffer->SetInvWorldMatrix(&invWorldMat);
				pMatBuffer->SetViewMatrix(m_pCam->GetViewMatrix());
				pMatBuffer->SetProjMatrix(m_pCam->GetProjectionMatrix());
				if (bSpotShadowPass)
				{
					pMatBuffer->SetLightSpaceMatrix(m_pLitMat);
					pMatBuffer->UpdateCombinedMat(bSpotShadowPass);
					pMatBuffer->UploadCombinedMat(m_pWokerCommandList[threadIndex], bSpotShadowPass);
				}
				else
				{
					pMatBuffer->UploadMatBuffer(m_pWokerCommandList[threadIndex], bSpotShadowPass);
				}

				for (int j = 0; j < m_pTotalModels[i]->GetMatCount(); ++j)
				{
					if (bGeoPass)
					{
						PerModel perModel = *(m_pTotalModels[i]->GetPerModel(j));
						if (perModel.hasNormal)
						{
							if (m_bHeightOnOff)
							{
								if (perModel.hasEmissive)
								{
									m_pWokerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalHeightEmissivePso);
								}
								else
								{
									m_pWokerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalHeightPso);
								}
							}
							else
							{
								m_pWokerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoNormalPso);
							}
						}
						else if (perModel.hasEmissive)
						{
							m_pWokerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoEmissivePso);
						}
						else
						{
							m_pWokerCommandList[threadIndex]->SetPipelineState(m_pPbrDeferredGeoPso);
						}
					}

					m_pTotalModels[i]->UploadBuffers(m_pWokerCommandList[threadIndex], j, bGeoPass);
					m_pWokerCommandList[threadIndex]->DrawIndexedInstanced(m_pTotalModels[i]->GetIndexCount(j),1,
						0, 0, 0);
				}

			}
		}

		m_pFrustum->SetENTTsInFrustum(ENTTDrawn);
		m_pFrustum->SetENTTsInTotal(ENTTCnt);
	}

	void D3D12::AddModelGroup(ModelGroup12* pModelGroup)
	{
		m_pModelGroups.push_back(pModelGroup);
	}

	void D3D12::RemoveModelGroup(int i)
	{
		delete m_pModelGroups[i];
		m_pModelGroups.erase(m_pModelGroups.begin() + i);
	}

	void D3D12::RemoveModel(int i, int j)
	{
		std::vector<Model12*>& pModels = m_pModelGroups[i]->GetModels();
		delete pModels[j];
		pModels.erase(pModels.begin() + j);
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
		case eLIGHT_TYPE::PNT:
			size = m_pLightBuffer->GetPointLightSize();
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
		case eLIGHT_TYPE::PNT:
			m_pLightBuffer->PushPointLight((PointLight12*)pLight);
			break;
		case eLIGHT_TYPE::SPT:
			m_pLightBuffer->PushSpotLight((SpotLight12*)pLight);
		}
	}

	void D3D12::RemoveLight(int i, Light12* pLight)
	{
		std::vector<DirectionalLight12*>& pDirLights = m_pLightBuffer->GetDirLights();;
		std::vector<PointLight12*>& pPointLights = m_pLightBuffer->GetPointLights();
		std::vector<SpotLight12*>& pSpotLights = m_pLightBuffer->GetSpotLights();
		switch (pLight->GetType())
		{
		case eLIGHT_TYPE::DIR:
			delete pDirLights[i];
			pDirLights.erase(pDirLights.begin() + i);
			break;
		case eLIGHT_TYPE::PNT:
			delete pPointLights[i];
			pPointLights.erase(pPointLights.begin() + i);
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
		int drawed = 0;
		bool bGeoPass = false;
		bool bSpotShadowPass = false;

		D3D12_RANGE readRange = { 0, };
	
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											* (m_pDescriptorHeapManager->GetCbvSrvHeap()) };
		m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps)/sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pMainCommandList->SetPipelineState(m_pCascadeDirShadowPso);

	
		ID3D12Resource* pTextures[] = { m_pScreenTex[m_curFrame], m_pViewportTex, m_pSceneTex, m_pSSAOTex, m_pSSAOBlurTex, m_pSSAOBlurDebugTex};
		UINT textureCount = sizeof(pTextures) / sizeof(ID3D12Resource*) + _GBUF_COUNT;
		std::vector<D3D12_RESOURCE_BARRIER> barriers(textureCount);
		barriers[0] = CreateResourceBarrier(m_pScreenTex[m_curFrame], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers[1] = CreateResourceBarrier(m_pViewportTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
		barriers[2] = CreateResourceBarrier(m_pSceneTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers[3] = CreateResourceBarrier(m_pSSAOTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers[4] = CreateResourceBarrier(m_pSSAOBlurTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers[5] = CreateResourceBarrier(m_pSSAOBlurDebugTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		
		for (int i = textureCount-_GBUF_COUNT, j=0; i < textureCount; ++i,++j)
		{
			barriers[i] = CreateResourceBarrier(m_pGBufTex[j], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}

		m_pMainCommandList->ResourceBarrier(textureCount, &barriers[0]);
		
		m_pMainCommandList->ClearRenderTargetView(m_ScreenRTV[m_curFrame], color, 0, nullptr);
		m_pMainCommandList->ClearRenderTargetView(m_ViewportRTV, color, 0, nullptr);
		m_pMainCommandList->ClearRenderTargetView(m_SceneRTV, color, 0, nullptr);
		m_pMainCommandList->ClearRenderTargetView(m_SSAORTV, color, 0, nullptr);
		m_pMainCommandList->ClearRenderTargetView(m_SSAOBlurRTV, color, 0, nullptr);
		m_pMainCommandList->ClearRenderTargetView(m_SSAOBlurDebugRTV, color, 0, nullptr);
		
		for (int i = 0; i < _GBUF_COUNT; ++i)
		{	
			m_pMainCommandList->ClearRenderTargetView(m_GBufRTV[i], color, 0, nullptr);
		}

		m_pMainCommandList->ClearDepthStencilView(m_pScreenDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		m_pMainCommandList->ClearDepthStencilView(m_pSceneDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		UINT litCounts[] = { m_pLightBuffer->GetDirLightSize(), m_pLightBuffer->GetPointLightSize(), m_pLightBuffer->GetSpotLightSize() };
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
		m_pMainCommandList->Close();

		m_pCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pMainCommandList);
		UINT fenceValue = m_fenceValue++;
		m_pCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		m_pMainCommandAllocator->Reset();
		m_pMainCommandList->Reset(m_pMainCommandAllocator, nullptr);

		m_pCam->Update();

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
		

		//Shadow Pass
		for (int i = 0; i < _THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginFrame[i]);
		}
		WaitForMultipleObjects(_THREAD_COUNT, m_workerFinshShadowPass, TRUE, INFINITE);
		ID3D12CommandList* cmdLists[_THREAD_COUNT] = {};
		for (int i = 0; i < _THREAD_COUNT; ++i)
		{
			cmdLists[i]=m_pWokerCommandList[i];
		}
		m_pCommandQueue->ExecuteCommandLists(_THREAD_COUNT, cmdLists);
		fenceValue = m_fenceValue++;
		m_pCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		if (litCountSum) 
		{
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList, litCounts,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
			m_pShadowMap->SetResourceBarrier(m_pMainCommandList, litCounts,
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, false);
		}
		

		//Draw PbrGeo
		for (int i = 0; i < _THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginDeferredGeoPass[i]);
		}
		WaitForMultipleObjects(_THREAD_COUNT, m_workerEndFrame, TRUE, INFINITE);
		m_pCommandQueue->ExecuteCommandLists(_THREAD_COUNT, cmdLists);

		fenceValue = m_fenceValue++;
		m_pCommandQueue->Signal(m_pFence, fenceValue);
		if (m_pFence->GetCompletedValue() < fenceValue)
		{
			m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		D3D12_RESOURCE_BARRIER gbuf_barriers[_GBUF_COUNT] = {};
		for (int i = 0; i < _GBUF_COUNT; ++i)
		{
			gbuf_barriers[i] =
				CreateResourceBarrier(m_pGBufTex[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}

		m_pMainCommandList->ResourceBarrier(_GBUF_COUNT, gbuf_barriers);
	
		//Upload HeightScale and bHeightOnOff
		if (!m_pModelGroups.empty())
		{	
			//SSAO Pass
			m_pMainCommandList->SetPipelineState(m_pSSAOPso);
			m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
			m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetSSAOShaderRootSingnature());
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eSsao_ePsVpos, m_GBufSRV[eGbuf_vPos]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eSsao_ePsVnormal, m_GBufSRV[eGbuf_vNormal]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eSsao_ePsNoise, m_NoiseSrv);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eSsao_ePsWrap, m_WrapSSV);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eSsao_ePsClamp, m_ClampSSV);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eSsao_ePsSamplePoints, m_SSAOKernelCBV);
			m_pMatBuffer->UploadProjMat(m_pMainCommandList);
			m_pMainCommandList->IASetVertexBuffers(0, 1, &m_QuadVBV);
			m_pMainCommandList->IASetIndexBuffer(&m_QuadIBV);
			m_pMainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pMainCommandList->RSSetViewports(1, &m_viewport);
			m_pMainCommandList->RSSetScissorRects(1, &m_scissorRect);
			m_pMainCommandList->OMSetRenderTargets(1, &m_SSAORTV, TRUE, nullptr);
			m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);

			//Blur SSAOTex
			D3D12_CPU_DESCRIPTOR_HANDLE SsaoBlurRtvs[2] = { m_SSAOBlurRTV, m_SSAOBlurDebugRTV };
			D3D12_RESOURCE_BARRIER rtvToSrv = 
				CreateResourceBarrier(m_pSSAOTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_pMainCommandList->ResourceBarrier(1, &rtvToSrv);

			m_pMainCommandList->SetPipelineState(m_pSSAOBlurPso);
			m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetSSAOBlurShaderRootSingnature());
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eSsaoBlur_ePsSsao, m_SSAOSRV);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eSsaoBlur_ePsWrap, m_WrapSSV);
			m_pMainCommandList->OMSetRenderTargets(2, SsaoBlurRtvs, FALSE, nullptr);
			m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);

			//Lighting Pass
			D3D12_RESOURCE_BARRIER ssaoBarriers[] = {
				CreateResourceBarrier(m_pSSAOBlurTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				CreateResourceBarrier(m_pSSAOBlurDebugTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			};
			m_pMainCommandList->ResourceBarrier(2, ssaoBarriers);
			
			m_pMainCommandList->SetPipelineState(m_pLightingPso);
			m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetPBRDeferredLightingShaderRootSingnature());
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsPos, m_GBufSRV[eGbuf_pos]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsNormal, m_GBufSRV[eGbuf_normal]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsAlbedo, m_GBufSRV[eGbuf_albedo]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsSpecular, m_GBufSRV[eGbuf_specular]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsEmissive, m_GBufSRV[eGbuf_emissive]);
			m_pMainCommandList->SetGraphicsRootDescriptorTable(ePbrLight_ePsVpos, m_GBufSRV[eGbuf_vPos]);
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
			m_pCam->SetCamPos(m_pMainCommandList, bGeoPass);
			m_pCam->SetCascadeLevels(m_pMainCommandList);
			m_pLightBuffer->UpdateLightBuffer(m_pMainCommandList);
			m_pLightBuffer->UpdateDirLightMatrices(m_pMainCommandList);
			m_pLightBuffer->UpdateSpotLightMatrices(m_pMainCommandList);
			
			m_pMainCommandList->OMSetRenderTargets(1, &m_SceneRTV, TRUE, nullptr);
			m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);
		}
		else
		{
			if (m_pFrustum->GetENTTsInTotal())
			{
				m_pFrustum->SetENTTsInFrustum(0);
				m_pFrustum->SetENTTsInTotal(0);
			}

			//3 = SSAOTex, SSAOBlurTex, SSAOBlurDebugTex
			D3D12_RESOURCE_BARRIER barriers[3] = {
				CreateResourceBarrier(m_pSSAOTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				CreateResourceBarrier(m_pSSAOBlurTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				CreateResourceBarrier(m_pSSAOBlurDebugTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			};
		
			m_pMainCommandList->ResourceBarrier(3, barriers);
		}
		

		m_pMainCommandList->OMSetStencilRef(0);

		//Draw SkyBox
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
		m_pMainCommandList->OMSetRenderTargets(1, &m_SceneRTV, TRUE, &m_pSceneDSV);

		m_pMatBuffer->SetWorldMatrix(&m_idMat);
		m_pMatBuffer->SetViewMatrix(m_pCam->GetViewMatrix());
		m_pMatBuffer->SetProjMatrix(m_pCam->GetProjectionMatrix());
		m_pMatBuffer->UpdateCombinedMat(bSpotShadowPass);
		m_pMatBuffer->UploadCombinedMat(m_pMainCommandList, bSpotShadowPass);
		m_pMainCommandList->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);
	
		//Submit Result
		m_pMainCommandList->SetPipelineState(m_pFinalPso);
		m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
		m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetFinalShaderRootSingnature());
		m_pMainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pMainCommandList->IASetVertexBuffers(0, 1, &m_QuadVBV);
		m_pMainCommandList->IASetIndexBuffer(&m_QuadIBV);
		m_pMainCommandList->RSSetViewports(1, &m_viewport);
		m_pMainCommandList->RSSetScissorRects(1, &m_scissorRect);
		
		//Exporsure CBV 바인딩
		{
			memcpy(m_pExposureCbBegin, &m_exposure, sizeof(float));
			m_pMainCommandList->SetGraphicsRootDescriptorTable(eFinalRP::eFinal_ePsCb, m_ExposureCBV);
		}

		D3D12_RESOURCE_BARRIER rtvToSrv = CreateResourceBarrier(m_pSceneTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_pMainCommandList->ResourceBarrier(1, &rtvToSrv);

		m_pMainCommandList->SetGraphicsRootDescriptorTable(eFinalRP::eFinal_ePsTex, m_SceneSRV);
		m_pMainCommandList->SetGraphicsRootDescriptorTable(eFinalRP::eFinal_ePsSampler, m_WrapSSV);
		m_pMainCommandList->OMSetRenderTargets(1, &m_ViewportRTV, FALSE, &m_pSceneDSV);
		m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);

		//DrawUI
		rtvToSrv.Transition.pResource = m_pViewportTex;
		m_pMainCommandList->ResourceBarrier(1, &rtvToSrv);
		m_pMainCommandList->OMSetRenderTargets(1, &m_ScreenRTV[m_curFrame], FALSE, &m_pScreenDSV);
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
	uploadCbDesc.Width = _64KB_ALIGN(uploadSize);
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
	for (int y = 0; y < height; ++y)
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
		ID3D12DescriptorHeap* ppHeaps[2] = { *(m_pDescriptorHeapManager->GetSamplerHeap()),
											*(m_pDescriptorHeapManager->GetCbvSrvHeap()) };

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pMainCommandList);
		D3D12_RESOURCE_BARRIER rtvToPresent = 
			CreateResourceBarrier(m_pScreenTex[m_curFrame], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_pMainCommandList->ResourceBarrier(1, &rtvToPresent);
		m_pMainCommandList->Close();
		m_pPbrSetupCommandList->Close();
		m_pCommandQueue->ExecuteCommandLists(sizeof(ppCommandList) / sizeof(ID3D12CommandList*), ppCommandList);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, (void*)m_pMainCommandList);
		}

		UINT fenceValue = m_fenceValue++;
		m_pCommandQueue->Signal(m_pFence, fenceValue);
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
		
		m_curFrame = m_pSwapChain->GetCurrentBackBufferIndex();
	}

	D3D12::D3D12(int screenWidth, int screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
		float fScreenFar, float fScreenNear) :m_selectedModelGroup(-1), m_exposure(1.0f)
	{  
		g_pD3D12 = this;

		m_pDebugController = nullptr;
		m_pDevice = nullptr;
		m_pCommandQueue = nullptr;
		m_pSwapChain = nullptr;
		
		for (int i = 0; i < _BUFFER_COUNT; ++i)
		{
			m_pScreenTex[i] = nullptr;
		}

		for (int i = 0; i < _THREAD_COUNT; ++i)
		{
			m_pWorkerCommandAllocator[i] = nullptr;
			m_pWokerCommandList[i] = nullptr;
		}
		m_pMainCommandAllocator = nullptr;
		m_pMainCommandList = nullptr;
		m_pPbrSetupCommandAllocator = nullptr;
		m_pPbrSetupCommandList = nullptr;
		m_pUiCommandAllocator = nullptr;
		m_pUiCommandList = nullptr;
		m_pFence = nullptr;

		m_pCascadeDirShadowPso = nullptr;
		m_pSpotShadowPso = nullptr;
		m_pOmniDirShadowPso = nullptr;
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
		m_pLightingPso = nullptr;
		m_pFinalPso = nullptr;
		m_pBRDFPso = nullptr;
		m_pPrefilterPso = nullptr;
		m_pDiffuseIrradiancePso = nullptr;
		m_pEquirect2CubePso = nullptr;
		m_pGenMipmapPso = nullptr;

		m_pQuadVB = nullptr;
		m_pQuadIB = nullptr;
		m_pSkyBoxVB = nullptr;
		m_pSkyBoxIB = nullptr;
		m_pAABBVB = nullptr;
		m_pAABBIB = nullptr;
		m_pBoolCB = nullptr;
		m_pColorCB = nullptr;
		m_pRoughnessCB = nullptr;
		m_pSSAOKernelCB = nullptr;
		m_pExposureCB = nullptr;
		m_pHeightScaleCB = nullptr;
		m_pEquirect2CubeCB = nullptr;
		m_pMipCB = nullptr;
		m_pHdrUploadCB = nullptr;
		m_pNoiseUploadCB = nullptr;
		m_pNoiseTex = nullptr;

		m_pViewportTex = nullptr;
		m_pSceneTex = nullptr;
		m_pSSAOTex = nullptr;
		m_pSSAOBlurTex = nullptr;
		m_pSSAOBlurDebugTex = nullptr;
		m_pBrightTex = nullptr;
		m_pBRDFTex = nullptr;
		m_pSkyBoxTex = nullptr;
		m_pDiffIrradianceTex = nullptr;
		m_pPrefilterTex = nullptr;
		m_pUAVTex = nullptr;
		m_pScreenDepthTex = nullptr;
		m_pSceneDepthTex = nullptr;
		for (int i = 0; i < 2; ++i)
		{
			m_pPingPongTex[i] = nullptr;
		}

		for (int i = 0; i < _GBUF_COUNT; ++i)
		{
			m_pGBufTex[i] = nullptr;
		}


		m_pDescriptorHeapManager = nullptr;
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
		IDXGIFactory3* pFactory;
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

		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
		assert(SUCCEEDED(hr));
		m_pCommandQueue->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pCommandQueue") - 1, "D3D12::m_pCommandQueue");


		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		swapChainDesc = {};
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = _BUFFER_COUNT;
		swapChainDesc.Width = m_clientWidth;
		swapChainDesc.Height = m_clientHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
		if (m_bVsyncOn)
		{
			fullScreenDesc.RefreshRate.Numerator = _REFRESH_RATE;
			fullScreenDesc.RefreshRate.Denominator = 1;
		}
		else
		{
			fullScreenDesc.RefreshRate.Numerator = 0;
			fullScreenDesc.RefreshRate.Denominator = 1;
		}

		fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fullScreenDesc.Windowed = bFullscreen ? FALSE : TRUE;
		
		IDXGISwapChain1* pSwapChain;
		hr = pFactory->CreateSwapChainForHwnd(m_pCommandQueue, hWnd, 
			&swapChainDesc, &fullScreenDesc, 
			nullptr, 
			&pSwapChain);
		assert(SUCCEEDED(hr));

		pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		m_pSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pSwapChain") - 1, "D3D12::m_pSwapChain");
		m_curFrame = m_pSwapChain->GetCurrentBackBufferIndex();

		delete[] pDisplayModeList;
		pDisplayModeList = nullptr;

		pAdapterOutput->Release();
		pAdapterOutput = nullptr;

		pAdapter->Release();
		pAdapter = nullptr;

		pFactory->Release();
		pFactory = nullptr;

		//Gen DescriptorHeap
		m_pDescriptorHeapManager = new DescriptorHeapManager(m_pDevice);

		ImGui_ImplDX12_Init(m_pDevice, _BUFFER_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM, *(m_pDescriptorHeapManager->GetCbvSrvHeap()),
			m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle(),
			m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle());
		m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		//Gen Rtvs;
		{

			for (UINT i = 0; i < _BUFFER_COUNT; ++i)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pDescriptorHeapManager->GetCurRtvHandle();
				hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pScreenTex[i]));
				assert(SUCCEEDED(hr));
				m_pDevice->CreateRenderTargetView(m_pScreenTex[i], nullptr, rtvHandle);
				m_ScreenRTV[i] = rtvHandle;
				m_pDescriptorHeapManager->IncreaseRtvHandleOffset();

				m_pScreenTex[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pScreenTex") - 1, "D3D12::m_pScreenTex");

			}


		}
		

		hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_pMainCommandAllocator));
		assert(SUCCEEDED(hr));
		m_pMainCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pMainCommandAllocator") - 1, "D3D12::m_pMainCommandAllocator");

		hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_pPbrSetupCommandAllocator));
		assert(SUCCEEDED(hr));
		m_pPbrSetupCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pPbrSetupCommandAllocator") - 1, "D3D12::m_pPbrSetupCommandAllocator");


		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pMainCommandAllocator, nullptr, IID_PPV_ARGS(&m_pMainCommandList));
		assert(SUCCEEDED(hr));
		m_pMainCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pMainCommandList") - 1, "D3D12::m_pMainCommandList");

		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pPbrSetupCommandAllocator, nullptr, IID_PPV_ARGS(&m_pPbrSetupCommandList));
		assert(SUCCEEDED(hr));
		m_pPbrSetupCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pPbrSetupCommandList") - 1, "D3D12::m_pPbrSetupCommandList");

		for (int i = 0; i < _THREAD_COUNT; ++i)
		{
			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pWorkerCommandAllocator[i]));
			assert(SUCCEEDED(hr));
			m_pWorkerCommandAllocator[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pWorkerCommandAllocator[i]") - 1, "D3D12::m_pWorkerCommandAllocator[i]");

			hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pWorkerCommandAllocator[i], nullptr, IID_PPV_ARGS(&m_pWokerCommandList[i]));
			assert(SUCCEEDED(hr));
			m_pWokerCommandList[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pWokerCommandList[i]") - 1, "D3D12::m_pWokerCommandList[i]");

			m_pWokerCommandList[i]->Close();
		}

		//Gen User-Defined Class
		{
			m_pShader = new Shader12(m_pDevice);
			m_pCam = new Camera12(m_pDevice, m_pMainCommandList, m_pDescriptorHeapManager, screenWidth, screenHeight, fScreenFar, fScreenNear);
			XMMATRIX* m_projMat = m_pCam->GetProjectionMatrix();
			XMMATRIX* m_viewMat = m_pCam->GetViewMatrix();
			m_pFrustum = new Frustum12(m_pCam);
			m_pMatBuffer = new MatBuffer12(m_pDevice, m_pMainCommandList, m_pDescriptorHeapManager, m_viewMat, m_projMat);
			m_pOutlinerMatBuffer = new MatBuffer12(m_pDevice, m_pMainCommandList, m_pDescriptorHeapManager, m_viewMat, m_projMat);

			m_pLightBuffer = new LightBuffer12(m_pDevice, m_pMainCommandList, m_pDescriptorHeapManager);

			m_pShadowMap = new ShadowMap12(m_pDevice, m_pMainCommandList,
				m_pDescriptorHeapManager,
				_SHADOWMAP_SIZE, _SHADOWMAP_SIZE, m_pCam->GetCascadeLevels().size(),
				m_pLightBuffer->GetDirLightCapacity(), m_pLightBuffer->GetPointLightCapacity(), m_pLightBuffer->GetSpotLightCapacity());
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
			geoRDesc.CullMode = D3D12_CULL_MODE_BACK;
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
			lightingPassBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;

			lightingBS = { FALSE, FALSE, lightingPassBlendDesc };

			defaultBlendDesc = GBufferWriteRTBlendDesc;
			defaultBlendDesc.BlendEnable = FALSE;

			defaultBS = { FALSE, FALSE, defaultBlendDesc };
		}

		D3D12_DEPTH_STENCIL_DESC defaultDss = {};
		D3D12_DEPTH_STENCIL_DESC offDss = {};
		D3D12_DEPTH_STENCIL_DESC skyboxDSD = {};
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

			skyboxDSD.DepthEnable= TRUE;
			skyboxDSD.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc.Count = 1; //AA기능 추가 전까진 고정

			psoDesc.pRootSignature = m_pShader->GetCascadeDirShadowRootSingnature();
			m_pShader->SetPosOnlyInputLayout(&psoDesc);
			m_pShader->SetCascadeDirShadowShader(&psoDesc);
			hr= m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pCascadeDirShadowPso));

			psoDesc.pRootSignature = m_pShader->GetSpotShadowRootSingnature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetSpotShadowShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pSpotShadowPso));

			psoDesc.RasterizerState = skyboxRD;
			psoDesc.pRootSignature= m_pShader->GetOmniDirShadowRootSingnature();
			m_pShader->SetOmniDirShadowShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pOmniDirShadowPso));

			psoDesc.pRootSignature = m_pShader->GetSkyBoxRootSingnature();
			psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;//고정
			psoDesc.DepthStencilState = skyboxDSD;
			m_pShader->SetSkyBoxShader(&psoDesc);
			hr=m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pSkyBoxPso));


			for (int i = 0; i < _GBUF_COUNT; ++i)
			{
				psoDesc.RTVFormats[i] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			psoDesc.pRootSignature = m_pShader->GetPBRDeferredGeoShaderRootSingnature();
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.DepthStencilState = defaultDss;
			psoDesc.BlendState = defaultBS;//GBufferBS;
			psoDesc.NumRenderTargets = _GBUF_COUNT;
			m_pShader->SetDeferredGeoLayout(&psoDesc);
			m_pShader->SetPBRDeferredGeoShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoPso));
			m_pPbrDeferredGeoPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoPso") - 1, "D3D12:::m_pPbrDeferredGeoPso");
			m_pShader->SetPBRDeferredGeoEmissiveShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoEmissivePso));
			m_pPbrDeferredGeoEmissivePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoEmissivePso") - 1, "D3D12:::m_pPbrDeferredGeoEmissivePso");
			m_pShader->SetPBRDeferredGeoNormalShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalPso));
			m_pPbrDeferredGeoNormalPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalPso") - 1, "D3D12:::m_pPbrDeferredGeoNormalPso");

			m_pShader->SetPBRDeferredGeoNormalHeightShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalHeightPso));
			m_pPbrDeferredGeoNormalHeightPso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalHeightPso") - 1, "D3D12:::m_pPbrDeferredGeoNormalHeightPso");
			m_pShader->SetPBRDeferredGeoNormalHeightEmissiveShader(&psoDesc);
			hr =m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPbrDeferredGeoNormalHeightEmissivePso));
			m_pPbrDeferredGeoNormalHeightEmissivePso->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pPbrDeferredGeoNormalHeightEmissivePso") - 1, "D3D12:::m_pPbrDeferredGeoNormalHeightEmissivePso");
			//GeoPass에서 picking 한 모델만  Stencil값을 처리해주기 떄문에 pso에서 DSS만 바꿔줌. 
			psoDesc.DepthStencilState = outlinerSetupDss;
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pOutlinerSetupPso));

			for (int i = 1; i < _GBUF_COUNT; ++i)
			{
				psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			}

			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.NumRenderTargets = 1;
			psoDesc.pRootSignature = m_pShader->GetSSAOShaderRootSingnature();
			m_pShader->SetTexInputlayout(&psoDesc);
			m_pShader->SetSSAOShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pSSAOPso));


			psoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			psoDesc.NumRenderTargets = 2;
			psoDesc.pRootSignature = m_pShader->GetSSAOBlurShaderRootSingnature();
			m_pShader->SetSSAOBlurShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pSSAOBlurPso));


			psoDesc.BlendState = lightingBS;
			psoDesc.NumRenderTargets = _GBUF_COUNT - 2;
			psoDesc.pRootSignature = m_pShader->GetPBRDeferredLightingShaderRootSingnature();
			for (int i = 1; i < _GBUF_COUNT-2; ++i)
			{
				psoDesc.RTVFormats[i] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			m_pShader->SetPBRDeferredLightingShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pLightingPso));
			for (int i = 1; i < _GBUF_COUNT - 2; ++i)
			{
				psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			}

			//OutlinerTest
			psoDesc.NumRenderTargets = 1;
			psoDesc.BlendState = defaultBS;
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.DepthStencilState = outlinerTestDss;
			psoDesc.pRootSignature = m_pShader->GetOutlinerTestShaderRootSingnature();
			m_pShader->SetOutlinerTestShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pOutlinerTestPso));


			//Final
			psoDesc.DepthStencilState = offDss;
			psoDesc.pRootSignature = m_pShader->GetFinalShaderRootSingnature();
			m_pShader->SetFinalShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pFinalPso));

			//BRDF
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			psoDesc.pRootSignature = m_pShader->GetBRDFRootSingnature();
			m_pShader->SetBRDFShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pBRDFPso));

			//Prefilter
			psoDesc.pRootSignature = m_pShader->GetPrefilterRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetPosOnlyInputLayout(&psoDesc);
			m_pShader->SetPrefilterShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPrefilterPso));

			//DiffuseIrradiance
			psoDesc.pRootSignature = m_pShader->GetDiffuseIrradianceRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetDiffuseIrradianceShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pDiffuseIrradiancePso));

			//Equirect2Cube
			psoDesc.pRootSignature = m_pShader->GetEquirect2CubeRootSingnature();
			psoDesc.BlendState = defaultBS;
			psoDesc.DepthStencilState = offDss;
			psoDesc.RasterizerState = quadRDesc;
			m_pShader->SetEquirect2CubeShader(&psoDesc);
			m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pEquirect2CubePso));

			D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
			computePsoDesc.pRootSignature = m_pShader->GetGenMipShaderRootSingnature();
			m_pShader->SetGenMipShader(&computePsoDesc);
			m_pDevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_pGenMipmapPso));

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
		}

		//Gen Samplers
		{
			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = m_pDescriptorHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = m_pDescriptorHeapManager->GetCurSamplerGpuHandle();

				D3D12_SAMPLER_DESC wrapSamplerDesc = {};
				wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				m_pDevice->CreateSampler(&wrapSamplerDesc, samplerCpuHandle);

				m_WrapSSV = samplerGpuHandle;
				m_pDescriptorHeapManager->IncreaseSamplerHandleOffset();
			}
			

			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = m_pDescriptorHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = m_pDescriptorHeapManager->GetCurSamplerGpuHandle();

				D3D12_SAMPLER_DESC clampSamplerDesc = {};
				clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				m_pDevice->CreateSampler(&clampSamplerDesc, samplerCpuHandle);

				m_ClampSSV = samplerGpuHandle;
				m_pDescriptorHeapManager->IncreaseSamplerHandleOffset();
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
			cbufferDesc.Width = _64KB_ALIGN(sizeof(BOOL) * 4);
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


			{
				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				UINT constantBufferSize = sizeof(BOOL) * 4;
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pBoolCB));
				assert(SUCCEEDED(hr));
				m_pBoolCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pBoolCB") - 1, "D3D12::m_pBoolCB");

				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pBoolCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
				m_BoolCBV = cbvGpuHandle;

				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

			}
			
			
			{
				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

				cbufferDesc.Width = _64KB_ALIGN(sizeof(XMVECTOR)); 
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pColorCB));
				assert(SUCCEEDED(hr));
				m_pColorCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pColorCB") - 1, "D3D12::m_pColorCB");


				UINT constantBufferSize = sizeof(XMVECTOR);
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pColorCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
				m_ColorCBV = cbvGpuHandle;

				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
			
			{
				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

				cbufferDesc.Width = _64KB_ALIGN(sizeof(XMVECTOR));
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pRoughnessCB));
				assert(SUCCEEDED(hr));
				m_pRoughnessCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pRoughnessCB") - 1, "D3D12::m_pRoughnessCB");


				UINT constantBufferSize = sizeof(XMVECTOR);
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pRoughnessCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
				m_RoughnessCBV = cbvGpuHandle;

				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}

			{	
				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

				cbufferDesc.Width = _64KB_ALIGN(sizeof(SamplePoints));
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pSSAOKernelCB));
				assert(SUCCEEDED(hr));
				m_pSSAOKernelCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pSSAOKernelCB") - 1, "D3D12::m_pSSAOKernelCB");


				UINT constantBufferSize = sizeof(SamplePoints);
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pSSAOKernelCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
				m_SSAOKernelCBV = cbvGpuHandle;

				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
			

			{
				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

				cbufferDesc.Width = _64KB_ALIGN(sizeof(XMFLOAT4));
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pExposureCB));
				assert(SUCCEEDED(hr));
				m_pExposureCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pExposureCB") - 1, "D3D12::m_pExposureCB");

				hr = m_pExposureCB->Map(0, &readRange, reinterpret_cast<void**>(&m_pExposureCbBegin));
				assert(SUCCEEDED(hr));


				UINT constantBufferSize = sizeof(XMFLOAT4);
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pExposureCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
				m_ExposureCBV = cbvGpuHandle;

				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

			}

			{	
				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

				cbufferDesc.Width = _64KB_ALIGN(sizeof(XMFLOAT4)); 
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pHeightScaleCB));
				assert(SUCCEEDED(hr));
				m_pHeightScaleCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pHeightScaleCB") - 1, "D3D12::m_pHeightScaleCB");

				hr = m_pHeightScaleCB->Map(0, &readRange, reinterpret_cast<void**>(&m_pHeightScaleCbBegin));
				assert(SUCCEEDED(hr));
				UINT constantBufferSize = sizeof(XMFLOAT4);
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pHeightScaleCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
				m_HeightScaleCBV = cbvGpuHandle;

				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
			
			{
				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

				cbufferDesc.Width = _64KB_ALIGN(sizeof(XMMATRIX) * 6); 
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pEquirect2CubeCB));
				assert(SUCCEEDED(hr));
				m_pEquirect2CubeCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pEquirect2CubeCB") - 1, "D3D12::m_pEquirect2CubeCB");


				UINT constantBufferSize = sizeof(XMMATRIX) * 6;
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pEquirect2CubeCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
				m_Equirect2CubeCBV = cbvGpuHandle;

				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}

			{
				D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

				cbufferDesc.Width = 64 * 1024;
				hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
					&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pMipCB));
				assert(SUCCEEDED(hr));
				m_pMipCB->SetPrivateData(WKPDID_D3DDebugObjectName,
					sizeof("D3D12::m_pMipCB") - 1, "D3D12::m_pMipCB");


				UINT constantBufferSize = sizeof(UINT) * 2;
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
				cbvDesc.BufferLocation = m_pMipCB->GetGPUVirtualAddress();
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
				m_MipCBV = cbvGpuHandle;

				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

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


			UINT8* pKernelCB;
			D3D12_RANGE readRange = { 0, };
			hr = m_pSSAOKernelCB->Map(0, &readRange, reinterpret_cast<void**>(&pKernelCB));
			assert(SUCCEEDED(hr));

			memcpy(pKernelCB, &kernels, sizeof(kernels));
			m_pSSAOKernelCB->Unmap(0, nullptr);



			XMFLOAT3 noiseVecs[_NOISE_VEC_COUNT];
			//Gen noise texture
			for (int i = 0; i < _NOISE_VEC_COUNT; ++i)
			{
				XMFLOAT3 rot(
					randomFloats(gen) * 2.0f - 1.0f,
					randomFloats(gen) * 2.0f - 1.0f,
					0.0f);
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

			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, 
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_pNoiseTex));
			assert(SUCCEEDED(hr));
			m_pNoiseTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12:::m_pNoiseTex") - 1, "D3D12:::m_pNoiseTex");

			//Gen Cbuffer for Upload
			UploadTexThroughCB(texDesc, sizeof(XMVECTOR)*_NOISE_TEX_SIZE, (UINT8*)noiseVecs, m_pNoiseTex, &m_pNoiseUploadCB, m_pMainCommandList);
				
			D3D12_RESOURCE_BARRIER copyDstToSrvBarrier =
				CreateResourceBarrier(m_pNoiseTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_pMainCommandList->ResourceBarrier(1, &copyDstToSrvBarrier);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = texDesc.Format;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
			m_pDevice->CreateShaderResourceView(m_pNoiseTex, &srvDesc, cbvCpuHandle);
			m_NoiseSrv = cbvGpuHandle;
			m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}

		//GenQuad
		{
			XMFLOAT3 vertices[] = {
				{XMFLOAT3(-1.0f, -1.0f, 0.0f)},
				{XMFLOAT3(-1.0f, 1.0f, 0.0f)},
				{XMFLOAT3(1.0f, 1.0f, 0.0f)},
				{XMFLOAT3(1.0f, -1.0f, 0.0f)}
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

			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC quadDesc = {};
			quadDesc.Width = quadsVBSize;
			quadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			quadDesc.Alignment = 0;
			quadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			quadDesc.Format = DXGI_FORMAT_UNKNOWN;
			quadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			quadDesc.Height = 1;
			quadDesc.DepthOrArraySize = 1;
			quadDesc.MipLevels = 1;
			quadDesc.SampleDesc.Count = 1;
			quadDesc.SampleDesc.Quality = 0;

			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &quadDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pQuadVB));
			assert(SUCCEEDED(hr));
			m_pQuadVB->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pQuadVB") - 1, "D3D12::m_pQuadVB");


			UINT8* pQuadVB;
			D3D12_RANGE readRange = { 0, };
			hr = m_pQuadVB->Map(0, &readRange, reinterpret_cast<void**>(&pQuadVB));
			assert(SUCCEEDED(hr));
			memcpy(pQuadVB, quads, quadsVBSize);
			m_pQuadVB->Unmap(0, nullptr);

			m_QuadVBV.BufferLocation = m_pQuadVB->GetGPUVirtualAddress();
			m_QuadVBV.SizeInBytes = quadsVBSize;
			m_QuadVBV.StrideInBytes = sizeof(QUAD);



			unsigned long indices[6] = { 0,1,2, 2,3,0 };
			UINT quadsIBSize = sizeof(indices);

			quadDesc.Width = quadsIBSize;

			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &quadDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pQuadIB));
			assert(SUCCEEDED(hr));
			m_pQuadIB->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pQuadIB") - 1, "D3D12::m_pQuadIB");


			UINT8* pQuadIB;
			hr = m_pQuadIB->Map(0, &readRange, reinterpret_cast<void**>(&pQuadIB));
			assert(SUCCEEDED(hr));
			memcpy(pQuadIB, indices, quadsIBSize);
			m_pQuadIB->Unmap(0, nullptr);

			m_QuadIBV.BufferLocation = m_pQuadIB->GetGPUVirtualAddress();
			m_QuadIBV.SizeInBytes = quadsIBSize;
			m_QuadIBV.Format = DXGI_FORMAT_R32_UINT;
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

			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC skyDesc = {};
			skyDesc.Width = skyVBSize;
			skyDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			skyDesc.Alignment = 0;
			skyDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			skyDesc.Format = DXGI_FORMAT_UNKNOWN;
			skyDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			skyDesc.Height = 1;
			skyDesc.DepthOrArraySize = 1;
			skyDesc.MipLevels = 1;
			skyDesc.SampleDesc.Count = 1;
			skyDesc.SampleDesc.Quality = 0;


			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &skyDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pSkyBoxVB));
			assert(SUCCEEDED(hr));
			m_pSkyBoxVB->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSkyBoxVB") - 1, "D3D12::m_pSkyBoxVB");


			UINT8* pSkyBoxVB;
			D3D12_RANGE readRange = { 0, };
			hr = m_pSkyBoxVB->Map(0, &readRange, reinterpret_cast<void**>(&pSkyBoxVB));
			assert(SUCCEEDED(hr));
			memcpy(pSkyBoxVB, vertices, skyVBSize);
			m_pSkyBoxVB->Unmap(0, nullptr);

			m_SkyBoxVBV.BufferLocation = m_pSkyBoxVB->GetGPUVirtualAddress();
			m_SkyBoxVBV.SizeInBytes = skyVBSize;
			m_SkyBoxVBV.StrideInBytes = sizeof(XMFLOAT3);

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

			skyDesc.Width = skyIBSize;

			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &skyDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pSkyBoxIB));
			assert(SUCCEEDED(hr));
			m_pSkyBoxIB->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pSkyBoxIB") - 1, "D3D12::m_pSkyBoxIB");


			UINT8* pSkyBoxIB;
			hr = m_pSkyBoxIB->Map(0, &readRange, reinterpret_cast<void**>(&pSkyBoxIB));
			assert(SUCCEEDED(hr));
			memcpy(pSkyBoxIB, indices, skyIBSize);
			m_pSkyBoxIB->Unmap(0, nullptr);

			m_SkyBoxIBV.BufferLocation = m_pSkyBoxIB->GetGPUVirtualAddress();
			m_SkyBoxIBV.SizeInBytes = skyIBSize;
			m_SkyBoxIBV.Format = DXGI_FORMAT_R32_UINT;

		}

		//GenAABB Debug 
		{
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC AABBDesc = {};
			AABBDesc.Width = sizeof(DirectX::XMFLOAT3) * 8;
			AABBDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			AABBDesc.Alignment = 0;
			AABBDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			AABBDesc.Format = DXGI_FORMAT_UNKNOWN;
			AABBDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			AABBDesc.Height = 1;
			AABBDesc.DepthOrArraySize = 1;
			AABBDesc.MipLevels = 1;
			AABBDesc.SampleDesc.Count = 1;
			AABBDesc.SampleDesc.Quality = 0;

			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &AABBDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pAABBVB));
			assert(SUCCEEDED(hr));
			m_pAABBVB->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pAABBVB") - 1, "D3D12::m_pAABBVB");

			m_AABBVBV.BufferLocation = m_pAABBVB->GetGPUVirtualAddress();
			m_AABBVBV.SizeInBytes = AABBDesc.Width;
			m_AABBVBV.StrideInBytes = sizeof(DirectX::XMFLOAT3);




			unsigned long cubeIndices[24] = { 0, 1,   1, 2,   2, 3,  3,0 ,
					   0, 4,   1, 5,   2, 6,  3,7,
					   4, 5,   5, 6,   6, 7,  7,0 };

			UINT AABBIBSize = sizeof(cubeIndices);
			AABBDesc.Width = AABBIBSize;

			hr = m_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &AABBDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pAABBIB));
			assert(SUCCEEDED(hr));
			m_pAABBIB->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("D3D12::m_pAABBIB") - 1, "D3D12::m_pAABBIB");


			UINT8* pAABBIB;
			D3D12_RANGE readRange = { 0, };

			hr = m_pAABBIB->Map(0, &readRange, reinterpret_cast<void**>(&pAABBIB));
			assert(SUCCEEDED(hr));
			memcpy(pAABBIB, cubeIndices, AABBIBSize);
			m_pAABBIB->Unmap(0, nullptr);

			m_AABBIBV.BufferLocation = m_pAABBIB->GetGPUVirtualAddress();
			m_AABBIBV.SizeInBytes = AABBIBSize;
			m_AABBIBV.Format = DXGI_FORMAT_R32_UINT;
		}

		if (!CreateRTVandSRV(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D12::CreateRTVandSRV()Failed");
		}

		if (!CreateDSV(m_clientWidth, m_clientHeight))
		{
			OutputDebugStringA("D3D12::CreateDSV()onScreenFailed");
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

		for (int i = 0; i < _THREAD_COUNT; ++i)
		{
			m_workerBeginFrame[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerFinshShadowPass[i] = CreateEvent(
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
				&WrapperThreadFun,		 //생성시 호출함수
				reinterpret_cast<LPVOID>(&m_workerThreadIdx[i]),//전달 인자
				0,							//즉시 실행
				nullptr ));
		}
		
	}
	D3D12::~D3D12()
	{	
		WaitForGpu();
		ImGui_ImplDX12_Shutdown();
		DestroyHDR();
		DestroyTexture();
		DestroySceneDepthTex();
		DestroyBackBuffer();
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

		if (m_pCommandQueue != nullptr)
		{
			m_pCommandQueue->Release();
			m_pCommandQueue = nullptr;
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

		if (m_pMainCommandAllocator != nullptr)
		{
			m_pMainCommandAllocator->Release();
			m_pMainCommandAllocator = nullptr;
		}

		if (m_pPbrSetupCommandAllocator != nullptr)
		{
			m_pPbrSetupCommandAllocator->Release();
			m_pPbrSetupCommandAllocator = nullptr;
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
		
		if (m_pOmniDirShadowPso != nullptr)
		{
			m_pOmniDirShadowPso->Release();
			m_pOmniDirShadowPso = nullptr;
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
		
		if (m_pLightingPso != nullptr)
		{
			m_pLightingPso->Release();
			m_pLightingPso = nullptr;
		}
		
		if (m_pFinalPso != nullptr)
		{
			m_pFinalPso->Release();
			m_pFinalPso = nullptr;
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

		if (m_pQuadVB != nullptr)
		{
			m_pQuadVB->Release();
			m_pQuadVB = nullptr;
		}

		if (m_pQuadIB != nullptr)
		{
			m_pQuadIB->Release();
			m_pQuadIB = nullptr;
		}

		if (m_pSkyBoxVB != nullptr)
		{
			m_pSkyBoxVB->Release();
			m_pSkyBoxVB = nullptr;
		}

		if (m_pSkyBoxIB != nullptr)
		{
			m_pSkyBoxIB->Release();
			m_pSkyBoxIB = nullptr;
		}

		if (m_pAABBVB != nullptr)
		{
			m_pAABBVB->Release();
			m_pAABBVB = nullptr;
		}

		if (m_pAABBIB != nullptr)
		{
			m_pAABBIB->Release();
			m_pAABBIB = nullptr;
		}

		if (m_pBoolCB != nullptr)
		{
			m_pBoolCB->Release();
			m_pBoolCB = nullptr;
		}

		if (m_pColorCB != nullptr)
		{	
			m_pColorCB->Release();
			m_pColorCB = nullptr;
		}

		if (m_pSSAOKernelCB != nullptr)
		{
			m_pSSAOKernelCB->Release();
			m_pSSAOKernelCB = nullptr;
		}

		if (m_pExposureCB != nullptr)
		{	
			m_pExposureCB->Release();
			m_pExposureCB = nullptr;
		}

		if (m_pHeightScaleCB != nullptr)
		{	
			m_pHeightScaleCB->Release();
			m_pHeightScaleCB = nullptr;
		}


		if (m_pRoughnessCB != nullptr)
		{
			m_pRoughnessCB->Release();
			m_pRoughnessCB = nullptr;
		}

		if (m_pEquirect2CubeCB != nullptr)
		{	
			m_pEquirect2CubeCB->Release();
			m_pEquirect2CubeCB = nullptr;
		}

		if (m_pMipCB != nullptr)
		{
			m_pMipCB->Release();
			m_pMipCB = nullptr;
		}

		if (m_pHdrUploadCB != nullptr)
		{
			m_pHdrUploadCB->Release();
			m_pHdrUploadCB = nullptr;
		}

		if (m_pNoiseUploadCB != nullptr)
		{
			m_pNoiseUploadCB->Release();
			m_pNoiseUploadCB = nullptr;
		}

		if (m_pNoiseTex != nullptr)
		{
			m_pNoiseTex->Release();
			m_pNoiseTex = nullptr;
		}

		if (m_pMainCommandList != nullptr)
		{
			m_pMainCommandList->Release();
			m_pMainCommandList = nullptr;
		}

		if (m_pPbrSetupCommandList != nullptr)
		{
			m_pPbrSetupCommandList->Release();
			m_pPbrSetupCommandList = nullptr;
		}

		for (int i = 0; i < _THREAD_COUNT; ++i)
		{
			if (m_pWokerCommandList[i] != nullptr)
			{
				m_pWokerCommandList[i]->Release();
				m_pWokerCommandList[i] = nullptr;
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

		if (m_pDescriptorHeapManager != nullptr)
		{	
			delete m_pDescriptorHeapManager;
			m_pDescriptorHeapManager = nullptr;
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

