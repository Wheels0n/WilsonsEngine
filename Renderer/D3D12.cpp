
#include "D3D12.h"
#include "typedef.h"
#include "HierarchicalZCull.h"
#include "CommandListWrapperPool.h"
#include "BackBuffer.h"
#include "PSOManager.h"
#include "PBRResourceManager.h"
#include "PostProcess.h"
#include "ResourceBarrierHandler.h"
#include "SSAOResourceManager.h"
#include "SamplerManager.h"
#include "Scene12.h"
#include "../ImGui/imgui_impl_dx12.h"

namespace wilson
{	

	ComPtr<ID3D12Device1> D3D12::m_pDevice = nullptr;

	void D3D12::BeginWorkers()
	{
		m_ssaoThreadArg = { weak_from_this(), };
		m_ssaoThreadHandle = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr,
			0,
			&WrapperSsaoThreadFun,
			reinterpret_cast<LPVOID>(&m_ssaoThreadArg),
			0,
			nullptr));

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			m_workerThreadArgs[i].pInstance = weak_from_this();
			m_workerThreadArgs[i].threadIdx = i;

			m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
				nullptr,
				0,
				&WrapperWorkerThreadFun,
				reinterpret_cast<LPVOID>(&m_workerThreadArgs[i]),
				0,
				nullptr));
		}

	}
	void D3D12::EndWorkers()
	{
		HANDLE handles[_WORKER_THREAD_COUNT + 1];
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			handles[i] = m_workerEndShutdown[i];
			SetEvent(m_workerBeginShutdown[i]);
		}
		SetEvent(m_ssaoBeginShutDown);
		handles[_WORKER_THREAD_COUNT] = m_ssaoEndShutDown;

		WaitForMultipleObjects(_WORKER_THREAD_COUNT + 1, handles, TRUE, INFINITE);
	}
	void D3D12::WorkerThread(const UINT threadIndex)
	{
		while (true)
		{
			//Zpass or Exit
			HANDLE handles[2] = { m_workerBeginShutdown[threadIndex],m_workerBeginFrame[threadIndex]};
			DWORD result = WaitForMultipleObjects(sizeof(handles) / sizeof(HANDLE), handles, FALSE, INFINITE);
			if (result == WAIT_OBJECT_0)
			{
				SetEvent(m_workerEndShutdown[threadIndex]);
				break;
			}

			DrawObject(ePass::zPass, threadIndex, _INVALID_INDEX);
			SetEvent(m_workerFinishZpass[threadIndex]);

			//ShadowPass
			WaitForSingleObject(m_workerBeginShadowPass[threadIndex], INFINITE);

			shared_ptr<LightBuffer12> pLightCb = m_curScene->GetLightCb();
			vector<shared_ptr<DirectionalLight12>>& dirLights = pLightCb->GetDirLights();
			vector<shared_ptr<CubeLight12>>& CubeLights = pLightCb->GetCubeLights();
			vector<shared_ptr<SpotLight12>>& spotLights = pLightCb->GetSpotLights();
			UINT nLights[3] = { dirLights.size(), CubeLights.size(), spotLights.size() };
			UINT nLightTotal = nLights[E_TO_UINT(eLIGHT_TYPE::DIR)]
				+ nLights[E_TO_UINT(eLIGHT_TYPE::CUBE)]
				+ nLights[E_TO_UINT(eLIGHT_TYPE::SPT)];
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
		ThreadArgs* threadArgs = reinterpret_cast<ThreadArgs*>(pParameter);
		UINT threadIndex = threadArgs->threadIdx;
		assert(threadIndex < _WORKER_THREAD_COUNT);
		shared_ptr<D3D12> pInstance = threadArgs->pInstance.lock();
		pInstance->WorkerThread(threadIndex);
		return 0;
	}
	void D3D12::SsaoThread()
	{
		while (true)
		{	
			//SSAO or Exit
			HANDLE handles[2] = { m_ssaoBeginShutDown,m_ssaoBeginFrame };
			DWORD result = WaitForMultipleObjects(sizeof(handles) / sizeof(HANDLE), handles, FALSE, INFINITE);
			if (result == WAIT_OBJECT_0)
			{
				SetEvent(m_ssaoEndShutDown);
				break;
			}
			shared_ptr<CommandListWrapper> pCmdListWrapper = g_pComputeCmdListPool->Pop();
			ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

			m_psoManager			->SetSSAO(pCmdList);
			g_pHeapManager			->SetDescriptorHeaps(pCmdList);
			g_pSamplerManager		->UploadWrapSampler(pCmdList, E_TO_UINT(eSsaoRP::csWrap), true);
			g_pSamplerManager		->UploadClampPointSampler(pCmdList, E_TO_UINT(eSsaoRP::csClamp), true);

			m_backBuffer			->SetSSAOPass(pCmdList);
			m_ssaoResourceManager	->SetSSAOPass(pCmdList);
		
			pCmdList->Dispatch(ceil(m_clientWidth / (float)8), ceil(m_clientHeight / (float)8), 1);

			//Blur SSAOTex
			
			m_ssaoResourceManager->SetSAAOBlurPassBarriers();
			g_pResourceBarrierHandler->Flush(pCmdList);

			m_psoManager			->SetSSAOBlur(pCmdList);
			g_pSamplerManager		->UploadWrapSampler(pCmdList, E_TO_UINT(eSsaoBlurRP::csWrap), true);
			m_ssaoResourceManager	->SetSSAOBlurPass(pCmdList);
			pCmdList				->Dispatch(ceil(m_clientWidth / (float)8), ceil(m_clientHeight / (float)8), 1);


			g_pComputeCmdListPool->Execute(pCmdListWrapper);
			g_pComputeCmdListPool->Push(pCmdListWrapper);
			SetEvent(m_ssaoEndFrame);
		}
		return;
	}
	UINT __stdcall D3D12::WrapperSsaoThreadFun(LPVOID pParameter)
	{
		ThreadArgs* threadArgs = reinterpret_cast<ThreadArgs*>(pParameter);
		shared_ptr<D3D12> pInstance = threadArgs->pInstance.lock();
		pInstance->SsaoThread();
		return 0;
	}

	void D3D12::ResizeBackBuffer(const UINT newWidth, const UINT newHeight)
	{
		g_pGraphicsCmdListPool->WaitForGpu();
		g_pComputeCmdListPool->WaitForGpu();

		m_backBuffer->Shutdown();

		m_clientWidth = newWidth;
		m_clientHeight = newHeight;

		m_pSwapChain->ResizeBuffers(_BUFFER_COUNT, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_EFFECT_FLIP_DISCARD);
		m_backBuffer->Init(m_pSwapChain, m_clientWidth, m_clientHeight);
		m_ssaoResourceManager->Init(m_clientWidth,m_clientHeight);
		
	}
	void D3D12::DrawObject(const ePass curPass, const UINT threadIndex, const UINT lightIdx)
	{
		XMMATRIX worldMat, invWorldMat;
		shared_ptr<LightBuffer12> pLightCb = m_curScene->GetLightCb();
		shared_ptr<ShadowMap12> pShadowMap = m_curScene->GetShadowMap();
	
		UINT pass = E_TO_UINT(curPass);
		while (1)
		{
			HANDLE handles[2] = { m_drawFinishedEvent ,m_dataReadyEvent};
			DWORD result = WaitForMultipleObjects(sizeof(handles)/sizeof(HANDLE), handles, FALSE, INFINITE);
			if (result == WAIT_OBJECT_0)
			{
				break;
			}
			
			{	
				if(m_lastMeshletIdx==0)
				{

					UINT context = 0;
					for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
					{
						for (int j = 0; j < _CONTEXT_COUNT; ++j)
						{
							//PIXBeginEvent(pCmdList, 0, L"ZPass");
						}
					}
				}
				UINT nMeshes = min(m_pMeshletQueue[pass].size() - m_lastMeshletIdx, _OBJECT_PER_THREAD);

				if (nMeshes == 0)
				{
					SetEvent(m_dataEmptyEvent);
					return;
				}

				UINT startMeshletIdx = m_lastMeshletIdx;
				m_lastMeshletIdx += nMeshes;
				UINT lastMeshletIdx = m_lastMeshletIdx;
				m_nDrawCall += nMeshes;

				//Set CB, SRV,

				shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
				ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();
				{
					shared_ptr<Mesh12> pParentMesh = m_pMeshletQueue[pass][startMeshletIdx]->GetParentMesh();

					SetPSO(pCmdList, curPass, pParentMesh);

					g_pHeapManager->SetDescriptorHeaps(pCmdList);
					pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					{
						switch (curPass)
						{
						case ePass::zPass:
						{
							g_pSamplerManager->UploadWrapSampler(pCmdList, E_TO_UINT(eZpassRP::psWrap), false);
							m_backBuffer->SetZPass(pCmdList);
						}
						break;
						case ePass::cascadeDirShadowPass:
						{
							pLightCb->SetDirPass(pCmdList, lightIdx);
							pShadowMap->SetDirPass(pCmdList,lightIdx);
						}
						break;
						case ePass::spotShadowPass:
						{
							g_pSamplerManager->UploadWrapSampler(pCmdList, E_TO_UINT(eSpotShadowRP::psSampler), false);
							pLightCb	->SetSpotPass(pCmdList, lightIdx);
							pShadowMap	->SetSpotPass(pCmdList, lightIdx);
						}
						break;
						case ePass::cubeShadowPass:
						{
							g_pSamplerManager->UploadWrapSampler(pCmdList, E_TO_UINT(eCubeShadowRP::psSampler), false);
							pShadowMap	->SetCubePass(pCmdList, lightIdx);
							pLightCb	->SetCubePass(pCmdList, lightIdx);
						}
						break;
						case ePass::geoPass:
						{
							g_pSamplerManager->UploadWrapSampler(pCmdList, E_TO_UINT(ePbrGeoRP::psSampler), false);
							m_pCam->UploadCamPos(pCmdList, true);
							m_backBuffer->SetGeoPass(pCmdList);
						}
						break;
						default:
							break;
						}

					}
					SetShaderResource(pCmdList, curPass, m_pMeshletQueue[pass][startMeshletIdx], lightIdx);
				}
	
				SetEvent(m_dataEmptyEvent);


				for (UINT i = startMeshletIdx; i < lastMeshletIdx; ++i)
				{	
					shared_ptr<Mesh12> pParentMesh = m_pMeshletQueue[pass][i]->GetParentMesh();
					
					int meshletIdx = m_pMeshletQueue[pass][i]->GetMeshletIdx();
					
					pParentMesh->UploadIB(pCmdList, meshletIdx);
					pCmdList->DrawIndexedInstanced(pParentMesh->GetIndexCount(meshletIdx), 1,
						0, 0, 0);
				}
				g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
				g_pGraphicsCmdListPool->Push(pCmdListWrapper);
			}
		}
		

	}
	void D3D12::HiZCull(const UINT threadIndex)
	{
		//Hi-Z Occlusion Pass
		{
			UINT hi_zPass = E_TO_UINT(ePass::hi_zPass);
			UINT hwOcclusionPass = E_TO_UINT(ePass::HWOcclusionPass);
			UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(_HI_Z_MIP_WIDTH, _HI_Z_MIP_HEIGHT)));

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
					ReleaseMutex(m_workerMutex);

					m_pHiZQueue[threadIndex].reserve(nMeshes);
					
					shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
					ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

					UINT threadGroupXCnt = sqrt(_HI_Z_CULL_COUNT);
					vector<XMVECTOR> vertices(nMeshes * _CUBE_VERTICES);
					vector<XMMATRIX> matrices(nMeshes);
					for (int i = startMeshletIdx; i < lastMeshletIdx; ++i)
					{
						shared_ptr<Meshlet> pMeshlet = m_pMeshletQueue[hi_zPass][i];
						shared_ptr<Mesh12> pParentMesh = pMeshlet->GetParentMesh();
						
						UINT cbufferIdx = i - startMeshletIdx;
						matrices[cbufferIdx] = vpMat;
						{
							{
								shared_ptr<AABB> aabb = pMeshlet->GetAABB();
								XMFLOAT3* pVertices = aabb->GetBound();
								for (int j = 0; j < _CUBE_VERTICES; ++j)
								{
									int idx = cbufferIdx * _CUBE_VERTICES + j;
									vertices[idx] =
										XMVectorSet(pVertices[j].x, pVertices[j].y, pVertices[j].z, 1.0f);
								}
							}
						}
					}

					m_hiZCull->UploadAABB(&vertices[0], threadIndex, nMeshes);
					m_hiZCull->UploadMatrix(&matrices[0], threadIndex, nMeshes);
				
					m_psoManager->SetHiZCull(pCmdList);
					g_pHeapManager->SetDescriptorHeaps(pCmdList);
					//밉레벨 지정 필요
					g_pSamplerManager->UploadBorderSampler(pCmdList, E_TO_UINT(eHiZCullRP::csBorder), true);
					m_hiZCull->UploadCullTexture(pCmdList, threadIndex);
					m_hiZCull->UploadCullData(pCmdList, threadIndex);
					pCmdList->Dispatch(threadGroupXCnt, 1, 1);


					DWORD result = WaitForSingleObject(m_copyMutex, INFINITE);
					{
						m_hiZCull->SetBeginReadBarriers(threadIndex);
						g_pResourceBarrierHandler->Flush(pCmdList);

						m_hiZCull->ReadBackResult(pCmdList, threadIndex);
						//TODO:READ기다린다고 루프가 멈춰선 안됌
						m_hiZCull->SetEndReadBarriers(threadIndex);
						g_pResourceBarrierHandler->Flush(pCmdList);
					}
					ReleaseMutex(m_copyMutex);
					g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
					g_pGraphicsCmdListPool->Push(pCmdListWrapper);

					UINT* pHiZCullReadCbBegin = m_hiZCull->GetRbRawPtr(threadIndex);

					//Refresh queue
					for (int j = 0; j < nMeshes; ++j)
					{
						//4096으로 했던이유가 최소 256바이트로 cb정렬이되어서 였음. 정렬감안
						int y= j / threadGroupXCnt;
						int x = j % threadGroupXCnt;
						int offset = y * 64 + x;
						UINT result = *(pHiZCullReadCbBegin + offset);
						shared_ptr<Meshlet> pMeshlet = m_pMeshletQueue[hi_zPass][startMeshletIdx + j];
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
	
		UINT pass = E_TO_UINT(ePass::HWOcclusionPass);
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

				shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
				ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();
				UINT nQuery = 0;

				ComPtr<ID3D12QueryHeap> pQueryHeap = g_pHeapManager->GetQueryHeap(threadIndex);
				for (UINT i = startMeshletIdx; i < lastMeshletIdx; ++i)
				{
					shared_ptr<Mesh12> pParentMesh = m_pMeshletQueue[pass][i]->GetParentMesh();
					int meshletIdx = m_pMeshletQueue[pass][i]->GetMeshletIdx();

					m_psoManager->SetHWOcclusion(pCmdList);
					g_pHeapManager->SetDescriptorHeaps(pCmdList);
					pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_backBuffer->SetHWCullPass(pCmdList);

					pParentMesh->UploadCompositeMatrix(pCmdList);
					pParentMesh->UploadVB(pCmdList);
					pParentMesh->UploadIB(pCmdList,meshletIdx);
					pCmdList->BeginQuery(pQueryHeap.Get(), D3D12_QUERY_TYPE_OCCLUSION, nQuery);
					pCmdList->DrawIndexedInstanced(pParentMesh->GetIndexCount(meshletIdx), 1,
						0, 0, 0);
					pCmdList->EndQuery(pQueryHeap.Get(), D3D12_QUERY_TYPE_OCCLUSION, nQuery++);
				}

				ComPtr<ID3D12Resource>	pQueryResultBlock = g_pHeapManager->GetQueryResultBlock(threadIndex);
				UINT64					queryResultOffset =  g_pHeapManager->GetQueryResultOffset(threadIndex);

				pCmdList->ResolveQueryData(pQueryHeap.Get(), D3D12_QUERY_TYPE_OCCLUSION, 0, nQuery,
					pQueryResultBlock.Get(), 0);

				{
					DWORD result = WaitForSingleObject(m_copyMutex, INFINITE);
					g_pResourceBarrierHandler->Push(
						ResourceBarrierHandler::CreateResourceBarrier(pQueryResultBlock, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE));
					g_pResourceBarrierHandler->Flush(pCmdList);

					ReleaseMutex(m_copyMutex);
				}
				

				pCmdList->CopyBufferRegion(g_pHeapManager->GetRbBlock(m_queryRbKey[threadIndex]).Get(), 0, 
					pQueryResultBlock.Get(), 0,
					sizeof(UINT64) * nQuery);
				

				{
					DWORD result = WaitForSingleObject(m_copyMutex, INFINITE);

					g_pResourceBarrierHandler->Push(
						ResourceBarrierHandler::CreateResourceBarrier(pQueryResultBlock, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
					g_pResourceBarrierHandler->Flush(pCmdList);

					ReleaseMutex(m_copyMutex);
				}
			


				g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
				g_pGraphicsCmdListPool->Push(pCmdListWrapper);

				UINT8* pResult = g_pHeapManager->GetRBRawPtr(m_queryRbKey[threadIndex]);
				for (int i = 0; i < nMeshes; ++i)
				{
					UINT64 result = *(reinterpret_cast<UINT64*>(pResult) + i);

					if (result)
					{
						m_pHWOcclusionQueue[threadIndex].push_back(m_pMeshletQueue[pass][i+startMeshletIdx]);
					}
				}

			}
		}
		
	}
	void D3D12::SetPSO(ComPtr<ID3D12GraphicsCommandList> pCmdList, ePass curPass , shared_ptr<Mesh12> pParentMesh)
	{
		//Set PSO
		{

			switch (curPass)
			{
				case wilson::ePass::zPass:
				{
					m_psoManager->SetZpass(pCmdList);
				}
				break;
				case wilson::ePass::cascadeDirShadowPass:
				{
					m_psoManager->SetCascadeDirShadow(pCmdList);
				}
				break;
				case wilson::ePass::spotShadowPass:
				{
					m_psoManager->SetSpotShadow(pCmdList);
				}
				break;
				case wilson::ePass::cubeShadowPass:
				{
					m_psoManager->SetCubeShadow(pCmdList);
				}
				break;
				case wilson::ePass::geoPass:
				{

					PerModel perModel = *(pParentMesh->GetPerModel(0));
					if (curPass == ePass::geoPass)
					{
						if (perModel.hasNormal)
						{
							if (*pParentMesh->GetHeightOnOff())
							{
								if (perModel.hasEmissive)
								{
									m_psoManager->SetPbrDeferredGeoNormalHeightEmissive(pCmdList);
								}
								else
								{
									m_psoManager->SetPbrDeferredGeoNormalHeight(pCmdList);
								}
							}
							else
							{
								if (perModel.hasEmissive)
								{
									m_psoManager->SetPbrDeferredGeoNormalEmissive(pCmdList);
								}
								else
								{
									m_psoManager->SetPbrDeferredGeoNormal(pCmdList);
								}

							}
						}
						else
						{
							if (perModel.hasEmissive)
							{
								m_psoManager->SetPbrDeferredGeoEmissive(pCmdList);
							}
							else
							{
								m_psoManager->SetPbrDeferredGeo(pCmdList);
							}

						}
					}

				}
				break;
				default:
					break;
			}
		}
	}
	void D3D12::SetShaderResource(ComPtr<ID3D12GraphicsCommandList> pCmdList, ePass curPass, shared_ptr<Meshlet> pMeshlet, const UINT lightIdx)
	{
		
		shared_ptr<Mesh12>		pParentMesh = pMeshlet->GetParentMesh();
		shared_ptr<WVPMatrices>	pWVPMatrics = pParentMesh->GetWVPMatrices();

		int meshletIdx = pMeshlet->GetMeshletIdx();
		pParentMesh->UploadVB(pCmdList);
		if (m_pCam->GetDirtyBit())
		{
			pWVPMatrics->vMat = *(m_pCam->GetViewMatrix());
			pWVPMatrics->pMat = *(m_pCam->GetProjectionMatrix());
			pParentMesh->SetMatrixDirtyBit(true);
		}
		if (pParentMesh->GetMatrixDirtyBit())
		{
			pParentMesh->UpdateCompositeMatrix();
			pParentMesh->SetMatrixDirtyBit(false);
		}


		switch (curPass)
		{
		case wilson::ePass::zPass:
			pParentMesh->UploadCompositeMatrix(pCmdList);
			break;
		case wilson::ePass::spotShadowPass:
			pParentMesh->UploadGeoPassMatrices(pCmdList);
			break;
		case wilson::ePass::cascadeDirShadowPass:
			pParentMesh->UploadGeoPassMatrices(pCmdList);
			break;
		case wilson::ePass::cubeShadowPass:
			pParentMesh->UploadGeoPassMatrices(pCmdList);
			break;
		case wilson::ePass::geoPass:
			pParentMesh->UpdateParllexMappingParemeters();
			pParentMesh->UploadGeoPassMatrices(pCmdList);
			break;
		default:
			break;
		}

		if (curPass == ePass::geoPass || curPass == ePass::zPass)
		{
			pParentMesh->UploadTextures(pCmdList, meshletIdx, curPass);
		}
		else
		{
			pParentMesh->UploadTextures(pCmdList, 0, curPass);
		}
		
	}
	void D3D12::UpdateScene()
	{
		HRESULT hr;
		
		shared_ptr<CommandListWrapper>	pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList>	pCmdList = pCmdListWrapper->GetCmdList();

		PIXBeginEvent(pCmdList.Get(), 0, L"Init Textures and Set Barriers");

		m_psoManager->SetZpass(pCmdList);
		g_pHeapManager->SetDescriptorHeaps(pCmdList);

		//SetInitBarrier
		m_backBuffer			->SetBeginSceneBarriers();
		m_ssaoResourceManager	->SetBeginSceneBarriers();
		g_pResourceBarrierHandler->Flush(pCmdList);

		m_backBuffer->ClearBackBufferView(pCmdList);
		PIXEndEvent(pCmdList.Get());

		PIXBeginEvent(pCmdList.Get(), 0, L"Init Shadow Textures and Set Barriers");
		shared_ptr<LightBuffer12> pLightCb = m_curScene->GetLightCb();
		shared_ptr<ShadowMap12> pShadowMap = m_curScene->GetShadowMap();
		UINT nLights[] = { pLightCb->GetDirLightsSize(), pLightCb->GetCubeLightsSize(), pLightCb->GetSpotLightsSize() };
		UINT nLightsTotal = nLights[E_TO_UINT(eLIGHT_TYPE::DIR)]
			+ nLights[E_TO_UINT(eLIGHT_TYPE::CUBE)]
			+ nLights[E_TO_UINT(eLIGHT_TYPE::SPT)];
		//Clear ShadowMap	
		if (nLightsTotal)
		{
			pShadowMap->SetClearViewBarrier(pCmdList, nLights);
			g_pResourceBarrierHandler->Flush(pCmdList);

			pShadowMap->ClearRtv(pCmdList, nLights);
			pShadowMap->ClearDsv(pCmdList, nLights);

			
		}
		PIXEndEvent(pCmdList.Get());


		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);
		

		HANDLE m_handles[_WORKER_THREAD_COUNT];
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginFrame[i]);
			m_handles[i] = m_workerFinishZpass[i];
		}


		m_nDrawCall = 0;
		UINT nfrustumCull=0;
		UINT nPassed = 0;
		
		shared_ptr<Frustum12>	pFrustum		= m_pCam->GetFrustum();
		XMVECTOR*				pPlanes			= pFrustum->GetPlanes();
		UINT					totalMeshlet	= 0;
		
		unordered_set<shared_ptr<Mesh12>>& pTotalMeshes = m_curScene->GetTotalMeshes();
		for (auto it = pTotalMeshes.begin(); it!=pTotalMeshes.end(); ++it)
		{
			vector<shared_ptr<Meshlet>>& pMeshlets =(*it)->GetMeshelets();
			UINT nMeshlet = pMeshlets.size();
			totalMeshlet += nMeshlet;
		}

		m_pMeshletQueue[E_TO_UINT(ePass::zPass)].reserve(totalMeshlet);
		m_pMeshletQueue[E_TO_UINT(ePass::hi_zPass)].reserve(totalMeshlet);
		m_pMeshletQueue[E_TO_UINT(ePass::spotShadowPass)].reserve(totalMeshlet);

		for (auto it = pTotalMeshes.begin(); it != pTotalMeshes.end(); ++it)
		{
			vector<shared_ptr<Meshlet>>& pMeshlets = (*it)->GetMeshelets();
			
			for (int j = 0; j < pMeshlets.size(); ++j)
			{
				shared_ptr<Meshlet> pMeshlet = pMeshlets[j];
				shared_ptr<AABB> aabb = pMeshlet->GetAABB();

				bool result = aabb->IsOnFrustum(pPlanes);
				if (result)
				{
					m_pMeshletQueue[E_TO_UINT(ePass::zPass)].push_back(pMeshlet);
					m_pMeshletQueue[E_TO_UINT(ePass::hi_zPass)].push_back(pMeshlet);

					if (m_pMeshletQueue[E_TO_UINT(ePass::zPass)].size() % _OBJECT_PER_THREAD == 0)
					{
						SetEvent(m_dataReadyEvent);
						//워커스레드로 신호
						WaitForSingleObject(m_dataEmptyEvent, INFINITE);

					}
				}
			}
			if (m_pMeshletQueue[E_TO_UINT(ePass::zPass)].size() % _OBJECT_PER_THREAD)
			{
				SetEvent(m_dataReadyEvent);
				WaitForSingleObject(m_dataEmptyEvent, INFINITE);
			}		
		}

		pFrustum->SetNumOfMeshletInScene(totalMeshlet);
		pFrustum->SetNumOfMeshletInFrustum(m_pMeshletQueue[E_TO_UINT(ePass::zPass)].size());
		

		//Zpass
		SetEvent(m_drawFinishedEvent);
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);
		ResetEvent(m_drawFinishedEvent);

		m_pMeshletQueue[E_TO_UINT(ePass::zPass)].clear();
		m_lastMeshletIdx = 0;
		//ShadowPass 
		
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginShadowPass[i]);
			m_handles[i] = m_workerFinishShadowPass[i];

		}

		if (nLightsTotal)
		{
			for (auto it = pTotalMeshes.begin(); it != pTotalMeshes.end(); ++it)
			{
				vector<shared_ptr<Meshlet>>& pMeshlets = (*it)->GetMeshelets();
			
				for (int j = 0; j < pMeshlets.size(); ++j)
				{
					shared_ptr<Meshlet> pMeshlet = pMeshlets[j];
					shared_ptr<AABB> aabb = pMeshlet->GetAABB();

					bool result = aabb->IsOnFrustum(pPlanes);
					if (result)
					{
						m_pMeshletQueue[E_TO_UINT(ePass::spotShadowPass)].push_back(pMeshlet);

						if (m_pMeshletQueue[E_TO_UINT(ePass::spotShadowPass)].size() % _OBJECT_PER_THREAD == 0)
						{
							SetEvent(m_dataReadyEvent);
							//워커스레드로 신호
							WaitForSingleObject(m_dataEmptyEvent, INFINITE);

						}
					}
				}
				if (m_pMeshletQueue[E_TO_UINT(ePass::spotShadowPass)].size() % _OBJECT_PER_THREAD)
				{
					SetEvent(m_dataReadyEvent);
					WaitForSingleObject(m_dataEmptyEvent, INFINITE);
				}
			}
		}
		
		SetEvent(m_drawFinishedEvent);
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);
		ResetEvent(m_drawFinishedEvent);
		m_pMeshletQueue[E_TO_UINT(ePass::spotShadowPass)].clear();

		pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		pCmdList = pCmdListWrapper->GetCmdList();
		m_lastMeshletIdx = 0;
	
		if (nLightsTotal)
		{
			pShadowMap->SetLightingPassBarrier(pCmdList, nLights);
			g_pResourceBarrierHandler->Flush(pCmdList);
		}

		//Set-Up HiZPass
		UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(_SHADOWMAP_SIZE, _SHADOWMAP_SIZE/2)));
		{
			PIXBeginEvent(pCmdList.Get(), 0, L"Gen Hi-Z");
			//DownSample DepthMap
			m_backBuffer->SetBeginDownSampleBarriers();
			g_pResourceBarrierHandler->Flush(pCmdList);

			g_pHeapManager	->SetDescriptorHeaps(pCmdList);
			m_psoManager	->SetDownSample(pCmdList);
			g_pSamplerManager->UploadBorderSampler(pCmdList, E_TO_UINT(eDownSampleRP::sampler),true);
			m_backBuffer	->UploadDownSampleTexture(pCmdList);
			m_hiZCull		->UploadDownSampleTexture(pCmdList, E_TO_UINT(eDownSampleRP::dst));
			m_hiZCull		->DrawMip(pCmdList, 0);
			
			m_backBuffer	->SetEndDownSampleBarriers();
			m_hiZCull		->SetGenMipBarriers();
			g_pResourceBarrierHandler->Flush(pCmdList);
			//SetUp
			{
				m_psoManager->SetHiZGen(pCmdList);
				g_pSamplerManager->UploadBorderSampler(pCmdList, E_TO_UINT(eGenHiZRP::sampler), true);
				for (int i = 1; i < mipLevels; ++i)
				{
					m_hiZCull->UploadMips(pCmdList, i);
					m_hiZCull->UploadResolution(pCmdList, i);
					m_hiZCull->DrawMip(pCmdList, i);
					m_hiZCull->SetMipBarries(i);
					g_pResourceBarrierHandler->Flush(pCmdList);
				}

				g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
				g_pGraphicsCmdListPool->Push(pCmdListWrapper);

				pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
				pCmdList = pCmdListWrapper->GetCmdList();

			}
			m_nHiZPassed = 0;
			PIXEndEvent(pCmdList.Get());
		
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginHiZpass[i]);
			m_handles[i] = m_workerFinishHiZpass[i];
		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);

		//Set-Up HWOcclusion Pass
		{
			m_lastMeshletIdx = 0;
			m_hiZCull->SetHWCullBarriers();
			g_pResourceBarrierHandler->Flush(pCmdList);
			
			g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
			g_pGraphicsCmdListPool->Push(pCmdListWrapper);
		
			UINT hwOcclusionPass = E_TO_UINT(ePass::HWOcclusionPass);
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				UINT nHiZPassed= m_pHiZQueue[i].size();
				m_nHiZPassed+= nHiZPassed;
				m_pMeshletQueue[hwOcclusionPass].reserve(nHiZPassed);
				for (int j = 0; j < m_pHiZQueue[i].size(); ++j)
				{
					shared_ptr<Meshlet> pMesh = m_pHiZQueue[i][j];
					m_pMeshletQueue[hwOcclusionPass].push_back(pMesh);
				}
				m_pHiZQueue[i].clear();
			}


			m_pMeshletQueue[E_TO_UINT(ePass::hi_zPass)].clear();
			m_lastMeshletIdx = 0;
		}
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginHWOcclusionTestPass[i]);
			m_handles[i] = m_workerFinishHWOcclusionTestPass[i];

		}
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);

		m_pMeshletQueue[E_TO_UINT(ePass::HWOcclusionPass)].clear();
		m_nNotOccluded  = 0;
		m_lastMeshletIdx = 0;
		//Draw PbrGeo
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			SetEvent(m_workerBeginDeferredGeoPass[i]);
			m_handles[i] = m_workerEndFrame[i];
		}
		
		
		UINT geoPass = E_TO_UINT(ePass::geoPass);
		UINT nDrawed = 0;
		shared_ptr<Mesh12> pParentMesh = nullptr;

		//메모리 재할당으로 인한 잘못된 참조를 방지
		UINT hwCullPassed = 0;
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			UINT nMeshlets = m_pHWOcclusionQueue[i].size();
			hwCullPassed += nMeshlets;
		}
		
		m_pMeshletQueue[geoPass].reserve(hwCullPassed);
		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			UINT nMeshlets = m_pHWOcclusionQueue[i].size();
			m_nNotOccluded += nMeshlets;
			for (int j = 0; j < m_pHWOcclusionQueue[i].size(); ++j)
			{
				shared_ptr<Meshlet> pMeshlet = m_pHWOcclusionQueue[i][j];
				if (pParentMesh!=pMeshlet->GetParentMesh())
				{
					SetEvent(m_dataReadyEvent);
					WaitForSingleObject(m_dataEmptyEvent, INFINITE);

					pParentMesh = pMeshlet->GetParentMesh();

					nDrawed = m_pMeshletQueue[geoPass].size();
				}
				else if (!m_pMeshletQueue[geoPass].empty()&&
					m_pMeshletQueue[geoPass].size() % _OBJECT_PER_THREAD == 0)
				{
					nDrawed = m_pMeshletQueue[geoPass].size();
					SetEvent(m_dataReadyEvent);
					WaitForSingleObject(m_dataEmptyEvent, INFINITE);
				}
				m_pMeshletQueue[geoPass].push_back(pMeshlet);
			}
			m_pHWOcclusionQueue[i].clear();

		}
		if (m_pMeshletQueue[geoPass].size()!=nDrawed)
		{
			SetEvent(m_dataReadyEvent);
			WaitForSingleObject(m_dataEmptyEvent, INFINITE);
		}

		
		SetEvent(m_drawFinishedEvent);
		WaitForMultipleObjects(_WORKER_THREAD_COUNT, m_handles, TRUE, INFINITE);
		ResetEvent(m_drawFinishedEvent);

		m_pMeshletQueue[E_TO_UINT(ePass::geoPass)].clear();
		m_lastMeshletIdx = 0;

		pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		pCmdList = pCmdListWrapper->GetCmdList();

		m_backBuffer				->SetGeoPassBarriers();
		g_pResourceBarrierHandler	->Flush(pCmdList);
		g_pGraphicsCmdListPool		->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool		->Push(pCmdListWrapper);


		pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		pCmdList = pCmdListWrapper->GetCmdList();


		if (m_pCam->GetDirtyBit())
		{
			m_wvpMatrices->vMat = *(m_pCam->GetViewMatrix());
			m_wvpMatrices->pMat = *(m_pCam->GetProjectionMatrix());
			m_pbrResourceManager->UpdateSkyBoxMat(m_wvpMatrices);
			m_ssaoResourceManager->UpdateProjMat(m_pCam->GetProjectionMatrixTransposed());
		}

		SetEvent(m_ssaoBeginFrame);
		WaitForSingleObject(m_ssaoEndFrame, INFINITE);
		
		g_pHeapManager				->SetDescriptorHeaps(pCmdList);
		m_backBuffer				->SetLightPassBarriers();
		m_ssaoResourceManager		->SetLightPassBarriers();
		g_pResourceBarrierHandler	->Flush(pCmdList);

		//Upload HeightScale and bHeightOnOff
		
		{	
		
			//LightingPass
			PIXBeginEvent(pCmdList.Get(), 0, L"PbrLighting Pass");
			m_psoManager			->SetPbrDeferredLighting(pCmdList);
			g_pHeapManager			->SetDescriptorHeaps(pCmdList);
			g_pSamplerManager		->UploadClampPointSampler(pCmdList, E_TO_UINT(ePbrLightRP::psClamp), false);
			g_pSamplerManager		->UploadBorderLinearPoint(pCmdList, E_TO_UINT(ePbrLightRP::psCubeShadowSampler), false);
			g_pSamplerManager		->UploadBorderLinearPointLess(pCmdList, E_TO_UINT(ePbrLightRP::psShadowSampler), false);


			m_ssaoResourceManager	->SetLightingPass(pCmdList);
			m_pbrResourceManager	->SetLightingPass(pCmdList);
			m_backBuffer			->SetLightingPass(pCmdList);
			m_pCam					->SetLightingPass(pCmdList);
			pShadowMap				->SetLightingPass(pCmdList);
			pLightCb				->SetLightingPass(pCmdList);

			pCmdList->IASetVertexBuffers(0, 1, &m_quadVbv);
			pCmdList->IASetIndexBuffer(&m_quadIbv);
			pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			pCmdList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);
			PIXEndEvent(pCmdList.Get());

		}
		

		//Draw SkyBox
		{
			PIXBeginEvent(pCmdList.Get(), 0, L"DrawSkyBox");
			m_psoManager		->SetSkyBox(pCmdList);
			g_pHeapManager		->SetDescriptorHeaps(pCmdList);
			g_pSamplerManager	->UploadWrapSampler(pCmdList, E_TO_UINT(eSkyboxRP::psSampler), false);
			m_pbrResourceManager->SetSkyBoxPass(pCmdList);
			m_backBuffer		->SetSkyboxPass(pCmdList);
			pCmdList			->DrawIndexedInstanced(_CUBE_IDX_COUNT, 1, 0, 0, 0);
			PIXEndEvent(pCmdList.Get());
		}
		m_pCam->SetDirty(false);
		
		m_backBuffer->SetPostProcessBarriers();
		g_pResourceBarrierHandler->Flush(pCmdList);
		
		//PostProcess
		{
			PIXBeginEvent(pCmdList.Get(), 0, L"PostProcess");

			m_psoManager		->SetPostProcess(pCmdList);
			g_pHeapManager		->SetDescriptorHeaps(pCmdList);
			g_pSamplerManager	->UploadWrapSampler(pCmdList, E_TO_UINT(ePostProcessRP::csSampler), true);
			m_backBuffer		->UploadPostProcessTexture(pCmdList);
			m_postprocess		->UploadExposureParemeters(pCmdList);
			pCmdList			->Dispatch(ceil(m_clientWidth / static_cast<float>(8)), ceil(m_clientHeight / static_cast<float>(8)), 1);

			PIXEndEvent(pCmdList.Get());
		}
		
		//DrawUI
		
		m_backBuffer->SetUIPassBarriers();
		g_pResourceBarrierHandler->Flush(pCmdList);

		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);
		return;
}
	void D3D12::DrawScene()
	{	
		shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

		g_pHeapManager->SetDescriptorHeaps(pCmdList);
		m_backBuffer->SetScreenRenderTargets(pCmdList);

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),pCmdList.Get());

		m_backBuffer				->SetPresentBarriers();
		g_pResourceBarrierHandler	->Flush(pCmdList);
		
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, reinterpret_cast<void*>(pCmdList.Get()));
		}


		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);

		if (m_bVsyncOn)
		{
			m_pSwapChain->Present(1, 0);
		}
		else
		{
			m_pSwapChain->Present(0, 0);
		}
		
		m_backBuffer->IncreaseFrameCount();
	}

	D3D12::D3D12(const UINT screenWidth, const UINT screenHeight, bool bVsync, HWND hWnd, bool bFullscreen,
		float fScreenFar, float fScreenNear) : m_lastMeshletIdx(0), m_clientWidth(screenWidth), m_clientHeight(screenHeight)
	{
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
		

		IDXGIAdapter* pAdapter;
		hr = pFactory->EnumAdapters(0, &pAdapter);
		assert(SUCCEEDED(hr));
		
		DXGI_ADAPTER_DESC adapterDesc = {};
		hr = pAdapter->GetDesc(&adapterDesc);
		assert(SUCCEEDED(hr));

		IDXGIOutput* pAdapterOutput;
		hr = pAdapter->EnumOutputs(0, &pAdapterOutput);
		assert(SUCCEEDED(hr));
		
		UINT numModes = {};
		hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
		assert(SUCCEEDED(hr));

		DXGI_MODE_DESC* pDisplayModeList = new DXGI_MODE_DESC[numModes];
		assert(SUCCEEDED(hr));

		hr = pAdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, pDisplayModeList);
		assert(SUCCEEDED(hr));

		hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_pDevice.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pDevice);
