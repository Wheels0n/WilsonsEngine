#pragma once
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{
	using namespace Microsoft::WRL;
	using namespace std;
	using namespace DirectX;


	constexpr UINT _RB_BLOCK_COUNT = _HI_Z_TEX_COUNT + _WORKER_THREAD_COUNT * 2;

	class HeapManager :public enable_shared_from_this<HeapManager>
	{
	public:
		UINT										AllocateCb(const UINT64 dataSize);
		UINT										AllocateRb(const UINT64 dataSize);

		void										AllocateIndexData(UINT8*const pIndexData, const UINT64 ibSize);
		void										AllocateVertexData(UINT8* const pVertexData, const UINT64 vbSize);

		void										CreateUploadBuffer(ComPtr<ID3D12Resource>& pUploadCB, const UINT64 uploadPitch, const UINT64 uploadSize);
		void										CreateTexture(const D3D12_RESOURCE_DESC& texDesc, const D3D12_RESOURCE_STATES state, 
																		ComPtr<ID3D12Resource>& pTex);
		inline ComPtr <ID3D12Resource>				GetCbBlock(const UINT idx)
		{
			return m_pCbBlock[idx];
		}
		inline ComPtr<ID3D12Heap>					GetCbHeap()
		{
			return m_pCbHeap;
		}
		inline UINT64								GetCbHeapOffset()
		{
			return m_curCbHeapOffset;
		}
		inline UINT8*								GetCbBlockMappedPtr(const UINT idx)
		{
			return m_pCurCbBlockMappedPtr[idx];
		}
		inline UINT64								GetCbBlockOffset(const UINT idx)
		{
			return m_curCbBlockOffset[idx];
		}
		UINT8*										GetCbMappedPtr(const UINT64 size);

		D3D12_GPU_DESCRIPTOR_HANDLE					GetCbv(const UINT64 size);
		inline ID3D12DescriptorHeap**				GetCbvSrvUavHeap()
		{
			return m_pCbvSrvUavHeap.GetAddressOf();
		};
		inline D3D12_CPU_DESCRIPTOR_HANDLE			GetCurCbvSrvCpuHandle()
		{
			return m_curCbvSrvUavCpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE			GetCurCbvSrvGpuHandle()
		{
			return m_curCbvSrvUavGpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE			GetCurDstCbvSrvCpuHandle()
		{
			return m_curDstCbvSrvUavCpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE			GetCurDstSamplerCpuHandle()
		{
			return m_curDstSamplerCpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE			GetCurDsvHandle()
		{
			return m_curDsvHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE			GetCurRtvHandle()
		{
			return m_curRtvHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE			GetCurSamplerCpuHandle()
		{
			return m_curSamplerCpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE			GetCurSamplerGpuHandle()
		{
			return m_curSamplerGpuHandle;
		}
		UINT8*										GetCurVbBlockPtr(const UINT64 vbSize);

		inline D3D12_GPU_DESCRIPTOR_HANDLE			GetDstCurCbvSrvGpuHandle()
		{
			return m_curDstCbvSrvUavGpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE			GetDstCurSamplerGpuHandle()
		{
			return m_curDstSamplerGpuHandle;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE					GetDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc, ComPtr<ID3D12Resource>pTex);
		inline ID3D12DescriptorHeap**				GetDsvHeap()
		{
			return m_pDsvHeap.GetAddressOf();
		};

		inline UINT64								GetIbBlockOffset(const UINT idx)
		{
			return m_curIbBlockOffset[idx];
		}
		inline UINT64								GetIbHeapOffset()
		{
			return m_curIbHeapOffset;
		}
		D3D12_INDEX_BUFFER_VIEW						GetIbv(const UINT64 size, const UINT64 offset);

		ComPtr<ID3D12QueryHeap>						GetQueryHeap(UINT threadIdx) { return m_pQueryHeap[threadIdx]; };
		ComPtr <ID3D12Resource>						GetQueryResultBlock(UINT threadIdx){return m_pQueryResultBlock[threadIdx];}
		inline UINT64								GetQueryResultOffset(UINT threadIdx)
		{
			return m_queryResultOffsets[threadIdx];
		}
		ComPtr <ID3D12Resource>						GetRbBlock(UINT i) { return m_pRbBlock[i]; };
		UINT8*										GetRBRawPtr(UINT i) { return m_rbRawPtrHash[i]; };

		D3D12_CPU_DESCRIPTOR_HANDLE					GetRtv(const D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, ComPtr<ID3D12Resource>pTex);
		inline ID3D12DescriptorHeap**				GetRtvHeap()
		{
			return m_pRtvHeap.GetAddressOf();
		};
		D3D12_GPU_DESCRIPTOR_HANDLE					GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, ComPtr<ID3D12Resource>pTex);
		inline ID3D12DescriptorHeap**				GetSamplerHeap()
		{
			return m_pSamplerHeap.GetAddressOf();
		};
		inline ComPtr<ID3D12Heap>								GetTexHeap()
		{
			return m_pTexHeap;
		}
		inline UINT64								GetTexHeapOffset()
		{
			return m_curTexHeapOffset;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE					GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, ComPtr<ID3D12Resource>pTex);
		D3D12_VERTEX_BUFFER_VIEW					GetVbv(const UINT64 size, const UINT64 stride);

		void										IncreaseCbvSrvHandleOffset();
		void										IncreaseDsvHandleOffset();
		void										IncreaseRtvHandleOffset();
		void										IncreaseSamplerHandleOffset();

		inline void									SetCbBlockOffset(const UINT idx, const UINT64 newCbBlockOffset)
		{
			m_curCbBlockOffset[idx] = newCbBlockOffset;
		}
		inline void									SetCbBlockMappedPtr(const UINT idx, UINT8* const newCbBlockMappedPtr)
		{
			m_pCurCbBlockMappedPtr[idx] = newCbBlockMappedPtr;
		}
		inline void									SetCbHeapOffset(const UINT64 newCbOffset)
		{
			m_curCbHeapOffset = newCbOffset;
		}
		inline void									SetIbBlockOffset(const UINT idx, const UINT64 newIbBlockOffset)
		{
			m_curIbBlockOffset[idx] = newIbBlockOffset;
		}
		inline void									SetIbBlockMappedPtr(const UINT idx, UINT8* const newIbBlockMappedPtr)
		{
			m_pCurIbBlockMappedPtr[idx] = newIbBlockMappedPtr;
		}
		inline void									SetTexHeapOffset(const UINT64 newTexOffset)
		{
			m_curTexHeapOffset = newTexOffset;
		}
		inline void									SetVbBlockOffset(const UINT idx, const UINT64 newVbBlockOffset)
		{
			m_curVbBlockOffset[idx] = newVbBlockOffset;
		}
		inline void 								SetVbMappedPtr(const UINT idx, UINT8* const newVbBlockMappedPtr)
		{
			m_pCurVbBlockMappedPtr[idx] = newVbBlockMappedPtr;
		}

		void										UploadGraphicsCb(const UINT key, const UINT rootParamIdx, ComPtr<ID3D12GraphicsCommandList> pCmdList);
		void										UploadComputeCb(const UINT key, const UINT rootParamIdx, ComPtr<ID3D12GraphicsCommandList> pCmdList);
		void										SetDescriptorHeaps(ComPtr<ID3D12GraphicsCommandList> pCmdList);
		void										SetQueryResultOffset(UINT threadIdx, UINT64 offset) { m_queryResultOffsets[threadIdx] = offset; };
		void										CopyDataToCb(const UINT key, const UINT64 dataSize, const void* pSrc);
		void										CopyDataToCbWithOffset(const UINT key, const UINT64 dataSize, const UINT offset, const void* pSrc);
		
													HeapManager(ComPtr<ID3D12Device>);
													~HeapManager() = default;
	private:

		UINT										m_cbvSrvUavHanldeSize;
		UINT										m_dsvHandleSize;
		UINT										m_rtvHandleSize;
		UINT										m_samplerHandleSize;
		UINT										m_rbIdx;

		UINT64										m_curRbHeapOffset;
		UINT64										m_curCbHeapOffset;
		UINT64										m_curIbHeapOffset;
		UINT64										m_curVbHeapOffset;
		UINT64										m_curTexHeapOffset;

		UINT64										m_queryResultOffsets[_WORKER_THREAD_COUNT];
		UINT64										m_curCbBlockOffset[_HEAP_BLOCK_COUNT];
		UINT64										m_curIbBlockOffset[_HEAP_BLOCK_COUNT];
		UINT64										m_curVbBlockOffset[_HEAP_BLOCK_COUNT];

		UINT8*										m_pCurCbBlockMappedPtr[_HEAP_BLOCK_COUNT];
		UINT8*										m_pCurIbBlockMappedPtr[_HEAP_BLOCK_COUNT];
		UINT8*										m_pCurVbBlockMappedPtr[_HEAP_BLOCK_COUNT];

		unordered_map<UINT, UINT8*>						m_cbRawPtrHash;
		unordered_map<UINT, UINT8*>						m_rbRawPtrHash;
		unordered_map<UINT, D3D12_CPU_DESCRIPTOR_HANDLE> m_cpuHandleHash;
		unordered_map<UINT, D3D12_GPU_DESCRIPTOR_HANDLE> m_gpuHandleHash;

		ComPtr<ID3D12Heap>							m_pRbHeap;
		ComPtr<ID3D12Heap>							m_pCbHeap;
		ComPtr<ID3D12Heap>							m_pIbHeap;
		ComPtr<ID3D12Heap>							m_pVbHeap;
		ComPtr<ID3D12Heap>							m_pTexHeap;
		ComPtr<ID3D12Heap>							m_pQueryResultHeap;
		ComPtr<ID3D12QueryHeap>						m_pQueryHeap[_WORKER_THREAD_COUNT];

		ComPtr<ID3D12Resource>						m_pRbBlock[_RB_BLOCK_COUNT];
		ComPtr<ID3D12Resource>						m_pCbBlock[_HEAP_BLOCK_COUNT];
		ComPtr<ID3D12Resource>						m_pIbBlock[_HEAP_BLOCK_COUNT];
		ComPtr<ID3D12Resource>						m_pVbBlock[_HEAP_BLOCK_COUNT];
		ComPtr<ID3D12Resource>						m_pQueryResultBlock[_WORKER_THREAD_COUNT];

		ComPtr<ID3D12DescriptorHeap>				m_pCbvSrvUavHeap;
		ComPtr<ID3D12DescriptorHeap>				m_pDstCbvSrvHeap;
		ComPtr<ID3D12DescriptorHeap>				m_pDstSamplerHeap;
		ComPtr<ID3D12DescriptorHeap>				m_pDsvHeap;
		ComPtr<ID3D12DescriptorHeap>				m_pRtvHeap;
		ComPtr<ID3D12DescriptorHeap>				m_pSamplerHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE					m_curCbvSrvUavCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE					m_curDstCbvSrvUavCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE					m_curDstSamplerCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE					m_curDsvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE					m_curRtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE					m_curSamplerCpuHandle;

		D3D12_GPU_DESCRIPTOR_HANDLE					m_curCbvSrvUavGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE					m_curDstCbvSrvUavGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE					m_curDstSamplerGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE					m_curSamplerGpuHandle;
	};

	extern shared_ptr<HeapManager> g_pHeapManager;
}