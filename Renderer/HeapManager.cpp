#include <DirectXTex.h>
#include "HeapManager.h"
#include "D3D12.h"
namespace wilson
{
	shared_ptr<HeapManager> g_pHeapManager = nullptr;
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

	UINT HeapManager::AllocateCb(const UINT64 dataSize)
	{

		UINT idx = m_cbRawPtrHash.size();
		m_cbRawPtrHash[idx]=GetCbMappedPtr(dataSize);
		m_gpuHandleHash[idx]= GetCbv(dataSize);
		return idx;
	}
	UINT HeapManager::AllocateRb(const UINT64 dataSize)
	{
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

		D3D12_RANGE readRange = { 0, dataSize};
		

		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();
		
		bufferDesc.Width = dataSize;
		HRESULT hr = pDevice->CreatePlacedResource(m_pRbHeap.Get(), m_curRbHeapOffset,
			&bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(m_pRbBlock[m_rbIdx].GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pRbBlock[m_rbIdx]);
		m_curRbHeapOffset += dataSize;

		UINT8* ptr = nullptr;
		hr = m_pRbBlock[m_rbIdx]->Map(0, &readRange, reinterpret_cast<void**>(&ptr));
		assert(SUCCEEDED(hr));
		m_rbRawPtrHash[m_rbIdx] = ptr;

		return m_rbIdx++;
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

	D3D12_GPU_DESCRIPTOR_HANDLE HeapManager::GetCbv(const UINT64 size)
	{	
		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_curCbvSrvUavCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_curCbvSrvUavGpuHandle;

		UINT idx = m_curCbHeapOffset /( _CB_HEAP_SIZE);
		//

		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.SizeInBytes = CUSTUM_ALIGN(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);//256aligned
		cbvDesc.BufferLocation = m_pCbBlock[idx]->GetGPUVirtualAddress() + m_curCbBlockOffset[idx];
		pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
		IncreaseCbvSrvHandleOffset();
		m_curCbBlockOffset[idx] += cbvDesc.SizeInBytes;
		return cbvSrvGpuHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE HeapManager::GetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, ComPtr<ID3D12Resource>pTex)
	{

		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();

		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_curCbvSrvUavCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_curCbvSrvUavGpuHandle;
		pDevice->CreateShaderResourceView(pTex.Get(), &srvDesc, cbvSrvCpuHandle);
		IncreaseCbvSrvHandleOffset();

		return cbvSrvGpuHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE HeapManager::GetRtv(const D3D12_RENDER_TARGET_VIEW_DESC rtvDesc, ComPtr<ID3D12Resource>pTex)
	{

		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();

		D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = m_curRtvHandle;
		pDevice->CreateRenderTargetView(pTex.Get(), &rtvDesc, rtvCpuHandle);
		IncreaseRtvHandleOffset();

		return rtvCpuHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE HeapManager::GetDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc, ComPtr<ID3D12Resource>pTex)
	{

		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();

		D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle = m_curDsvHandle;
		pDevice->CreateDepthStencilView(pTex.Get(), &dsvDesc, dsvCpuHandle);
		IncreaseDsvHandleOffset();

		return dsvCpuHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE HeapManager::GetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, ComPtr<ID3D12Resource>pTex)
	{

		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();

		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = m_curCbvSrvUavCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = m_curCbvSrvUavGpuHandle;
		pDevice->CreateUnorderedAccessView(pTex.Get(), nullptr, &uavDesc, cbvSrvCpuHandle);
		IncreaseCbvSrvHandleOffset();
		return cbvSrvGpuHandle;
	}

	void HeapManager::CreateTexture(const D3D12_RESOURCE_DESC& texDesc, const D3D12_RESOURCE_STATES state, ComPtr<ID3D12Resource>&  pTex)
	{	

		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();

		D3D12_CLEAR_VALUE clearVal = {};
		if (texDesc.Flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		{
			clearVal.Format = DXGI_FORMAT_D32_FLOAT;
			clearVal.DepthStencil.Depth = 1.0f;
		}
		else
		{
			clearVal.Format = texDesc.Format;
			clearVal.Color[0] = 0.0f;
			clearVal.Color[1] = 0.0f;
			clearVal.Color[2] = 0.0f;
			clearVal.Color[3] = 1.0f;
		}

		//pOptimizedClearValue must be NULL when D3D12_RESOURCE_DESC::Dimension is
		//not D3D12_RESOURCE_DIMENSION_BUFFER and neither D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		//nor D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL are set in D3D12_RESOURCE_DESC::Flags.
		D3D12_CLEAR_VALUE* pClearValue = (texDesc.Dimension!=D3D12_RESOURCE_DIMENSION_BUFFER)&&
			((texDesc.Flags!= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) && (texDesc.Flags != D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			?NULL:&clearVal;

		D3D12_RESOURCE_ALLOCATION_INFO info = pDevice->GetResourceAllocationInfo(0, 1, &texDesc);

		HRESULT hr = pDevice->CreatePlacedResource(m_pTexHeap.Get(), m_curTexHeapOffset, &texDesc,
			state, pClearValue, IID_PPV_ARGS(pTex.GetAddressOf()));
		assert(SUCCEEDED(hr));
		m_curTexHeapOffset += CUSTUM_ALIGN(info.SizeInBytes,_64KB);
	}

	void HeapManager::UploadGraphicsCb(const UINT key, const UINT rootParamIdx,
		ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->SetGraphicsRootDescriptorTable(rootParamIdx, m_gpuHandleHash[key]);
		return;
	}
	void HeapManager::UploadComputeCb(const UINT key, const UINT rootParamIdx, ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->SetComputeRootDescriptorTable(rootParamIdx, m_gpuHandleHash[key]);
	}

	void HeapManager::SetDescriptorHeaps(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		ID3D12DescriptorHeap* ppHeaps[2] = { *(g_pHeapManager->GetSamplerHeap()),
										*(g_pHeapManager->GetCbvSrvUavHeap()) };
		pCmdList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
	}

	void HeapManager::CopyDataToCb(const UINT key, const UINT64 dataSize, const void* pSrc)
	{
		memcpy(m_cbRawPtrHash[key], pSrc, dataSize);
	}
	void HeapManager::CopyDataToCbWithOffset(const UINT key, const UINT64 dataSize, const UINT offset, const void* pSrc)
	{
		memcpy(m_cbRawPtrHash[key]+offset, pSrc, dataSize);
	}
	void HeapManager::CreateUploadBuffer(ComPtr<ID3D12Resource>& pUploadCb, const UINT64 uploadPitch, const UINT64 uploadSize)
	{

		ComPtr<ID3D12Device> pDevice = D3D12::GetDevice();

		if ((pUploadCb.Get()) != nullptr)
		{
			(pUploadCb)->Release();
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

		hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &uploadCbDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(pUploadCb.GetAddressOf()));
		assert(SUCCEEDED(hr));

	}
	HeapManager::HeapManager(ComPtr<ID3D12Device> pDevice)
	{

		m_curVbHeapOffset = 0;
		m_curIbHeapOffset = 0;
		m_curCbHeapOffset = 0;
		m_curRbHeapOffset = 0;
		m_curTexHeapOffset = 0;

		m_rbIdx = 0;
		for (int i = 0; i < _HEAP_BLOCK_COUNT; ++i)
		{
			m_curVbBlockOffset[i] = 0;
			m_curIbBlockOffset[i] = 0;
			m_curCbBlockOffset[i] = 0;
		}

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

		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(m_pVbHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pVbHeap);

		heapDesc.SizeInBytes = _IB_HEAP_SIZE;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(m_pIbHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pIbHeap);

		heapDesc.SizeInBytes = _CB_HEAP_SIZE;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(m_pCbHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pCbHeap);

		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapDesc.SizeInBytes = _TEX_HEAP_SIZE;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(m_pTexHeap.GetAddressOf()));
		assert(SUCCEEDED(hr)); 
		SET_PRIVATE_DATA(m_pTexHeap);

		heapDesc.SizeInBytes = _64KB*_WORKER_THREAD_COUNT;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(m_pQueryResultHeap.GetAddressOf()));
		assert(SUCCEEDED(hr)); 
		SET_PRIVATE_DATA(m_pQueryResultHeap);


		heapDesc.Properties.Type = D3D12_HEAP_TYPE_READBACK;
		heapDesc.SizeInBytes = _RB_BLOCK_COUNT * _64KB;
		hr = pDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(m_pRbHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pRbHeap);


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
			hr = pDevice->CreatePlacedResource(m_pVbHeap.Get(), offset,
				&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(m_pVbBlock[i].GetAddressOf()));
			assert(SUCCEEDED(hr));
			SET_PRIVATE_DATA(m_pVbBlock[i]);
			hr = m_pVbBlock[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pCurVbBlockMappedPtr[i]));
			assert(SUCCEEDED(hr));

			bufferDesc.Width = (_IB_HEAP_SIZE) / _HEAP_BLOCK_COUNT;
			hr = pDevice->CreatePlacedResource(m_pIbHeap.Get(), i * bufferDesc.Width,
				&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS(m_pIbBlock[i].GetAddressOf()));
			assert(SUCCEEDED(hr));
			SET_PRIVATE_DATA(m_pIbBlock[i]);

			hr = m_pIbBlock[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pCurIbBlockMappedPtr[i]));
			assert(SUCCEEDED(hr));

			bufferDesc.Width = (_CB_HEAP_SIZE) / _HEAP_BLOCK_COUNT;
			hr = pDevice->CreatePlacedResource(m_pCbHeap.Get(), i* bufferDesc.Width,
				&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(m_pCbBlock[i].GetAddressOf()));
			assert(SUCCEEDED(hr));
			SET_PRIVATE_DATA(m_pCbBlock[i]);

			hr = m_pCbBlock[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_pCurCbBlockMappedPtr[i]));
			assert(SUCCEEDED(hr));
		}


		{
			bufferDesc.Width = _64KB;
			for (UINT i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				hr = pDevice->CreatePlacedResource(m_pQueryResultHeap.Get(), i*_64KB,
					&bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr, IID_PPV_ARGS(&m_pQueryResultBlock[i]));
				assert(SUCCEEDED(hr));
				SET_PRIVATE_DATA(m_pQueryResultBlock[i]);

				
			}

			
		}
		//Gen QueryHeap
		{
			D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
			queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
			queryHeapDesc.Count = _4KB;

			for (int i = 0; i < _WORKER_THREAD_COUNT; ++i)
			{
				hr = pDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(m_pQueryHeap[i].GetAddressOf()));
				assert(SUCCEEDED(hr));
				SET_PRIVATE_DATA(m_pQueryHeap[i]);
			}

		}



		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = _RTV_DSV_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_pRtvHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pRtvHeap);


		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = _RTV_DSV_COUNT;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_pDsvHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pDsvHeap);

		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = _CBV_SRV_COUNT;
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(m_pCbvSrvUavHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pCbvSrvUavHeap);

		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = _SAMPLER_COUNT;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(m_pSamplerHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pSamplerHeap);

		D3D12_DESCRIPTOR_HEAP_DESC dstCbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = _CBV_SRV_COUNT;
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(m_pDstCbvSrvHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pDstCbvSrvHeap);

		D3D12_DESCRIPTOR_HEAP_DESC dstSamplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = _SAMPLER_COUNT;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(m_pDstSamplerHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
		SET_PRIVATE_DATA(m_pDstSamplerHeap);

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
}