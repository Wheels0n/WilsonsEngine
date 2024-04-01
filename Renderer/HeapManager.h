#pragma once
#include <Windows.h>
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{	
	class HeapManager
	{
	public:
		inline ID3D12Heap* GetConstantBufferHeap()
		{
			return m_pCbHeap;
		}
		inline ID3D12Heap* GetTexHeap()
		{
			return m_pTexHeap;
		}
		inline UINT64 GetIndexBufferHeapOffset()
		{
			return m_curIbHeapOffset;
		}
		inline UINT64 GetConstantBufferHeapOffset()
		{
			return m_curCbHeapOffset;
		}
		inline UINT64 GetTexHeapOffset()
		{
			return m_curTexHeapOffset;
		}
		inline UINT64 GetQueryHeapOffset()
		{
			return m_curQueryHeapOffset;
		}
		inline UINT64 GetIndexBufferBlockOffset(UINT idx)
		{
			return m_curIbBlockOffset[idx];
		}
		inline UINT64 GetConstantBufferBlockOffset(UINT idx)
		{
			return m_curCbBlockOffset[idx];
		}
		inline UINT8* GetConstantBlockMappedPtr(UINT idx)
		{
			return m_pCurCbBlockMappedPtr[idx];
		}
		inline ID3D12Resource* GetConstantBufferBlock(UINT idx)
		{
			return m_pCbBlock[idx];
		}
		inline ID3D12Resource* GetQueryBlock()
		{
			return m_pQueryResultBlock;
		}
		inline void SetConstantBufferHeapOffset(UINT64 newCbOffset)
		{
			m_curCbHeapOffset = newCbOffset;
		}
		inline void SetTexHeapOffset(UINT64 newTexOffset)
		{
			m_curTexHeapOffset = newTexOffset;
		}
		inline void SetQueryHeapOffset(UINT64 newQueryOffset)
		{
			m_curQueryHeapOffset = newQueryOffset;
		}
		inline void SetVertexBufferBlockOffset(UINT idx, UINT64 newVbBlockOffset)
		{
			m_curVbBlockOffset[idx] = newVbBlockOffset;
		}
		inline void SetIndexBufferBlockOffset(UINT idx, UINT64 newIbBlockOffset)
		{
			m_curIbBlockOffset[idx] = newIbBlockOffset;
		}
		inline void SetConstantBufferBlockOffset(UINT idx, UINT64 newCbBlockOffset)
		{
			m_curCbBlockOffset[idx] = newCbBlockOffset;
		}
		inline void SetVertexBlockMappedPtr(UINT idx, UINT8* newVbBlockMappedPtr)
		{
			m_pCurVbBlockMappedPtr[idx] = newVbBlockMappedPtr;
		}
		inline void SeIndexBlockMappedPtr(UINT idx, UINT8* newIbBlockMappedPtr)
		{
			m_pCurIbBlockMappedPtr[idx] = newIbBlockMappedPtr;
		}
		inline void SetConstantBlockMappedPtr(UINT idx, UINT8* newCbBlockMappedPtr)
		{
			m_pCurCbBlockMappedPtr[idx] = newCbBlockMappedPtr;
		}
		inline ID3D12DescriptorHeap** GetCbvSrvUavHeap()
		{
			return &m_pCbvSrvUavHeap;
		};
		inline ID3D12DescriptorHeap** GetRtvHeap()
		{
			return &m_pRtvHeap;
		};
		inline ID3D12DescriptorHeap** GetDsvHeap()
		{
			return &m_pDsvHeap;
		};
		inline ID3D12DescriptorHeap** GetSamplerHeap()
		{
			return &m_pSamplerHeap;
		};
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurRtvHandle()
		{
			return m_curRtvHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurDsvHandle()
		{
			return m_curDsvHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurCbvSrvCpuHandle()
		{
			return m_curCbvSrvUavCpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurSamplerCpuHandle()
		{
			return m_curSamplerCpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetCurCbvSrvGpuHandle()
		{
			return m_curCbvSrvUavGpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetCurSamplerGpuHandle()
		{
			return m_curSamplerGpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurDstCbvSrvCpuHandle()
		{
			return m_curDstCbvSrvUavCpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurDstSamplerCpuHandle()
		{
			return m_curDstSamplerCpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetDstCurCbvSrvGpuHandle()
		{
			return m_curDstCbvSrvUavGpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetDstCurSamplerGpuHandle()
		{
			return m_curDstSamplerGpuHandle;
		}

		void IncreaseRtvHandleOffset();
		void IncreaseDsvHandleOffset();
		void IncreaseCbvSrvHandleOffset();
		void IncreaseSamplerHandleOffset();
		UINT8* GetCbMappedPtr(UINT64 size);
		void AllocateVertexData(UINT8* pVertexData, UINT64 vbSize);
		void AllocateIndexData(UINT8* pIndexData, UINT64 ibSize);
		D3D12_VERTEX_BUFFER_VIEW GetVBV(UINT64 size, UINT64 stride);
		D3D12_INDEX_BUFFER_VIEW GetIBV(UINT64 size, UINT64 offset);
		D3D12_GPU_DESCRIPTOR_HANDLE GetCBV(UINT64 size, ID3D12Device* pDevice);
		D3D12_GPU_DESCRIPTOR_HANDLE GetSRV(D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, ID3D12Resource* pTex, ID3D12Device* pDevice);
		D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, ID3D12Resource* pTex, ID3D12Device* pDevice);
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc, ID3D12Resource* pTex, ID3D12Device* pDevice);
		D3D12_GPU_DESCRIPTOR_HANDLE GetUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, ID3D12Resource* pTex, ID3D12Device* pDevice);
		void CreateTexture(D3D12_RESOURCE_DESC& texDesc, D3D12_RESOURCE_STATES state, ID3D12Resource** ppTex, ID3D12Device* pDevice);

		HeapManager(ID3D12Device*);
		~HeapManager();
	private:
		UINT m_rtvHandleSize;
		UINT m_dsvHandleSize;
		UINT m_samplerHandleSize;
		UINT m_cbvSrvUavHanldeSize;

		UINT64 m_curVbHeapOffset;
		UINT64 m_curIbHeapOffset;
		UINT64 m_curCbHeapOffset;
		UINT64 m_curVbBlockOffset[_HEAP_BLOCK_COUNT];
		UINT64 m_curIbBlockOffset[_HEAP_BLOCK_COUNT];
		UINT64 m_curCbBlockOffset[_HEAP_BLOCK_COUNT];
		UINT8* m_pCurVbBlockMappedPtr[_HEAP_BLOCK_COUNT];
		UINT8* m_pCurIbBlockMappedPtr[_HEAP_BLOCK_COUNT];
		UINT8* m_pCurCbBlockMappedPtr[_HEAP_BLOCK_COUNT];
		UINT64 m_curTexHeapOffset;
		UINT64 m_curQueryHeapOffset;
		ID3D12Heap* m_pVbHeap;
		ID3D12Heap* m_pIbHeap;
		ID3D12Heap* m_pCbHeap;
		ID3D12Heap* m_pQueryHeap;
		ID3D12Heap* m_pTexHeap;
		ID3D12Resource* m_pVbBlock[_HEAP_BLOCK_COUNT];
		ID3D12Resource* m_pIbBlock[_HEAP_BLOCK_COUNT];
		ID3D12Resource* m_pCbBlock[_HEAP_BLOCK_COUNT];

		ID3D12Resource* m_pQueryResultBlock;
		ID3D12DescriptorHeap* m_pRtvHeap;
		ID3D12DescriptorHeap* m_pDsvHeap;
		ID3D12DescriptorHeap* m_pSamplerHeap;
		ID3D12DescriptorHeap* m_pCbvSrvUavHeap;
		ID3D12DescriptorHeap* m_pDstSamplerHeap;
		ID3D12DescriptorHeap* m_pDstCbvSrvHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE m_curRtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDsvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curCbvSrvUavCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curCbvSrvUavGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curSamplerGpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDstCbvSrvUavCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDstSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curDstCbvSrvUavGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curDstSamplerGpuHandle;
	};
}