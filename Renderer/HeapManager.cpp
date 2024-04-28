#include <DirectXTex.h>
#include "HeapManager.h"
namespace wilson
{
	void HeapManager::IncreaseRtvHandleOffset()
	{
		m_curRtvHandle.ptr += m_rtvHandleSize;
	}
	void HeapManager::IncreaseDsvHandleOffset()
	{
		m_curDsvHandle.ptr += m_dsvHandleSize;
	}
	void HeapManager::IncreaseCbvSrvHandleOffset()
	{
		m_curCbvSrvUavCpuHandle.ptr += m_cbvSrvUavHanldeSize;
		m_curCbvSrvUavGpuHandle.ptr += m_cbvSrvUavHanldeSize;
	}
	void HeapManager::IncreaseSamplerHandleOffset()
	{
		m_curSamplerCpuHandle.ptr += m_samplerHandleSize;
		m_curSamplerGpuHandle.ptr += m_samplerHandleSize;
	}

	UINT8* HeapManager::GetCbMappedPtr(const UINT64 size)
	{
		//Get Block Idx
		//현재 Offset+size가 64kb경계를 넘기면 
		//그게 아니면 바로 _CBV_ALIGN(size)
		UINT64 blockSize = (_CB_HEAP_SIZE / _HEAP_BLOCK_COUNT);
		UINT idx = m_curCbHeapOffset / blockSize;
		UINT newIdx = (m_curCbHeapOffset + (CUSTUM_ALIGN(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)))/ blockSize;
		if (newIdx != idx)
		{
			m_curCbHeapOffset = CUSTUM_ALIGN(m_curCbHeapOffset, blockSize);
		}
		m_curCbHeapOffset += (CUSTUM_ALIGN(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));

		UINT8* pMappedPtr = m_pCurCbBlockMappedPtr[newIdx];
		m_pCurCbBlockMappedPtr[newIdx] = pMappedPtr + (CUSTUM_ALIGN(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
		return pMappedPtr;
	}

	void HeapManager::AllocateVertexData(UINT8* const pVertexData, const UINT64 vbSize)
	{
		UINT64 blockSize = (_VB_HEAP_SIZE )/ _HEAP_BLOCK_COUNT;
		UINT idx = m_curVbHeapOffset / blockSize;
		UINT newIdx = (m_curVbHeapOffset + vbSize) / blockSize;
		if (newIdx != idx)
		{
			m_curVbHeapOffset = CUSTUM_ALIGN(m_curVbHeapOffset, blockSize);
		}
		m_curVbHeapOffset += vbSize;

		memcpy(m_pCurVbBlockMappedPtr[newIdx], pVertexData, vbSize);
		m_pCurVbBlockMappedPtr[newIdx] += vbSize;
	}

	UINT8* HeapManager::GetCurVbBlockPtr(const UINT64 vbSize)
	{

		UINT64 blockSize = (_VB_HEAP_SIZE) / _HEAP_BLOCK_COUNT;
		UINT idx = m_curVbHeapOffset / blockSize;
		UINT newIdx = (m_curVbHeapOffset + vbSize) / blockSize;
		if (newIdx != idx)
		{
			m_curVbHeapOffset = CUSTUM_ALIGN(m_curVbHeapOffset, blockSize);
		}
		return m_pCurVbBlockMappedPtr[newIdx];
	}

	void HeapManager::AllocateIndexData(UINT8* const pIndexData, const UINT64 ibSize)
	{
		UINT64 blockSize = (_IB_HEAP_SIZE / _HEAP_BLOCK_COUNT);
		UINT idx = m_curIbHeapOffset / blockSize;
		UINT newIdx = (m_curIbHeapOffset + ibSize) / blockSize;
		if (newIdx != idx)
		{
			m_curIbHeapOffset = CUSTUM_ALIGN(m_curIbHeapOffset, blockSize);
		}
		m_curIbHeapOffset += ibSize;

		memcpy(m_pCurIbBlockMappedPtr[newIdx], pIndexData, ibSize);
		m_pCurIbBlockMappedPtr[newIdx] += ibSize;
	}

	D3D12_VERTEX_BUFFER_VIEW HeapManager::GetVbv(const UINT64 size, const UINT64 stride)
	{
		UINT64 blockSize = (_VB_HEAP_SIZE / _HEAP_BLOCK_COUNT);
		UINT idx = m_curVbHeapOffset / blockSize;
		D3D12_VERTEX_BUFFER_VIEW vbv = {};
		vbv.BufferLocation = m_pVbBlock[idx]->GetGPUVirtualAddress() + m_curVbBlockOffset[idx];
		vbv.SizeInBytes = size;
		vbv.StrideInBytes = stride;

		m_curVbBlockOffset[idx]+=size;
		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW HeapManager::GetIbv(const UINT64 size, const UINT64 offset)
	{
		UINT64 blockSize = (_IB_HEAP_SIZE / _HEAP_BLOCK_COUNT);
		UINT idx = m_curIbHeapOffset / blockSize;
		D3D12_INDEX_BUFFER_VIEW ibv = {};
		ibv.BufferLocation = m_pIbBlock[idx]->GetGPUVirtualAddress() + m_curIbBlockOffset[idx]+offset;
		ibv.Format = DXGI_FORMAT_R32_UINT;
		ibv.SizeInBytes = size;
		
		return ibv;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE HeapManager::GetCbv(const UINT64 size, ID3D12Device* const  pDevice)
	{	
		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_curCbvSrvUavCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_curCbvSrvUavGpuHandle;

		UINT idx = m_curCbHeapOffset /( _CB_HEAP_SIZE);
		//

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.SizeInBytes = CUSTUM_ALIGN(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);//256aligned
		cbvDesc.BufferLocation = m_pCbBlock[idx]->GetGPUVirtualAddress() + m_curCbBlockOffset[idx];
		pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
		IncreaseCbvSrvHandleOffset();
		m_curCbBlockOffset[idx] += cbvDesc.SizeInBytes;
		return cbvSrvGpuHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE HeapManager::GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, ID3D12Resource* const pTex, ID3D12Device* const pDevice)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_curCbvSrvUavCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_curCbvSrvUavGpuHandle;
		pDevice->CreateShaderResourceView(pTex, &srvDesc, cbvSrvCpuHandle);
		IncreaseCbvSrvHandleOffset();

		return cbvSrvGpuHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE HeapManager::GetRtv(const D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, ID3D12Resource* const pTex, ID3D12Device* const pDevice)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = m_curRtvHandle;
		pDevice->CreateRenderTargetView(pTex, &rtvDesc, rtvCpuHandle);
		IncreaseRtvHandleOffset();

		return rtvCpuHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE HeapManager::GetDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc, ID3D12Resource* const pTex, ID3D12Device* const pDevice)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = m_curDsvHandle;
		pDevice->CreateDepthStencilView(pTex, &dsvDesc, dsvCpuHandle);
		IncreaseDsvHandleOffset();

		return dsvCpuHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE HeapManager::GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, ID3D12Resource* const pTex, ID3D12Device* const pDevice)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_curCbvSrvUavCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_curCbvSrvUavGpuHandle;
		pDevice->CreateUnorderedAccessView(pTex, nullptr, &uavDesc, cbvSrvCpuHandle);
		IncreaseCbvSrvHandleOffset();
		return cbvSrvGpuHandle;
	}

	void HeapManager::CreateTexture(const D3D12_RESOURCE_DESC& texDesc, const D3D12_RESOURCE_STATES state, ID3D12Resource** const  ppTex, ID3D12Device* const  pDevice)
	{
		D3D12_CLEAR_VALUE clearVal = {};
		if (texDesc.Flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		{
			clearVal.Format = DXGI_FORMAT_D32_FLOAT;
			clearVal.DepthStencil.Depth = 1.0f;
		}
		else
		{
			clearVal.Format = texDesc.Format;
			clearVal.Color[3] = 1.0f;
		}

		//pOptimizedClearValue must be NULL when D3D12_RESOURCE_DESC::Dimension is
		//not D3D12_RESOURCE_DIMENSION_BUFFER and neither D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		//nor D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL are set in D3D12_RESOURCE_DESC::Flags.
		D3D12_CLEAR_VALUE* pClearValue = (texDesc.Dimension!=D3D12_RESOURCE_DIMENSION_BUFFER)&&
			((texDesc.Flags!= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) && (texDesc.Flags != D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			?NULL:&clearVal;

		D3D12_RESOURCE_ALLOCATION_INFO info = pDevice->GetResourceAllocationInfo(0, 1, &texDesc);

		HRESULT hr = pDevice->CreatePlacedResource(m_pTexHeap, m_curTexHeapOffset, &texDesc,
			state, pClearValue, IID_PPV_ARGS(ppTex));
		assert(SUCCEEDED(hr));
		m_curTexHeapOffset += CUSTUM_ALIGN(info.SizeInBytes,_64KB);
	}

	HeapManager::HeapManager(ID3D12Device* const pDevice)
	{
		m_pVbHeap = nullptr;
		m_pIbHeap = nullptr;
		m_pCbHeap = nullptr;
		m_pQueryHeap = nullptr;
		m_pTexHeap = nullptr;
		m_pRtvHeap = nullptr;
		m_pDsvHeap = nullptr;
		m_pSamplerHeap = nullptr;
		m_pCbvSrvUavHeap = nullptr;
		m_pDstSamplerHeap = nullptr;
		m_pDstCbvSrvHeap = nullptr;

		m_curVbHeapOffset = 0;
		m_curIbHeapOffset = 0;
		m_curCbHeapOffset = 0;
		m_curTexHeapOffset = 0;

		for (int i = 0; i < _HEAP_BLOCK_COUNT; ++i)
		{
			m_curVbBlockOffset[i] = 0;
			m_curIbBlockOffset[i] = 0;
			m_curCbBlockOffset[i] = 0;
		}
		m_pQueryResultBlock = nullptr;
	
		HRESULT hr;

		D3D12_HEAP_DESC heapDesc = {};
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		heapDesc.SizeInBytes = _VB_HEAP_SIZE;
		heapDesc.Properties = heapProps;

		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_pVbHeap));
		assert(SUCCEEDED(hr));
		m_pVbHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pVbHeap") - 1, "HeapManager::m_pVbHeap");

		heapDesc.SizeInBytes = _IB_HEAP_SIZE;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_pIbHeap));
		assert(SUCCEEDED(hr));
		m_pIbHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pIbHeap") - 1, "HeapManager::m_pIbHeap");

		heapDesc.SizeInBytes = _CB_HEAP_SIZE;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_pCbHeap));
		assert(SUCCEEDED(hr));
		m_pCbHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pCbHeap") - 1, "HeapManager::m_pCbHeap");

		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.SizeInBytes = _TEX_HEAP_SIZE;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_pTexHeap));
		assert(SUCCEEDED(hr));
		m_pTexHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pTexHeap") - 1, "HeapManager::m_pTexHeap");


		heapDesc.SizeInBytes = _64KB;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_pQueryHeap));
		assert(SUCCEEDED(hr));
		m_pQueryHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pQueryHeap") - 1, "HeapManager::m_pQueryHeap");


		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		bufferDesc.Alignment = 0;
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;

		D3D12_RANGE readRange = { 0, };
		for (UINT i = 0; i < _HEAP_BLOCK_COUNT; ++i)
		{
			bufferDesc.Width = (_VB_HEAP_SIZE)/_HEAP_BLOCK_COUNT;
			UINT64 offset = i * bufferDesc.Width;
			hr = pDevice->CreatePlacedResource(m_pVbHeap, offset,
				&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pVbBlock[i]));
			assert(SUCCEEDED(hr));
			m_pVbBlock[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("HeapManager::m_pVbBlock[i]") - 1, "HeapManager::m_pVbBlock[i]");
			hr = m_pVbBlock[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pCurVbBlockMappedPtr[i]));
			assert(SUCCEEDED(hr));

			bufferDesc.Width = (_IB_HEAP_SIZE) / _HEAP_BLOCK_COUNT;
			hr = pDevice->CreatePlacedResource(m_pIbHeap, i * bufferDesc.Width,
				&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(&m_pIbBlock[i]));
			assert(SUCCEEDED(hr));
			m_pIbBlock[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("HeapManager::m_pIbBlock[i]") - 1, "HeapManager::m_pIbBlock[i]");

			hr = m_pIbBlock[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pCurIbBlockMappedPtr[i]));
			assert(SUCCEEDED(hr));

			bufferDesc.Width = (_CB_HEAP_SIZE) / _HEAP_BLOCK_COUNT;
			hr = pDevice->CreatePlacedResource(m_pCbHeap, i* bufferDesc.Width,
				&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pCbBlock[i]));
			assert(SUCCEEDED(hr));
			m_pCbBlock[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("HeapManager::m_pCbBlock[i]") - 1, "HeapManager::m_pCbBlock[i]");

			hr = m_pCbBlock[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pCurCbBlockMappedPtr[i]));
			assert(SUCCEEDED(hr));
		}

		//QueryBlock
		{
			bufferDesc.Width = _64KB;
			hr = pDevice->CreatePlacedResource(m_pQueryHeap, 0,
				&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
				nullptr, IID_PPV_ARGS(&m_pQueryResultBlock));
			assert(SUCCEEDED(hr));
			m_pQueryResultBlock->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("HeapManager::m_pQueryResultBlock") - 1, "HeapManager::m_pQueryResultBlock");
		}



		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = _RTV_DSV_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap));
		assert(SUCCEEDED(hr));
		m_pRtvHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pRtvHeap") - 1, "HeapManager::m_pRtvHeap");


		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = _RTV_DSV_COUNT;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap));
		assert(SUCCEEDED(hr));
		m_pDsvHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pDsvHeap") - 1, "HeapManager::m_pDsvHeap");

		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = _CBV_SRV_COUNT;
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_pCbvSrvUavHeap));
		assert(SUCCEEDED(hr));
		m_pCbvSrvUavHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pCbvSrvUavHeap") - 1, "HeapManager::m_pCbvSrvUavHeap");

		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = _SAMPLER_COUNT;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_pSamplerHeap));
		assert(SUCCEEDED(hr));
		m_pSamplerHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pSamplerHeap") - 1, "HeapManager::m_pSamplerHeap");

		D3D12_DESCRIPTOR_HEAP_DESC dstCbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = _CBV_SRV_COUNT;
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_pDstCbvSrvHeap));
		assert(SUCCEEDED(hr));
		m_pDstCbvSrvHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pDstCbvSrvHeap") - 1, "HeapManager::m_pDstCbvSrvHeap");

		D3D12_DESCRIPTOR_HEAP_DESC dstSamplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = _SAMPLER_COUNT;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_pDstSamplerHeap));
		assert(SUCCEEDED(hr));
		m_pDstSamplerHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("HeapManager::m_pDstSamplerHeap") - 1, "HeapManager::m_pDstSamplerHeap");

		m_curRtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
		m_curDsvHandle = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
		m_curCbvSrvUavCpuHandle = m_pCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
		m_curSamplerCpuHandle = m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart();
		m_curCbvSrvUavGpuHandle = m_pCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
		m_curSamplerGpuHandle = m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
		m_curDstCbvSrvUavCpuHandle = m_pDstCbvSrvHeap->GetCPUDescriptorHandleForHeapStart();
		m_curDstSamplerCpuHandle = m_pDstSamplerHeap->GetCPUDescriptorHandleForHeapStart();
		m_curDstCbvSrvUavGpuHandle = m_pDstCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
		m_curDstSamplerGpuHandle = m_pDstSamplerHeap->GetGPUDescriptorHandleForHeapStart();

		m_rtvHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_dsvHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_cbvSrvUavHanldeSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_samplerHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}
	HeapManager::~HeapManager()
	{	
		for (int i = 0; i < _HEAP_BLOCK_COUNT; ++i)
		{
			if (m_pVbBlock[i] != nullptr)
			{
				m_pVbBlock[i]->Release();
				m_pVbBlock[i] = nullptr;
			}

			if (m_pIbBlock[i] != nullptr)
			{
				m_pIbBlock[i]->Release();
				m_pIbBlock[i] = nullptr;
			}

			if (m_pCbBlock[i] != nullptr)
			{
				m_pCbBlock[i]->Release();
				m_pCbBlock[i] = nullptr;
			}
		}

		if (m_pQueryResultBlock != nullptr)
		{
			m_pQueryResultBlock->Release();
			m_pQueryResultBlock = nullptr;
		}

		if (m_pVbHeap != nullptr)
		{
			m_pVbHeap->Release();
			m_pVbHeap = nullptr;			
		}

		if (m_pIbHeap != nullptr)
		{
			m_pIbHeap->Release();
			m_pIbHeap = nullptr;
		}

		if (m_pCbHeap != nullptr)
		{
			m_pCbHeap->Release();
			m_pCbHeap = nullptr;
		}
		if (m_pTexHeap != nullptr)
		{
			m_pTexHeap->Release();
			m_pTexHeap = nullptr;
		}

		if (m_pQueryHeap != nullptr)
		{
			m_pQueryHeap->Release();
			m_pQueryHeap = nullptr;
		}

		if (m_pRtvHeap != nullptr)
		{
			m_pRtvHeap->Release();
			m_pRtvHeap = nullptr;
		}

		if (m_pDsvHeap != nullptr)
		{
			m_pDsvHeap->Release();
			m_pDsvHeap = nullptr;
		}

		if (m_pSamplerHeap != nullptr)
		{
			m_pSamplerHeap->Release();
			m_pSamplerHeap = nullptr;
		}

		if (m_pCbvSrvUavHeap != nullptr)
		{
			m_pCbvSrvUavHeap->Release();
			m_pCbvSrvUavHeap = nullptr;
		}

		if (m_pDstSamplerHeap != nullptr)
		{
			m_pDstSamplerHeap->Release();
			m_pDstSamplerHeap = nullptr;
		}

		if (m_pDstCbvSrvHeap != nullptr)
		{
			m_pDstCbvSrvHeap->Release();
			m_pDstCbvSrvHeap = nullptr;
		}

	}
}