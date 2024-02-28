#include "LightBuffer12.h"
#include "DescriptorHeapManager.h"
namespace wilson
{
	void LightBuffer12::UpdateDirLightMatrices(ID3D12GraphicsCommandList* pCommandlist)
	{

		UINT offset = 0;
		UINT len = sizeof(DirectX::XMMATRIX)*_CASCADE_LEVELS;
		UINT numOfLights = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			std::vector<DirectX::XMMATRIX> cascadeMat = m_pDirLights[i]->GetLightSpaceMat();
			memcpy(m_pDirLitMatricesCbBegin + offset, &cascadeMat[0], len);
			offset += len;
		}
		offset = len * _MAX_DIR_LIGHTS;
		memcpy(m_pDirLitMatricesCbBegin + offset, &numOfLights, sizeof(UINT));
	
		pCommandlist->SetGraphicsRootDescriptorTable(ePbrLightRP::ePbrLight_ePsDirLitMat, m_dirLitMatrices12BufferCBV);
		return;
	}

	void LightBuffer12::UpdateSpotLightMatrices(ID3D12GraphicsCommandList* pCommandlist)
	{
		UINT offset = 0;
		UINT len = sizeof(DirectX::XMMATRIX);
		UINT numOfLights = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			memcpy(m_pSpotLitMatricesCbBegin + offset, m_pSpotLights[i]->GetLightSpaceMat(), len);
			offset += len;
		}
		offset = len * _MAX_SPT_LIGHTS;
		memcpy(m_pSpotLitMatricesCbBegin + offset, &numOfLights, sizeof(UINT));
		pCommandlist->SetGraphicsRootDescriptorTable(ePbrLightRP::ePbrLight_ePsSpotLitMat, m_spotLitMatrices12BufferCBV);
		return;
	}
	
	void LightBuffer12::UpdateLightBuffer(ID3D12GraphicsCommandList* pCommandlist)
	{
		UINT dirLightOffset = 0;
		UINT len = sizeof(DirLightProperty);
		UINT numOfLights = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			memcpy(m_pLightPropertyCbBegin + dirLightOffset, m_pDirLights[i]->GetProperty(), len);
			dirLightOffset += len;
		}
		dirLightOffset = len * _MAX_DIR_LIGHTS;
		memcpy(m_pLightPropertyCbBegin + dirLightOffset, &numOfLights, sizeof(UINT));
		dirLightOffset += 16;//hlsl 16bytes align;


		UINT pntLightOffset = dirLightOffset;
		len = sizeof(PointLightProperty);
		numOfLights = m_pPointLights.size();
		for (int i = 0; i < m_pPointLights.size(); ++i)
		{
			memcpy(m_pLightPropertyCbBegin + pntLightOffset, m_pPointLights[i]->GetProperty(), len);
			pntLightOffset += len;
		}
		pntLightOffset = dirLightOffset+len * _MAX_PNT_LIGHTS;
		memcpy(m_pLightPropertyCbBegin + pntLightOffset, &numOfLights, sizeof(UINT));
		pntLightOffset += 16;
		
		
		UINT sptLightOffset = pntLightOffset;
		len = sizeof(SpotLightProperty);
		numOfLights = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			memcpy(m_pLightPropertyCbBegin + sptLightOffset, m_pSpotLights[i]->GetProperty(), len);
			sptLightOffset += len;
		}

		sptLightOffset = pntLightOffset+len * _MAX_SPT_LIGHTS;
		memcpy(m_pLightPropertyCbBegin + sptLightOffset, &numOfLights, sizeof(UINT));

		pCommandlist->SetGraphicsRootDescriptorTable(ePbrLightRP::ePbrLight_ePsLight, m_lightPropertyBufferCBV);
		return;


	}
	
	LightBuffer12::LightBuffer12(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, DescriptorHeapManager* pDescriptorHeapManager)
	{
	
		m_pLightProperty12Buffer = nullptr;
		m_pDirLitMatrices12Buffer = nullptr;
		m_pSpotLitMatrices12Buffer = nullptr;

		m_pLightPropertyCbBegin = nullptr;
		m_pDirLitMatricesCbBegin = nullptr;
		m_pSpotLitMatricesCbBegin = nullptr;

		m_pDirLights.reserve(_MAX_DIR_LIGHTS);
		m_pPointLights.reserve(_MAX_PNT_LIGHTS);
		m_pSpotLights.reserve(_MAX_SPT_LIGHTS);


		//Gen Null Srvs
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
			nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			nullSrvDesc.Texture2D.MipLevels = 1;
			nullSrvDesc.Texture2D.MostDetailedMip = 0;
			nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;


			for (int i = 0; i < _MAX_DIR_LIGHTS + _MAX_PNT_LIGHTS + _MAX_SPT_LIGHTS; ++i)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvCpuHandle);
				pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
		}
		
		HRESULT hr;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC cbufferDesc = {};
		cbufferDesc.Width = _64KB_ALIGN(sizeof(LightBufferProperty));
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


		{
			D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

			hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pLightProperty12Buffer));
			assert(SUCCEEDED(hr));
			m_pLightProperty12Buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("LightBuffer::m_pLightPropertyBuffer") - 1, "LightBuffer::m_pLightPropertyBuffer");

			D3D12_RANGE readRange = { 0, };
			hr = m_pLightProperty12Buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pLightPropertyCbBegin));
			assert(SUCCEEDED(hr));

			UINT constantBufferSize = sizeof(LightBufferProperty);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
			cbvDesc.BufferLocation = m_pLightProperty12Buffer->GetGPUVirtualAddress();
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_lightPropertyBufferCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		}
		

		{
			D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

			cbufferDesc.Width = _64KB_ALIGN(sizeof(DirLightMatrices));

			hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pDirLitMatrices12Buffer));
			assert(SUCCEEDED(hr));
			m_pDirLitMatrices12Buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("LightBuffer::m_pDirLitMatrices12Buffer") - 1, "LightBuffer::m_pDirLitMatrices12Buffer");

			D3D12_RANGE readRange = { 0, };
			hr = m_pDirLitMatrices12Buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pDirLitMatricesCbBegin));
			assert(SUCCEEDED(hr));

			UINT constantBufferSize = sizeof(DirLightMatrices);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
			cbvDesc.BufferLocation = m_pDirLitMatrices12Buffer->GetGPUVirtualAddress();
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_dirLitMatrices12BufferCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}

		{
			D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

			cbufferDesc.Width = _64KB_ALIGN(sizeof(SpotLightMatrices));


			hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&cbufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_pSpotLitMatrices12Buffer));
			assert(SUCCEEDED(hr));
			m_pSpotLitMatrices12Buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("LightBuffer::m_pSpotLitMatrices12Buffer") - 1, "LightBuffer::m_pSpotLitMatrices12Buffer");

			D3D12_RANGE readRange = { 0, };
			hr = m_pSpotLitMatrices12Buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pSpotLitMatricesCbBegin));
			assert(SUCCEEDED(hr));

			UINT constantBufferSize = sizeof(SpotLightMatrices);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.SizeInBytes = _CBV_ALIGN(constantBufferSize);
			cbvDesc.BufferLocation = m_pSpotLitMatrices12Buffer->GetGPUVirtualAddress();
			pDevice->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
			m_spotLitMatrices12BufferCBV = cbvSrvGpuHandle;
			pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();

		}

	}

	LightBuffer12::~LightBuffer12()
	{

		if (m_pLightProperty12Buffer != nullptr)
		{
			m_pLightProperty12Buffer->Release();
			m_pLightProperty12Buffer = nullptr;
		}

		if (m_pDirLitMatrices12Buffer != nullptr)
		{
			m_pDirLitMatrices12Buffer->Release();
			m_pDirLitMatrices12Buffer = nullptr;
		}

		if (m_pSpotLitMatrices12Buffer != nullptr)
		{
			m_pSpotLitMatrices12Buffer->Release();
			m_pSpotLitMatrices12Buffer = nullptr;
		}

		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			if (m_pDirLights[i] != nullptr)
			{
				delete m_pDirLights[i];
			}
		}
		for (int i = 0; i < m_pPointLights.size(); ++i)
		{
			if (m_pPointLights[i] != nullptr)
			{
				delete m_pPointLights[i];
			}
		}
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			if (m_pSpotLights[i] != nullptr)
			{
				delete m_pSpotLights[i];
			}
		}
	}
}