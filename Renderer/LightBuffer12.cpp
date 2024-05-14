#include "LightBuffer12.h"
#include "HeapManager.h"
namespace wilson
{
	void LightBuffer12::UpdateDirLightMatrices(ID3D12GraphicsCommandList* const pCommandlist)
	{

		UINT offset = 0;
		UINT len = sizeof(DirectX::XMMATRIX)*_CASCADE_LEVELS;
		UINT nLights = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			std::vector<DirectX::XMMATRIX> cascadeMat = m_pDirLights[i]->GetLightSpaceMat();
			memcpy(m_pDirLitMatricesCbBegin + offset, &cascadeMat[0], len);
			offset += len;
		}
		offset = len * _MAX_DIR_LIGHTS;
		memcpy(m_pDirLitMatricesCbBegin + offset, &nLights, sizeof(UINT));
	
		pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psDirLitMat), m_dirLitMatricesCbv);
		return;
	}

	void LightBuffer12::UpdateSpotLightMatrices(ID3D12GraphicsCommandList* const pCommandlist)
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
		pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psSpotLitMat), m_spotLitMatricesCbv);
		return;
	}
	
	void LightBuffer12::UpdateLightBuffer(ID3D12GraphicsCommandList* const pCommandlist)
	{
		UINT dirLightOffset = 0;
		UINT len = sizeof(DirLightProperty);
		UINT nLights = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			memcpy(m_pLightPropertyCbBegin + dirLightOffset, m_pDirLights[i]->GetProperty(), len);
			dirLightOffset += len;
		}
		dirLightOffset = len * _MAX_DIR_LIGHTS;
		memcpy(m_pLightPropertyCbBegin + dirLightOffset, &nLights, sizeof(UINT));
		dirLightOffset += 16;//hlsl 16bytes align;


		UINT pntLightOffset = dirLightOffset;
		len = sizeof(CubeLightProperty);
		nLights = m_pCubeLights.size();
		for (int i = 0; i < m_pCubeLights.size(); ++i)
		{
			memcpy(m_pLightPropertyCbBegin + pntLightOffset, m_pCubeLights[i]->GetProperty(), len);
			pntLightOffset += len;
		}
		pntLightOffset = dirLightOffset+len * _MAX_PNT_LIGHTS;
		memcpy(m_pLightPropertyCbBegin + pntLightOffset, &nLights, sizeof(UINT));
		pntLightOffset += 16;
		
		
		UINT sptLightOffset = pntLightOffset;
		len = sizeof(SpotLightProperty);
		nLights = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			memcpy(m_pLightPropertyCbBegin + sptLightOffset, m_pSpotLights[i]->GetProperty(), len);
			sptLightOffset += len;
		}

		sptLightOffset = pntLightOffset+len * _MAX_SPT_LIGHTS;
		memcpy(m_pLightPropertyCbBegin + sptLightOffset, &nLights, sizeof(UINT));

		pCommandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ePbrLightRP::psLight), m_lightPropertyCbv);
		return;


	}
	
	LightBuffer12::LightBuffer12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandList, HeapManager* const pHeapManager)
	{
		m_pLightPropertyCbBegin = nullptr;
		m_pDirLitMatricesCbBegin = nullptr;
		m_pSpotLitMatricesCbBegin = nullptr;

		m_pDirLights.reserve(_MAX_DIR_LIGHTS);
		m_pCubeLights.reserve(_MAX_PNT_LIGHTS);
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
				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = pHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = pHeapManager->GetCurCbvSrvGpuHandle();
				pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvCpuHandle);
				pHeapManager->IncreaseCbvSrvHandleOffset();
			}
		}
		
		UINT cbSize = sizeof(LightBufferProperty);
		m_pLightPropertyCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_lightPropertyCbv = pHeapManager->GetCbv(cbSize, pDevice);
		
		cbSize = sizeof(DirLightMatrices);
		m_pDirLitMatricesCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_dirLitMatricesCbv = pHeapManager->GetCbv(cbSize, pDevice);


		
		cbSize = sizeof(SpotLightMatrices);
		m_pSpotLitMatricesCbBegin = pHeapManager->GetCbMappedPtr(cbSize);
		m_spotLitMatricesCbv = pHeapManager->GetCbv(cbSize, pDevice);
		

	}

	LightBuffer12::~LightBuffer12()
	{

	}
}