#pragma once
#include <Windows.h>
#include <d3d12.h>
#include "typedef.h"
namespace wilson
{	
	class DescriptorHeapManager
	{
	public:
		inline ID3D12DescriptorHeap** GetCbvSrvHeap()
		{
			return &m_pCbvSrvHeap;
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
			return m_curCbvSrvCpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurSamplerCpuHandle()
		{
			return m_curSamplerCpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetCurCbvSrvGpuHandle()
		{
			return m_curCbvSrvGpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetCurSamplerGpuHandle()
		{
			return m_curSamplerGpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurDstCbvSrvCpuHandle()
		{
			return m_curDstCbvSrvCpuHandle;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurDstSamplerCpuHandle()
		{
			return m_curDstSamplerCpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetDstCurCbvSrvGpuHandle()
		{
			return m_curDstCbvSrvGpuHandle;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetDstCurSamplerGpuHandle()
		{
			return m_curDstSamplerGpuHandle;
		}

		void IncreaseRtvHandleOffset();
		void IncreaseDsvHandleOffset();
		void IncreaseCbvSrvHandleOffset();
		void IncreaseSamplerHandleOffset();

		DescriptorHeapManager(ID3D12Device*);
		~DescriptorHeapManager();
	private:
		UINT m_rtvHandleSize;
		UINT m_dsvHandleSize;
		UINT m_samplerHandleSize;
		UINT m_cbvSrvHanldeSize;

		ID3D12DescriptorHeap* m_pRtvHeap;
		ID3D12DescriptorHeap* m_pDsvHeap;
		ID3D12DescriptorHeap* m_pSamplerHeap;
		ID3D12DescriptorHeap* m_pCbvSrvHeap;
		ID3D12DescriptorHeap* m_pDstSamplerHeap;
		ID3D12DescriptorHeap* m_pDstCbvSrvHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE m_curRtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDsvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curCbvSrvCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curCbvSrvGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curSamplerGpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDstCbvSrvCpuHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_curDstSamplerCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curDstCbvSrvGpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_curDstSamplerGpuHandle;
	};
}