#pragma once
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DirectXTex.lib")

#include <dxgi1_4.h>
#include <d3dcommon.h>
#include <d3d12.h>

#include "Camera12.h"
#include "Frustum12.h"
#include "MatrixBuffer12.h"
#include "LightBuffer12.h"
#include "Shader12.h"
#include "Import12.h"
#include "ShadowMap12.h"
#include "HeapManager.h"
#include "typedef.h"
#include "Object12.h"

namespace wilson 
{   
	
	class D3D12 
	{
	private:
		void ConvertEquirectagular2Cube();
		void CreateDiffuseIrradianceMap();
		bool CreateDsv(const UINT, const UINT);
		bool CreateEquirentangularMap(const char* pPath);
		bool CreateRtvSrv(const UINT, const UINT);
		void CreatePrefileterMap();
		void CreateMipMap();
		void CreateBrdfMap();
		void DestroyTexture();
		void DestroySceneDepthTex();
		void DestroyHdr();
		void DestroyBackBuffer();
		void DrawObject(const ePass curPass, const UINT threadIndex, const UINT lightIdx);
		void HWQueryForOcclusion(const UINT threadIdx);
		void PopulateBundle();
		void SsaoThread();
		void WaitForGpu();
		void WorkerThread(const UINT threadIndex);
		static UINT WINAPI WrapperSsaoThreadFun(LPVOID pParameter);
		static UINT WINAPI WrapperWorkerThreadFun(LPVOID pParameter);
	public:
		void AddLight(Light12* const pLight);
		void AddObject(Object12* const);
		static D3D12_RESOURCE_BARRIER CreateResourceBarrier(ID3D12Resource* const pResource,
			const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState);
		static D3D12_RESOURCE_BARRIER CreateResourceBarrier(ID3D12Resource* const pResource,
			const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState, const UINT mipLevel);
		void CreateUploadBuffer(ID3D12Resource** const ppUploadCB, const UINT64 uploadPitch, const UINT64 uploadSize);
		void DrawScene();
		void ExecuteCommandLists(ID3D12GraphicsCommandList** const ppCmdLists, const UINT cnt);
		inline Camera12* GetCam() const
		{
			return m_pCam.get();
		};
		inline int GetClientHeight() const
		{
			return m_clientHeight;
		};
		inline int GetClientWidth() const
		{
			return m_clientWidth;
		};
		inline ID3D12GraphicsCommandList* GetCommandList() const
		{
			return m_pMainCommandList.Get();
		};
		inline ID3D12Device* GetDevice() const
		{
			return m_pDevice.Get();
		};
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetDepthSrv()
		{
			return &m_sceneDepthSrv;
		};
		inline float* GetExposure()
		{
			return &m_exposure;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetpGbufferSrvs()
		{
			return m_GBufSrvs;
		};
		inline HeapManager* GetHeapManager()
		{
			return m_pHeapManager.get();
		}
		inline BOOL* GetpbHeightOn()
		{
			return &m_bHeightOn;
		}
		inline float* GetpHeightScale()
		{
			return &m_heightScale;
		}
		inline UINT GetNumDrawCall() const
		{
			return m_nDrawCall;
		}
		inline UINT GetNumGBuffer() const
		{
			return static_cast<UINT>(eGbuf::cnt);
		}
		UINT GetNumLight(const eLIGHT_TYPE);
		UINT GetNumMesh(const int i);
		inline UINT GetNumObject() const
		{
			return m_pObjects.size();
		}
		inline UINT* GetNumSsaoSample()
		{
			return &m_nSsaoSample;
		}
		inline UINT GetNumSubMeshHiZPassed() const
		{
			return m_nHiZPassed;
		}
		inline UINT GetNumSubMeshNotOccluded() const
		{
			return m_nNotOccluded;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetPostProcessSrv()
		{
			return &m_viewportSrv;
		};
		inline ShadowMap12* GetShadowMap() const
		{
			return m_pShadowMap.get();
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetSsaoBlurredSrv()
		{
			return &m_ssaoBlurDebugSrv;
		};
		inline float* GetSsaoBias()
		{
			return &m_ssaoBias;
		}
		inline float* GetSsaoRadius()
		{
			return &m_ssaoRadius;
		}
		inline void PickSubMesh(const int i, const int j)
		{
			m_selectedObject = i;
			m_selectedMesh = j;
		}
		void ResizeBackBuffer(const UINT, const UINT);
		void RemoveMesh(const int i, const int j);
		void RemoveObject(const int i);
		void RemoveLight(const int, Light12* cont);
		void UpdateDrawLists();
		void UpdateScene();
		void UploadTexThroughCB(const D3D12_RESOURCE_DESC texDesc, const UINT rowPitch,
			const UINT8* pData, ID3D12Resource* const pDst, ID3D12Resource** const ppUploadCB, ID3D12GraphicsCommandList* const pCommandList);

		D3D12() = default;
		D3D12(const UINT, const UINT, const bool, HWND, const bool, const float, const float);
		D3D12(const D3D12&) = delete;
		~D3D12();
	private:
		HANDLE m_fenceEvent;
		HANDLE m_pbrSetupEvent;
		HANDLE m_ssaoBeginFrame;
		HANDLE m_ssaoFenceEvent;
		HANDLE m_ssaoEndFrame;
		HANDLE m_ssaoThreadHandle;
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
		HANDLE m_workerFenceEvents[_WORKER_THREAD_COUNT];


		bool m_bVsyncOn;

		Microsoft::WRL::ComPtr<ID3D12Debug> m_pDebugController;
		Microsoft::WRL::ComPtr<ID3D12Device1> m_pDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>m_pMainCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pComputeCommandQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;
		Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_pQueryHeap[_WORKER_THREAD_COUNT];
	

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pBundles[static_cast<UINT>(ePass::cnt)];
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pCopyCommandList;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pImporterCommandList;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pMainCommandList;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pPbrSetupCommandList;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pSsaoCommandList;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pWorkerCommandLists[_WORKER_THREAD_COUNT];

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pBundleAllocators[static_cast<UINT>(ePass::cnt)];
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCopyCommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pImporterCommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pMainCommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pPbrSetupCommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pSsaoCommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pWorkerCommandAllocators[_WORKER_THREAD_COUNT];

		Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_pPbrSetupFence;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_pSsaoFence;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_pWorkerFences[_WORKER_THREAD_COUNT];

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pAabbPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pBrdfPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pCascadeDirShadowPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pCubeShadowPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pDiffuseIrradiancePso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pDownSamplePso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pEquirect2CubePso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pGenHiZPassPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pGenMipmapPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pHiZCullPassPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pHWOcclusionQueryPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pOutlinerTestPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pOutlinerSetupPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoEmissivePso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoNormalPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoNormalEmissivePso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoNormalHeightPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPbrDeferredGeoNormalHeightEmissivePso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPbrDeferredLightingPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPostProcessPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPrefilterPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pSkyBoxPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pSpotShadowPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pSsaoPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pSsaoBlurPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pZpassPso;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_pBrdfTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pDepthDebugTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pDiffIrradianceTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pDownSampleTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pGBufTexs[static_cast<UINT>(eGbuf::cnt)];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pGenMipUavTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pHdrTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pHdrUploadCb;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pHiZCullReadCb;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pHiZCullListTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pHiZTempTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pNoiseTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pNoiseUploadCb;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pPrefilterTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pQueryReadBuffers[_WORKER_THREAD_COUNT];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pSceneTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pSceneDepthTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pScreenTexs[_BUFFER_COUNT];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pScreenDepthTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pSkyBoxTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pSsaoTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pSsaoBlurTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pSsaoBlurDebugTex;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pViewportTex;

		D3D12_CPU_DESCRIPTOR_HANDLE m_brdfRtv;
		D3D12_CPU_DESCRIPTOR_HANDLE m_pDepthDebugRtv;
		D3D12_CPU_DESCRIPTOR_HANDLE m_diffIrradianceRtv;
		D3D12_CPU_DESCRIPTOR_HANDLE m_downSampleRtv;
		D3D12_CPU_DESCRIPTOR_HANDLE m_GBufRtvs[static_cast<UINT>(eGbuf::cnt)];
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_hiZTempRtvs;
		D3D12_CPU_DESCRIPTOR_HANDLE m_sceneDsv;
		D3D12_CPU_DESCRIPTOR_HANDLE m_sceneRtv;
		D3D12_CPU_DESCRIPTOR_HANDLE m_screenDsv;
		D3D12_CPU_DESCRIPTOR_HANDLE m_screenRtvs[_BUFFER_COUNT];
		D3D12_CPU_DESCRIPTOR_HANDLE m_skyBoxRtv;
		D3D12_CPU_DESCRIPTOR_HANDLE m_prefilterRtv;
		

		D3D12_GPU_DESCRIPTOR_HANDLE m_aabbCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_borderSsv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_brdfSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_clampSsv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_depthCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_diffIrradianceSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_equirect2CubeCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_exposureCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GBufSrvs[static_cast<UINT>(eGbuf::cnt)];
		D3D12_GPU_DESCRIPTOR_HANDLE m_heightScaleCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_hdrSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_hiZCullListUav;
		D3D12_GPU_DESCRIPTOR_HANDLE m_hiZCullMatrixCbv;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_hiZTempSrvs;
		D3D12_GPU_DESCRIPTOR_HANDLE m_noiseSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_prefilterSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_resolutionCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_roughnessCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_sceneSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_sceneDepthSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_skyBoxSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ssaoSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ssaoUav;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ssaoBlurSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ssaoBlurUav;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ssaoBlurDebugSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ssaoBlurDebugUav;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ssaoKernelCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_ssaoParameterCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_viewportSrv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_viewportUav;
		D3D12_GPU_DESCRIPTOR_HANDLE m_wrapSsv;
		

		D3D12_VERTEX_BUFFER_VIEW m_quadVbv;
		D3D12_VERTEX_BUFFER_VIEW m_skyBoxVbv;
		D3D12_INDEX_BUFFER_VIEW m_aabbIbv;
		D3D12_INDEX_BUFFER_VIEW m_quadIbv;
		D3D12_INDEX_BUFFER_VIEW m_skyBoxIbv;

		D3D12_VIEWPORT m_viewport;
		D3D12_VIEWPORT m_diffIrradViewport;
		D3D12_VIEWPORT m_prefilterViewport;
		D3D12_RECT m_diffIrradRect;
		D3D12_RECT m_prefilterRect;
		D3D12_RECT m_scissorRect;

		std::unique_ptr<Camera12> m_pCam;
		std::unique_ptr<HeapManager> m_pHeapManager;
		std::unique_ptr<LightBuffer12> m_pLightCb;
		std::unique_ptr<MatBuffer12> m_pMatricesCb;
		std::unique_ptr<MatBuffer12> m_pOutlinerMatBuffer;
		std::vector<std::unique_ptr<Object12>> m_pObjects;
		std::unique_ptr<Shader12> m_pShader;
		std::unique_ptr<ShadowMap12> m_pShadowMap;
		static D3D12* g_pD3D12;
		std::vector<Mesh12*>m_pTotalMeshes;
		std::queue<Mesh12*>m_pHWOcclusionQueue[_WORKER_THREAD_COUNT];
		std::queue<Mesh12*> m_pMeshQueue;



		XMMATRIX m_idMat = XMMatrixIdentity();
		XMMATRIX* m_pLitMat;
		BOOL m_bHeightOn;
	
		float m_exposure;
		float m_heightScale;
		float m_ssaoBias;
		float m_ssaoRadius;

		UINT8* m_pAabbCbBegin;
		UINT8* m_pDepthCbBegin;
		UINT8* m_pEquirect2CubeCbBegin;
		UINT8* m_pExposureCbBegin;
		UINT8* m_pHeightScaleCbBegin;
		UINT8* m_pHiZCullMatrixCbBegin;
		UINT8* m_pHiZCullReadCbBegin;
		UINT8* m_pQueryReadCbBegins[_WORKER_THREAD_COUNT];
		UINT8* m_pResolutionCbBegin;
		UINT8* m_pRoughnessCbBegin;
		UINT8* m_pSsaoKernalCbBegin;
		UINT8* m_pSsaoParametersCbBegin;
	
		UINT m_clientWidth;
		UINT m_clientHeight;
		UINT m_curFrame;
		UINT m_fenceValue;
		UINT m_nDrawCall;
		UINT m_nHiZPassed;
		UINT m_nHWOcclusionPassed[_WORKER_THREAD_COUNT];
		UINT m_nNotOccluded;
		UINT m_nSsaoSample;
		UINT m_pbrSetupFenceValue;
		UINT m_queryResultOffsets[_WORKER_THREAD_COUNT];
		UINT m_selectedMesh;
		UINT m_selectedObject;
		UINT m_ssaoFenceValue;
		UINT m_workerFenceValues[_WORKER_THREAD_COUNT];
		UINT m_workerThreadIdx[_WORKER_THREAD_COUNT];

	};
}