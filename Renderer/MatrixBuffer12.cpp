#include "MatrixBuffer12.h"
#include "DescriptorHeapManager.h"
namespace wilson
{

	MatBuffer12::MatBuffer12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, DescriptorHeapManager* pDescriptorHeapManager,
		XMMATRIX* pViewMat, XMMATRIX* pProjMat)
	{
		m_pMat12Cb = nullptr;
		m_pProjMat12Cb = nullptr;
		m_pCombinedMat12Cb = nullptr;

		m_worldMat = XMMatrixIdentity();
		m_invWorldMat = m_worldMat;
		m_lightSpaceMat = m_worldMat;
		m_extraMat = m_worldMat;

		m_viewMat = *pViewMat;
		m_projMat = *pProjMat;


		D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
		D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC cbufferDesc = {};
		cbufferDesc.Width = _64KB_ALIGN(sizeof(MatrixBuffer));
		cbufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		cbufferDesc.Alignment = 0;
		cbufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		cbufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		cbufferDesc.Height = 1;
		cbufferDesc.DepthOrArraySize = 1;
		cbufferDesc.MipLevels = 1;
		cbufferDesc.SampleDesc.Count = 1;
		cbufferDesc.SampleDesc.Quality = 0;

		HRESULT hr;
		{
			hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pMat12Cb));
			if (FAILED(hr))
			{
				OutputDebugStringA("MatBuffer::m_pMat12Cb::CreateBufferFailed");
			}
			m_pMat12Cb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("MatBuffer::m_pMat12Cb") - 1, "MatBuffer::m_pMat12Cb");

			UINT constantBufferSize = sizeof(MatrixBuffer);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
			cbvDesc.BufferLocation = m_pMat12Cb->GetGPUVirtualAddress();
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_matCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}
		
		{
			D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
			cbufferDesc.Width = _64KB_ALIGN(sizeof(DirectX::XMMATRIX));

			hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pProjMat12Cb));
			if (FAILED(hr))
			{
				OutputDebugStringA("MatBuffer::m_pProjMat12Cb::CreateBufferFailed");
			}
			m_pProjMat12Cb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("MatBuffer::m_pProjMat12Cb") - 1, "MatBuffer::m_pProjMat12Cb");

			UINT constantBufferSize = sizeof(DirectX::XMMATRIX);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
			cbvDesc.BufferLocation = m_pProjMat12Cb->GetGPUVirtualAddress();
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_projMatCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}

		{
			D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
			cbufferDesc.Width = _64KB_ALIGN(sizeof(DirectX::XMMATRIX));

			hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pCombinedMat12Cb));
			if (FAILED(hr))
			{
				OutputDebugStringA("MatBuffer::m_pCombinedMat12Cb::CreateBufferFailed");
			}
			m_pCombinedMat12Cb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("MatBuffer::m_pCombinedMat12Cb") - 1, "MatBuffer::m_pCombinedMat12Cb");

			UINT constantBufferSize = sizeof(DirectX::XMMATRIX);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
			cbvDesc.BufferLocation = m_pCombinedMat12Cb->GetGPUVirtualAddress();
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_combinedMatCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}
	}

	MatBuffer12::~MatBuffer12()
	{

		if (m_pMat12Cb != nullptr)
		{
			m_pMat12Cb->Release();
			m_pMat12Cb = nullptr;
		}

		if (m_pProjMat12Cb != nullptr)
		{
			m_pProjMat12Cb->Release();
			m_pProjMat12Cb = nullptr;
		}

		if (m_pCombinedMat12Cb != nullptr)
		{
			m_pCombinedMat12Cb->Release();
			m_pCombinedMat12Cb = nullptr;
		}
	}

	void MatBuffer12::UploadMatBuffer(ID3D12GraphicsCommandList* pCommandlist, bool bSpotShadowPass)
	{
		HRESULT hr;
		UINT8* pMatBuffer;
		D3D12_RANGE readRange = { 0, };
		hr = m_pMat12Cb->Map(0, &readRange, reinterpret_cast<void**>(&pMatBuffer));
		if (FAILED(hr))
		{
			OutputDebugStringA("MatBuffer::m_pMat12Cb::Map()Failed");
		}
		MatrixBuffer matBuffer;
		matBuffer.m_worldMat = m_worldMat;
		matBuffer.m_viewMat = m_viewMat;
		matBuffer.m_projMat = m_projMat;
		matBuffer.m_extraMat = bSpotShadowPass ? m_lightSpaceMat : m_invWorldMat;


		memcpy(pMatBuffer, &matBuffer, sizeof(matBuffer));
		m_pMat12Cb->Unmap(0, nullptr);

		pCommandlist->SetGraphicsRootDescriptorTable(0, m_matCBV);
		return;
	}

	void MatBuffer12::UploadProjMat(ID3D12GraphicsCommandList* pCommandlist)
	{
		HRESULT hr;
		UINT8* pMatBuffer;
		D3D12_RANGE readRange = { 0, };
		hr = m_pProjMat12Cb->Map(0, &readRange, reinterpret_cast<void**>(&pMatBuffer));
		if (FAILED(hr))
		{
			OutputDebugStringA("MatBuffer::m_pMat12Cb::Map()Failed");
		}

		memcpy(pMatBuffer, &m_projMat, sizeof(XMMATRIX));
		m_pProjMat12Cb->Unmap(0, nullptr);

		pCommandlist->SetGraphicsRootDescriptorTable(eSsaoRP::eSsao_ePsProj, m_projMatCBV);
		return;
	}
	void MatBuffer12::UploadCombinedMat(ID3D12GraphicsCommandList* pCommandlist, bool bSpotShadowPass)
	{
		HRESULT hr;
		UINT8* pMatBuffer;
		D3D12_RANGE readRange = { 0, };
		hr = m_pCombinedMat12Cb->Map(0, &readRange, reinterpret_cast<void**>(&pMatBuffer));
		if (FAILED(hr))
		{
			OutputDebugStringA("MatBuffer:: m_pCombinedMat12Cb::Map()Failed");
		}
		const void* pSrc = bSpotShadowPass ? &m_wvpLitMat : &m_wvpMat;
		memcpy(pMatBuffer, pSrc, sizeof(XMMATRIX));
		m_pCombinedMat12Cb->Unmap(0, nullptr);

		//Skybox, SpotShadow, OutlinerTest Pass¿¡ ÀÌ¿ëµÊ 
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_combinedMatCBV);
		return;
	}
	void MatBuffer12::UpdateCombinedMat(bool bSpotShadowPass)
	{
		if (bSpotShadowPass)
		{
			m_wvpLitMat = XMMatrixMultiplyTranspose(m_worldMat, m_lightSpaceMat);
		}
		else
		{
			XMMATRIX wv = XMMatrixMultiply(m_worldMat, XMMatrixTranspose(m_viewMat));
			m_wvpMat = XMMatrixMultiplyTranspose(wv, XMMatrixTranspose(m_projMat));
		}
		
	}
}
