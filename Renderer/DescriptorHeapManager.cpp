#include "DescriptorHeapManager.h"
namespace wilson
{
	void DescriptorHeapManager::IncreaseRtvHandleOffset()
	{
		m_curRtvHandle.ptr += m_rtvHandleSize;
	}
	void DescriptorHeapManager::IncreaseDsvHandleOffset()
	{
		m_curDsvHandle.ptr += m_dsvHandleSize;
	}
	void DescriptorHeapManager::IncreaseCbvSrvHandleOffset()
	{
		m_curCbvSrvCpuHandle.ptr += m_cbvSrvHanldeSize;
		m_curCbvSrvGpuHandle.ptr += m_cbvSrvHanldeSize;
	}
	void DescriptorHeapManager::IncreaseSamplerHandleOffset()
	{
		m_curSamplerCpuHandle.ptr += m_samplerHandleSize;
		m_curSamplerGpuHandle.ptr += m_samplerHandleSize;
	}

	DescriptorHeapManager::DescriptorHeapManager(ID3D12Device* pDevice)
	{
		m_pRtvHeap = nullptr;
		m_pDsvHeap = nullptr;
		m_pSamplerHeap = nullptr;
		m_pCbvSrvHeap = nullptr;
		m_pDstSamplerHeap = nullptr;
		m_pDstCbvSrvHeap = nullptr;


		HRESULT hr;
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = _RTV_DSV_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap));
		if (FAILED(hr))
		{
			OutputDebugStringW(L"DescriptorHeapManager::m_pRtvHeap::CreateDescriptorHeap()Failed");
		}
		m_pRtvHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("DescriptorHeapManager::m_pRtvHeap") - 1, "DescriptorHeapManager::m_pRtvHeap");


		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = _RTV_DSV_COUNT;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap));
		if (FAILED(hr))
		{
			OutputDebugStringW(L"DescriptorHeapManager::m_pDsvHeap::CreateDescriptorHeap()Failed");
		}
		m_pDsvHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("DescriptorHeapManager::m_pDsvHeap") - 1, "DescriptorHeapManager::m_pDsvHeap");

		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = _CBV_SRV_COUNT;
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_pCbvSrvHeap));
		if (FAILED(hr))
		{
			OutputDebugStringW(L"DescriptorHeapManager::m_pCbvSrvHeap::CreateDescriptorHeap()Failed");
		}
		m_pCbvSrvHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("DescriptorHeapManager::m_pCbvSrvHeap") - 1, "DescriptorHeapManager::m_pCbvSrvHeap");

		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = _SAMPLER_COUNT;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_pSamplerHeap));
		if (FAILED(hr))
		{
			OutputDebugStringW(L"DescriptorHeapManager::m_pSamplerHeap::CreateDescriptorHeap()Failed");
		}
		m_pSamplerHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("DescriptorHeapManager::m_pSamplerHeap") - 1, "DescriptorHeapManager::m_pSamplerHeap");

		D3D12_DESCRIPTOR_HEAP_DESC dstCbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = _CBV_SRV_COUNT;
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_pDstCbvSrvHeap));
		if (FAILED(hr))
		{
			OutputDebugStringW(L"DescriptorHeapManager::m_pDstCbvSrvHeap::CreateDescriptorHeap()Failed");
		}
		m_pDstCbvSrvHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("DescriptorHeapManager::m_pDstCbvSrvHeap") - 1, "DescriptorHeapManager::m_pDstCbvSrvHeap");

		D3D12_DESCRIPTOR_HEAP_DESC dstSamplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = _SAMPLER_COUNT;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_pDstSamplerHeap));
		if (FAILED(hr))
		{
			OutputDebugStringW(L"DescriptorHeapManager::m_pDstSamplerHeap::CreateDescriptorHeap()Failed");
		}
		m_pDstSamplerHeap->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("DescriptorHeapManager::m_pDstSamplerHeap") - 1, "DescriptorHeapManager::m_pDstSamplerHeap");

		m_curRtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
		m_curDsvHandle = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
		m_curCbvSrvCpuHandle = m_pCbvSrvHeap->GetCPUDescriptorHandleForHeapStart();
		m_curSamplerCpuHandle = m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart();
		m_curCbvSrvGpuHandle = m_pCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
		m_curSamplerGpuHandle = m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart();
		m_curDstCbvSrvCpuHandle = m_pDstCbvSrvHeap->GetCPUDescriptorHandleForHeapStart();
		m_curDstSamplerCpuHandle = m_pDstSamplerHeap->GetCPUDescriptorHandleForHeapStart();
		m_curDstCbvSrvGpuHandle = m_pDstCbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
		m_curDstSamplerGpuHandle = m_pDstSamplerHeap->GetGPUDescriptorHandleForHeapStart();

		m_rtvHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_dsvHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_cbvSrvHanldeSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_samplerHandleSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}
	DescriptorHeapManager::~DescriptorHeapManager()
	{
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

		if (m_pCbvSrvHeap != nullptr)
		{
			m_pCbvSrvHeap->Release();
			m_pCbvSrvHeap = nullptr;
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