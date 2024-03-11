#include "MatrixBuffer12.h"
#include "DescriptorHeapManager.h"
namespace wilson
{
	MatBuffer12::MatBuffer12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandlist, DescriptorHeapManager* pDescriptorHeapManager,
		XMMATRIX* pViewMat, XMMATRIX* pProjMat)
	{
		m_pMat12Cb = nullptr;
		m_pProjMat12Cb = nullptr;
		m_pViewMat12Cb = nullptr;
		m_pCombinedMat12Cb = nullptr;

		m_pMatricesCbBegin = nullptr;
		m_pProjMatCbBegin = nullptr;
		m_pViewMatCbBegin = nullptr;
		m_pCombinedMatCbBegin = nullptr;

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
			assert(SUCCEEDED(hr));
			m_pMat12Cb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("MatBuffer::m_pMat12Cb") - 1, "MatBuffer::m_pMat12Cb");

			D3D12_RANGE readRange = { 0, };
			hr = m_pMat12Cb->Map(0, &readRange, reinterpret_cast<void**>(&m_pMatricesCbBegin));
			assert(SUCCEEDED(hr));
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
			assert(SUCCEEDED(hr));
			m_pProjMat12Cb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("MatBuffer::m_pProjMat12Cb") - 1, "MatBuffer::m_pProjMat12Cb");

			D3D12_RANGE readRange = { 0, };
			hr = m_pProjMat12Cb->Map(0, &readRange, reinterpret_cast<void**>(&m_pProjMatCbBegin));
			assert(SUCCEEDED(hr));

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
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pViewMat12Cb));
			assert(SUCCEEDED(hr));
			m_pViewMat12Cb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("MatBuffer::m_pViewMat12Cb") - 1, "MatBuffer::m_pViewMat12Cb");

			D3D12_RANGE readRange = { 0, };
			hr = m_pViewMat12Cb->Map(0, &readRange, reinterpret_cast<void**>(&m_pViewMatCbBegin));
			assert(SUCCEEDED(hr));

			UINT constantBufferSize = sizeof(DirectX::XMMATRIX);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
			cbvDesc.BufferLocation = m_pViewMat12Cb->GetGPUVirtualAddress();
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_viewMatCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}

		{
			D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
			cbufferDesc.Width = _64KB_ALIGN(sizeof(DirectX::XMMATRIX));

			hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pCombinedMat12Cb));
			assert(SUCCEEDED(hr));
			m_pCombinedMat12Cb->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("MatBuffer::m_pCombinedMat12Cb") - 1, "MatBuffer::m_pCombinedMat12Cb");

			D3D12_RANGE readRange = { 0, };
			hr = m_pCombinedMat12Cb->Map(0, &readRange, reinterpret_cast<void**>(&m_pCombinedMatCbBegin));
			assert(SUCCEEDED(hr));

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

		if (m_pViewMat12Cb != nullptr)
		{
			m_pViewMat12Cb->Release();
			m_pViewMat12Cb = nullptr;
		}

		if (m_pCombinedMat12Cb != nullptr)
		{
			m_pCombinedMat12Cb->Release();
			m_pCombinedMat12Cb = nullptr;
		}
	}

	void MatBuffer12::UploadMatBuffer(ID3D12GraphicsCommandList* pCommandlist)
	{
		XMMATRIX invWVMat = XMMatrixMultiply(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_viewMat));
		invWVMat = XMMatrixInverse(nullptr, invWVMat);
		invWVMat = XMMatrixTranspose(invWVMat);

		MatrixBuffer matBuffer;
		matBuffer.m_worldMat = m_worldMat;
		matBuffer.m_viewMat = m_viewMat;
		matBuffer.m_invWorldMat = m_invWorldMat;
		matBuffer.m_invWVMat = invWVMat;
		matBuffer.m_wvpMat = m_wvpMat;


		memcpy(m_pMatricesCbBegin, &matBuffer, sizeof(matBuffer));
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_matCBV);
		return;
	}

	void MatBuffer12::UploadProjMat(ID3D12GraphicsCommandList* pCommandlist, bool bSSAO)
	{	
		memcpy(m_pProjMatCbBegin, &m_projMat, sizeof(XMMATRIX));
		if (bSSAO)
		{
			pCommandlist->SetComputeRootDescriptorTable(eSsao_eCsProj,  m_projMatCBV);
		}
		else
		{
			pCommandlist->SetGraphicsRootDescriptorTable( ePbrLight_ePsProjMat, m_projMatCBV);
		}
		return;
	}
	void MatBuffer12::UploadViewMat(ID3D12GraphicsCommandList* pCommandlist)
	{
		memcpy(m_pViewMatCbBegin, &m_viewMat, sizeof(XMMATRIX));
		pCommandlist->SetGraphicsRootDescriptorTable(ePbrLight_ePsViewMat, m_viewMatCBV);
		
	}
	void MatBuffer12::UploadCombinedMat(ID3D12GraphicsCommandList* pCommandlist, bool bSpotShadowPass)
	{
		const void* pSrc = bSpotShadowPass ? &m_wvpLitMat : &m_wvpMat;
		memcpy(m_pCombinedMatCbBegin, pSrc, sizeof(XMMATRIX));
		//Zpass, Skybox, SpotShadow, OutlinerTest Pass¿¡ ÀÌ¿ëµÊ 
		pCommandlist->SetGraphicsRootDescriptorTable(0, m_combinedMatCBV);
		return;
	}
	void MatBuffer12::UpdateCombinedMat(bool bSpotShadowPass)
	{
		if (bSpotShadowPass)
		{
			m_wvpLitMat = XMMatrixMultiplyTranspose(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_lightSpaceMat));
		}
		else
		{
			XMMATRIX wv = XMMatrixMultiply(XMMatrixTranspose(m_worldMat), XMMatrixTranspose(m_viewMat));
			m_wvpMat = XMMatrixMultiplyTranspose(wv, XMMatrixTranspose(m_projMat));
		}
		
	}
}
