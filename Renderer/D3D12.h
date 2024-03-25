#pragma once
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DirectXTex.lib")

#include <dxgi1_4.h>
#include <d3dcommon.h>
#include <d3d12.h>
#include <DirectXMath.h>

#include <Windows.h>
#include <vector>
#include <queue>

#include "Camera12.h"
#include "Frustum12.h"
#include "MatrixBuffer12.h"
#include "LightBuffer12.h"
#include "Shader12.h"
#include "Terrain.h"
#include "Import12.h"
#include "ShadowMap12.h"
#include "DescriptorHeapManager.h"
#include "typedef.h"

namespace wilson 
{   
	
	class D3D12 
	{
	private:
		void WaitForGpu();
		bool CreateRTVandSRV(UINT, UINT);
		bool CreateDSV(UINT, UINT);
		bool CreateEquirentangularMap(const char* pPath);
		void ConvertEquirectagular2Cube();
		void CreateDiffuseIrradianceMap();
		void CreatePrefileterMap();
		void CreateMipMap();
		void CreateBRDFMap();
		void DestroyTexture();
		void DestroySceneDepthTex();
		void DestroyHDR();
		void DestroyBackBuffer();
		void UpdateTotalModels();
		void DrawObject(ePass curPass, UINT threadIndex, UINT lightIdx);
		void HWQueryForOcclusion(UINT threadIdx);
		void D3DMemoryLeakCheck();
		void SSAOThread();
		void WorkerThread(UINT threadIndex);
		static UINT WINAPI WrapperWorkerThreadFun(LPVOID pParameter);
		static UINT WINAPI WrapperSSAOThreadFun(LPVOID pParameter);
	public:
		static D3D12_RESOURCE_BARRIER CreateResourceBarrier(ID3D12Resource* pResource,
			D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
		static D3D12_RESOURCE_BARRIER CreateResourceBarrier(ID3D12Resource* pResource,
			D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, UINT mipLevel);
		void ExecuteCommandLists(ID3D12GraphicsCommandList** ppCmdLists, UINT cnt);
		void ResizeBackBuffer(int, int);
		inline ID3D12Device* GetDevice() const
		{
			return m_pDevice;
		};
		inline ID3D12GraphicsCommandList* GetCommandList() const
		{
			return m_pMainCommandList;
		};
		inline DescriptorHeapManager* GetDescriptorHeapManager()
		{
			return m_pDescriptorHeapManager;
		}
		inline ShadowMap12* GetShadowMap() const
		{
			return m_pShadowMap;
		}
		inline Camera12* GetCam() const
		{
			return m_pCam;
		};
		inline Frustum12* GetFrustum() const
		{
			return m_pFrustum;
		};
		inline void SetNewFrustum(Frustum12* pFrustum)
		{
			m_pFrustum = pFrustum;
		}
		inline float* GetExposure()
		{
			return &m_exposure;
		}
		inline float* GetHeightScale()
		{
			return &m_heightScale;
		}
		inline BOOL* GetHeighOnOFF()
		{
			return &m_bHeightOnOff;
		}
		inline int GetClientWidth() const
		{
			return m_clientWidth;
		};
		inline int GetClientHeight() const
		{
			return m_clientHeight;
		};
		inline void PickModel(int i, int j)
		{
			m_selectedModelGroup = i;
			m_selectedModel = j;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetFinalSRV() 
		{
			return &m_ViewportSRV;
		};
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetSSAOBlurredSRV() 
		{
			return &m_SSAOBlurDebugSRV;
		};
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetDepthSRV()
		{
			return &m_SceneDepthSRV;
		};
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetGbufferSRV()
		{
			return m_GBufSRV;
		};
		inline UINT GetGbufferCount() const
		{
			return eGbuf_cnt;
		}
		inline UINT GetModelGroupSize() const
		{
			return m_pModelGroups.size();
		}
		inline UINT GetModelsHiZPassed() const
		{
			return m_numOfModelsHiZPassed;
		}
		inline UINT GetModelsNotOccluded() const
		{
			return m_numOfModelsNotOccluded;
		}

		void AddModelGroup(ModelGroup12*);
		void RemoveModelGroup(int i);
		void RemoveModel(int i, int j);
		UINT GetModelSize(int i);
		UINT GetLightSize(eLIGHT_TYPE);
		void AddLight(Light12* pLight);
		void RemoveLight(int i, Light12* pLight);
		void UpdateScene();
		void GenUploadBuffer(ID3D12Resource** ppUploadCB, const UINT64 uploadPitch, const UINT64 uploadSize);
		void UploadTexThroughCB(D3D12_RESOURCE_DESC texDesc, const UINT rowPitch,
			const UINT8* pData, ID3D12Resource* pDst, ID3D12Resource** ppUploadCB, ID3D12GraphicsCommandList* pCommandList);
		void DrawScene();
		D3D12() = default;
		D3D12(int, int, bool, HWND, bool, float, float);
		D3D12(const D3D12&) = delete;
		~D3D12();
	private:
		HANDLE m_ssaoThreadHandle;
		HANDLE m_ssaoBeginFrame;
		HANDLE m_ssaoEndFrame;
		HANDLE m_threadHandles[_WORKER_THREAD_COUNT];
		HANDLE m_workerBeginFrame[_WORKER_THREAD_COUNT];
		HANDLE m_workerFinishZpass[_WORKER_THREAD_COUNT];
		HANDLE m_workerBeginHWOcclusionTestPass[_WORKER_THREAD_COUNT];
		HANDLE m_workerFinishHWOcclusionTestPass[_WORKER_THREAD_COUNT];
		HANDLE m_workerBeginShadowPass[_WORKER_THREAD_COUNT];
		HANDLE m_workerFinishShadowPass[_WORKER_THREAD_COUNT];
		HANDLE m_workerBeginDeferredGeoPass[_WORKER_THREAD_COUNT];
		HANDLE m_workerEndFrame[_WORKER_THREAD_COUNT];
		HANDLE m_workerMutex;
		UINT m_workerThreadIdx[_WORKER_THREAD_COUNT];

		UINT m_clientWidth; 
		UINT m_clientHeight;
		bool m_bVsyncOn;

		ID3D12Debug* m_pDebugController;
		ID3D12Device1* m_pDevice;
		ID3D12CommandQueue* m_pMainCommandQueue;
		ID3D12CommandQueue* m_pComputeCommandQueue;
		IDXGISwapChain* m_pSwapChain;
		ID3D12Resource* m_pScreenTex[_BUFFER_COUNT];
		ID3D12QueryHeap* m_pQueryHeap[_WORKER_THREAD_COUNT];
		ID3D12Resource* m_pQueryResultBuffer[_WORKER_THREAD_COUNT];
		ID3D12Resource* m_pQueryReadBuffer[_WORKER_THREAD_COUNT];
	
		ID3D12GraphicsCommandList* m_pMainCommandList;
		ID3D12GraphicsCommandList* m_pSSAOCommandList;
		ID3D12GraphicsCommandList* m_pPbrSetupCommandList;
		ID3D12GraphicsCommandList* m_pCopyCommandList;
		ID3D12GraphicsCommandList* m_pImporterCommandList;
		ID3D12GraphicsCommandList* m_pWorkerCommandList[_WORKER_THREAD_COUNT];
		ID3D12CommandAllocator* m_pWorkerCommandAllocator[_WORKER_THREAD_COUNT];
		ID3D12CommandAllocator* m_pMainCommandAllocator;
		ID3D12CommandAllocator* m_pSSAOCommandAllocator;
		ID3D12CommandAllocator* m_pPbrSetupCommandAllocator;
		ID3D12CommandAllocator* m_pImporterCommandAllocator;
		ID3D12CommandAllocator* m_pCopyCommandAllocator;

		ID3D12Fence* m_pFence;
		ID3D12Fence* m_pWorkerFence[_WORKER_THREAD_COUNT];
		ID3D12Fence* m_pSsaoFence;

		ID3D12PipelineState* m_pZpassPso;
		ID3D12PipelineState* m_pGenHiZPassPso;
		ID3D12PipelineState* m_pHiZCullPassPso;
		ID3D12PipelineState* m_pHWOcclusionQueryPso;
		ID3D12PipelineState* m_pCascadeDirShadowPso;
		ID3D12PipelineState* m_pSpotShadowPso;
		ID3D12PipelineState* m_pCubeShadowPso;
		ID3D12PipelineState* m_pSkyBoxPso;
		ID3D12PipelineState* m_pPbrDeferredGeoPso;
		ID3D12PipelineState* m_pPbrDeferredGeoEmissivePso;
		ID3D12PipelineState* m_pPbrDeferredGeoNormalPso;
		ID3D12PipelineState* m_pPbrDeferredGeoNormalHeightPso;
		ID3D12PipelineState* m_pPbrDeferredGeoNormalHeightEmissivePso;
		ID3D12PipelineState* m_pOutlinerSetupPso;
		ID3D12PipelineState* m_pOutlinerTestPso;
		ID3D12PipelineState* m_pSSAOPso;
		ID3D12PipelineState* m_pSSAOBlurPso;
		ID3D12PipelineState* m_pPbrDeferredLightingPso;
		ID3D12PipelineState* m_pPostProcessPso;
		ID3D12PipelineState* m_pBRDFPso;
		ID3D12PipelineState* m_pPrefilterPso;
		ID3D12PipelineState* m_pDiffuseIrradiancePso;
		ID3D12PipelineState* m_pEquirect2CubePso;
		ID3D12PipelineState* m_pGenMipmapPso;

		ID3D12Resource* m_pQuadVB;
		ID3D12Resource* m_pSkyBoxVB;
		ID3D12Resource* m_pQuadIB;
		ID3D12Resource* m_pSkyBoxIB;
		ID3D12Resource* m_pAABBVB;
		ID3D12Resource* m_pAABBIB;
		ID3D12Resource* m_pBoolCB;
		ID3D12Resource* m_pColorCB;
		ID3D12Resource* m_pHiZCullMatrixCb;
		ID3D12Resource* m_pSphereCb;
		ID3D12Resource* m_pRoughnessCB;
		ID3D12Resource* m_pSSAOKernelCB;
		ID3D12Resource* m_pExposureCB;
		ID3D12Resource* m_pHeightScaleCB;
		ID3D12Resource* m_pEquirect2CubeCB;
		ID3D12Resource* m_pMipCB;
		ID3D12Resource* m_pHdrUploadCB;
		ID3D12Resource* m_pNoiseUploadCB;
		ID3D12Resource* m_pResolutionCB;
		ID3D12Resource* m_pNoiseTex;
		ID3D12Resource* m_pHiZCullReadCb;

		ID3D12Resource* m_pViewportTex;
		ID3D12Resource* m_pSceneTex;
		ID3D12Resource* m_pHiZTempTex;
		ID3D12Resource* m_pHiZCullListTex;
		ID3D12Resource* m_pSSAOBlurDebugTex;
		ID3D12Resource* m_pBrightTex;
		ID3D12Resource* m_pPingPongTex[2];
		ID3D12Resource* m_pGBufTex[eGbuf_cnt];
		ID3D12Resource* m_pSSAOTex;
		ID3D12Resource* m_pSSAOBlurTex;
		ID3D12Resource* m_pBRDFTex;
		ID3D12Resource* m_pSkyBoxTex;
		ID3D12Resource* m_pDiffIrradianceTex;
		ID3D12Resource* m_pPrefilterTex;
		ID3D12Resource* m_pUAVTex;
		ID3D12Resource* m_pHDRTex;
		ID3D12Resource* m_pScreenDepthTex;
		ID3D12Resource* m_pSceneDepthTex;

		D3D12_CPU_DESCRIPTOR_HANDLE m_ScreenRTV[_BUFFER_COUNT];
		D3D12_CPU_DESCRIPTOR_HANDLE m_SceneRTV;
		D3D12_CPU_DESCRIPTOR_HANDLE m_BrightRTV;
		D3D12_CPU_DESCRIPTOR_HANDLE m_PingPongRTV[2];
		D3D12_CPU_DESCRIPTOR_HANDLE m_GBufRTV[eGbuf_cnt];
		D3D12_CPU_DESCRIPTOR_HANDLE m_BRDFRTV;
		D3D12_CPU_DESCRIPTOR_HANDLE m_SkyBoxRTV;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DiffIrradianceRTV;
		D3D12_CPU_DESCRIPTOR_HANDLE m_PrefilterRTV;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_hiZTempRtvs;

		D3D12_CPU_DESCRIPTOR_HANDLE m_ScreenDSV;
		D3D12_CPU_DESCRIPTOR_HANDLE m_SceneDSV;
		
		
		D3D12_GPU_DESCRIPTOR_HANDLE m_ViewportSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ViewportUAV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SceneSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SceneDepthSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_BrightSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_PingPongSRV[2];
		D3D12_GPU_DESCRIPTOR_HANDLE m_GBufSRV[eGbuf_cnt];
		D3D12_GPU_DESCRIPTOR_HANDLE m_NoiseSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SSAOSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SSAOUAV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SSAOBlurSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SSAOBlurUAV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SSAOBlurDebugSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SSAOBlurDebugUAV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_BRDFSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SkyBoxSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_DiffIrradianceSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_PrefilterSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_HDRSRV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_HiZCullListUAV;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_hiZTempSrvs;

		D3D12_VERTEX_BUFFER_VIEW m_QuadVBV;
		D3D12_VERTEX_BUFFER_VIEW m_SkyBoxVBV;
		D3D12_VERTEX_BUFFER_VIEW m_AABBVBV;
		D3D12_INDEX_BUFFER_VIEW m_QuadIBV;
		D3D12_INDEX_BUFFER_VIEW m_SkyBoxIBV;
		D3D12_INDEX_BUFFER_VIEW m_AABBIBV;

		D3D12_GPU_DESCRIPTOR_HANDLE m_BoolCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ColorCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SSAOKernelCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ExposureCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_HeightScaleCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_Equirect2CubeCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_MipCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_RoughnessCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ResolutionCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_HiZCullMatrixCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_SphereCBV;

		D3D12_GPU_DESCRIPTOR_HANDLE m_ClampSSV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_WrapSSV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_borderSSV;

		D3D12_VIEWPORT m_viewport;
		D3D12_VIEWPORT m_diffIrradViewport;
		D3D12_VIEWPORT m_prefilterViewport;
		D3D12_RECT m_scissorRect;
		D3D12_RECT m_diffIrradRect;
		D3D12_RECT m_prefilterRect;

		static D3D12* g_pD3D12;
		std::vector<Model12*>m_pTotalModels;
		std::queue<Model12*>m_pModelQueue;
		std::queue<Model12*>m_pHWOcclusionQueue[_WORKER_THREAD_COUNT];
		std::vector<ModelGroup12*> m_pModelGroups;
		DescriptorHeapManager* m_pDescriptorHeapManager;
		Camera12* m_pCam;
		Frustum12* m_pFrustum;
		LightBuffer12* m_pLightBuffer;
		MatBuffer12* m_pMatBuffer;
		MatBuffer12* m_pOutlinerMatBuffer;
		Shader12* m_pShader;
		ShadowMap12* m_pShadowMap;


		XMMATRIX m_idMat = XMMatrixIdentity();
		XMMATRIX* m_pLitMat;
		BOOL m_bHeightOnOff;
		UINT m_curFrame;
		UINT m_numOfModelsHiZPassed;
		UINT m_numOfModelsNotOccluded;
		float m_exposure;
		float m_heightScale;

		UINT8* m_pHeightScaleCbBegin;
		UINT8* m_pQueryReadCbBegin[_WORKER_THREAD_COUNT];
		UINT8* m_pExposureCbBegin;
		UINT8* m_pHiZCullMatrixCbBegin;
		UINT8* m_pSphereCbBegin;
		UINT8* m_pResolutionCbBegin;
		UINT8* m_pHiZCullReadCbBegin;


		UINT m_selectedModelGroup;
		UINT m_selectedModel;
		
		UINT m_fenceValue;
		UINT m_SsaoFenceValue;
		UINT m_workerFenceValue[_WORKER_THREAD_COUNT];
		HANDLE m_fenceEvent;
		HANDLE m_workerFenceEvent[_WORKER_THREAD_COUNT];
		HANDLE m_SsaoFenceEvent;

	};
}