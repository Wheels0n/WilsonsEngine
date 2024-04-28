#pragma once
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{	
	class HeapManager
	{
	public:
		void AllocateIndexData(UINT8*const pIndexData, const UINT64 ibSize);
		void AllocateVertexData(UINT8* const pVertexData, const UINT64 vbSize);
		void CreateTexture(const D3D12_RESOURCE_DESC& texDesc, const D3D12_RESOURCE_STATES state, ID3D12Resource** const ppTex, ID3D12Device* const pDevice);
		inline ID3D12Resource* GetCbBlock(const UINT idx)
		{
			return m_pCbBlock[idx];
		}
		inline ID3D12Heap* GetCbHeap()
		{
			return m_pCbHeap;
		}
		inline UINT64 GetCbHeapOffset()
		{
			return m_curCbHeapOffset;
		}
		inline UINT8* GetCbBlockMappedPtr(const UINT idx)
		{
			return m_pCurCbBlockMappedPtr[idx];
		}
		inline UINT64 GetCbBlockOffset(const UINT idx)
		{
			return m_curCbBlockOffset[idx];
		}
		UINT8* GetCbMappedPtr(const UINT64 size);
		D3D12_GPU_DESCRIPTOR_HANDLE GetCbv(const UINT64 size, ID3D12Device* const pDevice);
		inline ID3D12DescriptorHeap** GetCbvSrvUavHeap()
		{
			return &m_pCbvSrvUavHeap;
		};
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurCbvSrvCpuHandle()
		{
			return m_curCbvSrvUavCpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetCurCbvSrvGpuHandle()
		{
			return m_curCbvSrvUavGpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurDstCbvSrvCpuHandle()
		{
			return m_curDstCbvSrvUavCpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurDstSamplerCpuHandle()
		{
			return m_curDstSamplerCpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurDsvHandle()
		{
			return m_curDsvHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurRtvHandle()
		{
			return m_curRtvHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurSamplerCpuHandle()
		{
			return m_curSamplerCpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetCurSamplerGpuHandle()
		{
			return m_curSamplerGpuHandle;
		}
		UINT8* GetCurVbBlockPtr(const UINT64 vbSize);
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetDstCurCbvSrvGpuHandle()
		{
			return m_curDstCbvSrvUavGpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetDstCurSamplerGpuHandle()
		{
			return m_curDstSamplerGpuHandle;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE GetDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc, ID3D12Resource* const pTex, ID3D12Device* const pDevice);
		inline ID3D12DescriptorHeap** GetDsvHeap()
		{
			return &m_pDsvHeap;
		};
		inline UINT64 GetIbBlockOffset(const UINT idx)
		{
			return m_curIbBlockOffset[idx];
		}
		inline UINT64 GetIbHeapOffset()
		{
			return m_curIbHeapOffset;
		}
		D3D12_INDEX_BUFFER_VIEW GetIbv(const UINT64 size, const UINT64 offset);
		inline ID3D12Resource* GetQueryBlock()
		{
			return m_pQueryResultBlock;
		}
		inline UINT64 GetQueryHeapOffset()
		{
			return m_curQueryHeapOffset;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE GetRtv(const D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, ID3D12Resource* const pTex, ID3D12Device* const pDevice);
		inline ID3D12DescriptorHeap** GetRtvHeap()
		{
			return &m_pRtvHeap;
		};
		D3D12_GPU_DESCRIPTOR_HANDLE GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, ID3D12Resource* const pTex, ID3D12Device* const pDevice);
		inline ID3D12DescriptorHeap** GetSamplerHeap()
		{
			return &m_pSamplerHeap;
		};
		inline ID3D12Heap* GetTexHeap()
		{
			return m_pTexHeap;
		}
		inline UINT64 GetTexHeapOffset()
		{
			return m_curTexHeapOffset;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, ID3D12Resource* const pTex, ID3D12Device* const pDevice);
		D3D12_VERTEX_BUFFER_VIEW GetVbv(const UINT64 size, const UINT64 stride);
		void IncreaseCbvSrvHandleOffset();
		void IncreaseDsvHandleOffset();
		void IncreaseRtvHandleOffset();
		void IncreaseSamplerHandleOffset();
		inline void SetCbBlockOffset(const UINT idx, const UINT64 newCbBlockOffset)
		{
			m_curCbBlockOffset[idx] = newCbBlockOffset;
		}
		inline void SetCbBlockMappedPtr(const UINT idx, UINT8* const newCbBlockMappedPtr)
		{
			m_pCurCbBlockMappedPtr[idx] = newCbBlockMappedPtr;
		}
		inline void SetCbHeapOffset(const UINT64 newCbOffset)
		{
			m_curCbHeapOffset = newCbOffset;
		}
		inline void SetIbBlockOffset(const UINT idx, const UINT64 newIbBlockOffset)
		{
			m_curIbBlockOffset[idx] = newIbBlockOffset;
		}
		inline void SetIbBlockMappedPtr(const UINT idx, UINT8* const newIbBlockMappedPtr)
		{
			m_pCurIbBlockMappedPtr[idx] = newIbBlockMappedPtr;
		}
		inline void SetQueryHeapOffset(const UINT64 newQueryOffset)
		{
			m_curQueryHeapOffset = newQueryOffset;
		}
		inline void SetTexHeapOffset(const UINT64 newTexOffset)
		{
			m_curTexHeapOffset = newTexOffset;
		}
		inline void SetVbBlockOffset(const UINT idx, const UINT64 newVbBlockOffset)
		{
			m_curVbBlockOffset[idx] = newVbBlockOffset;
		}
		inline void SetVbMappedPtr(const UINT idx, UINT8* const newVbBlockMappedPtr)
		{
			m_pCurVbBlockMappedPtr[idx] = newVbBlockMappedPtr;
		}

		HeapManager(ID3D12Device* const);
		~HeapManager();
	private:
		UINT m_cbvSrvUavHanldeSize;
		UINT m_dsvHandleSize;
		UINT m_rtvHandleSize;
		UINT m_samplerHandleSize;

		UINT64 m_curCbHeapOffset;
		UINT64 m_curIbHeapOffset;
		UINT64 m_curQueryHeapOffset;
		UINT64 m_curVbHeapOffset;
		UINT64 m_curTexHeapOffset;
		UINT64 m_curCbBlockOffset[_HEAP_BLOCK_COUNT];
		UINT64 m_curIbBlockOffset[_HEAP_BLOCK_COUNT];
		UINT64 m_curVbBlockOffset[_HEAP_BLOCK_COUNT];

		UINT8* m_pCurCbBlockMappedPtr[_HEAP_BLOCK_COUNT];
		UINT8* m_pCurIbBlockMappedPtr[_HEAP_BLOCK_COUNT];
		UINT8* m_pCurVbBlockMappedPtr[_HEAP_BLOCK_COUNT];

		ID3D12Heap* m_pCbHeap;
		ID3D12Heap* m_pIbHeap;
		ID3D12Heap* m_pQueryHeap;
		ID3D12Heap* m_pVbHeap;
		ID3D12Heap* m_pTexHeap;

		ID3D12Resource* m_pCbBlock[_HEAP_BLOCK_COUNT];
		ID3D12Resource* m_pIbBlock[_HEAP_BLOCK_COUNT];
		ID3D12Resource* m_pQueryResultBlock;
		ID3D12Resource* m_pVbBlock[_HEAP_BLOCK_COUNT];

		ID3D12DescriptorHeap* m_pCbvSrvUavHeap;
		ID3D12DescriptorHeap* m_pDstCbvSrvHeap;
		ID3D12DescriptorHeap* m_pDstSamplerHeap;
		ID3D12DescriptorHeap* m_pDsvHeap;
		ID3D12DescriptorHeap* m_pRtvHeap;
		ID3D12DescriptorHeap* m_pSamplerHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE m_curCbvSrvUavCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDstCbvSrvUavCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDstSamplerCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDsvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curRtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curSamplerCpuHandle;

		D3D12_GPU_DESCRIPTOR_HANDLE m_curCbvSrvUavGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curDstCbvSrvUavGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curDstSamplerGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curSamplerGpuHandle;
	};
}