#ifdef _DEBUG
		hr = m_pDevice->QueryInterface(IID_PPV_ARGS(&m_pInfoQueue));
		assert(SUCCEEDED(hr));
		m_pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		m_pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		m_pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
#endif
		g_pHeapManager = make_shared<HeapManager>(m_pDevice);
		//Create CmdListPool
		g_pGraphicsCmdListPool = make_shared<CommandListWrapperPool>(D3D12_COMMAND_LIST_TYPE_DIRECT);
		SET_PRIVATE_DATA_WITH_NAME("GraphicsQueue",g_pGraphicsCmdListPool->GetCmdQueue());
		g_pComputeCmdListPool = make_shared<CommandListWrapperPool>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		SET_PRIVATE_DATA_WITH_NAME("ComputeQueue", g_pComputeCmdListPool->GetCmdQueue());

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

		hr = pFactory->CreateSwapChain(g_pGraphicsCmdListPool->GetCmdQueue().Get(), &swapChainDesc, m_pSwapChain.GetAddressOf());
		assert(SUCCEEDED(hr));

		SET_PRIVATE_DATA(m_pSwapChain);
		m_backBuffer = make_shared<BackBuffer>(m_pSwapChain, screenWidth, screenHeight);

		delete[] pDisplayModeList;
		pDisplayModeList = nullptr;

		pAdapterOutput->Release();
		pAdapterOutput = nullptr;

		pAdapter->Release();
		pAdapter = nullptr;

		pFactory->Release();
		pFactory = nullptr;


		ImGui_ImplDX12_Init(m_pDevice.Get(), _BUFFER_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM, *(g_pHeapManager->GetCbvSrvUavHeap()),
			g_pHeapManager->GetCurCbvSrvCpuHandle(),
			g_pHeapManager->GetCurCbvSrvGpuHandle());
		g_pHeapManager->IncreaseCbvSrvHandleOffset();
		g_pTextureManager = make_shared<TextureManager>();
		g_pSamplerManager = make_shared<SamplerManager>();
		g_pResourceBarrierHandler = make_shared<ResourceBarrierHandler>();

		//Gen User-Defined Class
		shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();
		{
			m_psoManager			= make_shared<PSOManager>();
			m_pCam					= make_shared<Camera12>(screenWidth, screenHeight, fScreenFar, fScreenNear);
			m_wvpMatrices			= make_shared<WVPMatrices>();
			m_compositeMatrices		= make_shared<CompositeMatrices>();

			m_ssaoResourceManager	= make_shared<SSAOResourceManager>(screenWidth, screenHeight, pCmdList);
			m_postprocess			= make_shared<PostProcess>();
			m_pbrResourceManager	= make_shared<PBRResourceManager>(m_psoManager);
			m_hiZCull				= make_shared<HierarchicalZCull>();
		}
		

		{
			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				m_queryRbKey[i] = g_pHeapManager->AllocateRb(_64KB);
			}
		}
	
		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);
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
			g_pHeapManager->AllocateVertexData(reinterpret_cast<UINT8*>(quads), quadsVbSize);
			m_quadVbv = g_pHeapManager->GetVbv(quadsVbSize, sizeof(QUAD));

			unsigned long indices[6] = { 0,1,2, 2,3,0 };
			UINT quadsIbSize = sizeof(indices);
			g_pHeapManager->AllocateIndexData(reinterpret_cast<UINT8*>(indices), quadsIbSize);
			m_quadIbv = g_pHeapManager->GetIbv(quadsIbSize, 0);
			UINT idx = g_pHeapManager->GetIbHeapOffset();
			idx /= _IB_HEAP_SIZE;
			UINT64 curBlockOffset = g_pHeapManager->GetIbBlockOffset(idx);
			g_pHeapManager->SetIbBlockOffset(idx, curBlockOffset + m_quadIbv.SizeInBytes);
		}

		//Create Events
		{
			m_dataReadyEvent = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);
			m_dataEmptyEvent = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_drawFinishedEvent = CreateEvent(
				NULL,
				TRUE,
				FALSE,
				NULL);

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

			m_ssaoBeginShutDown = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_ssaoEndShutDown = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);
		}

		for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
		{
			m_workerBeginShutdown[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

			m_workerEndShutdown[i] = CreateEvent(
				NULL,
				FALSE,
				FALSE,
				NULL);

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
	
		}
		
		m_workerMutex = CreateMutex(NULL, FALSE, NULL);
		m_copyMutex = CreateMutex(NULL, FALSE, NULL);
	}
	D3D12::~D3D12()
	{	
		ImGui_ImplDX12_Shutdown();
		g_pGraphicsCmdListPool->WaitForGpu();
		g_pComputeCmdListPool->WaitForGpu();

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

		}
		g_pHeapManager = nullptr;
		g_pComputeCmdListPool = nullptr;
		g_pGraphicsCmdListPool = nullptr;
		m_pDevice = nullptr;
	}
}

