#include "LightBuffer12.h"
#include "HeapManager.h"
namespace wilson
{

	LightBuffer12::LightBuffer12(ComPtr<ID3D12Device> pDevice)
	{
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
				D3D12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle = g_pHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle = g_pHeapManager->GetCurCbvSrvGpuHandle();
				pDevice->CreateShaderResourceView(nullptr, &nullSrvDesc, cbvSrvCpuHandle);
				g_pHeapManager->IncreaseCbvSrvHandleOffset();
			}
		}

		m_lightPropertyKey		= g_pHeapManager->AllocateCb(sizeof(LightBufferProperty));
		m_dirLitMatricesKey		= g_pHeapManager->AllocateCb(sizeof(DirLightMatrices));
		m_spotLitMatricesKey	= g_pHeapManager->AllocateCb(sizeof(SpotLightMatrices));
	}

	vector<shared_ptr<DirectionalLight12>>& LightBuffer12::GetDirLights()
	{
		return m_pDirLights;
	}
	vector<shared_ptr<CubeLight12>>& LightBuffer12::GetCubeLights()
	{
		return m_pCubeLights;
	}
	vector<shared_ptr<SpotLight12>>& LightBuffer12::GetSpotLights()
	{
		return m_pSpotLights;
	}

	UINT LightBuffer12::GetCubeLightsSize() 
	{
		return m_pCubeLights.size();
	}

	UINT LightBuffer12::GetDirLightsSize() 
	{
		return m_pDirLights.size();
	}

	UINT LightBuffer12::GetSpotLightsSize() 
	{
		return m_pSpotLights.size();
	}

	void LightBuffer12::PushCubeLight(shared_ptr<CubeLight12> pCubeLight)
	{
		m_pCubeLights.push_back(pCubeLight);
	}
	void LightBuffer12::PushDirLight(shared_ptr<DirectionalLight12> pDirLight)
	{
		m_pDirLights.push_back(pDirLight);
	}
	void LightBuffer12::PushSpotLight(shared_ptr<SpotLight12>pSpotLight)
	{
		m_pSpotLights.push_back(pSpotLight);
	}

	void LightBuffer12::Update()
	{
		BOOL dir = m_dirty & (1 << 0);
		BOOL cube = m_dirty & (1 << 1);
		BOOL spot = m_dirty & (1 << 2);


		UpdateLightProperty();
		if (dir)
		{
			UpdateDirLightMatrices();
		}

		if (spot)
		{

			UpdateSpotLightMatrices();
		}
	}

	void LightBuffer12::UpdateDirLightMatrices()
	{

		UINT offset = 0;
		UINT len = sizeof(XMMATRIX)*_CASCADE_LEVELS;
		UINT nLights = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			vector<XMMATRIX> cascadeMat = m_pDirLights[i]->GetDirLightMatrix();
			g_pHeapManager->CopyDataToCbWithOffset(m_dirLitMatricesKey, len, offset, &cascadeMat[0]);
			offset += len;
		}
		offset = len * _MAX_DIR_LIGHTS; 
		g_pHeapManager->CopyDataToCbWithOffset(m_dirLitMatricesKey, sizeof(UINT), offset, &nLights);
	
		return;
	}
	void LightBuffer12::UpdateSpotLightMatrices()
	{
		UINT offset = 0;
		UINT len = sizeof(XMMATRIX);
		UINT nLights = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			g_pHeapManager->CopyDataToCbWithOffset(m_spotLitMatricesKey, len, offset, m_pSpotLights[i]->GetSpotLightSpaceMatrix());
			offset += len;
		}
		offset = len * _MAX_SPT_LIGHTS;
		g_pHeapManager->CopyDataToCbWithOffset(m_spotLitMatricesKey, sizeof(UINT), offset, &nLights);
		return;
	}
	void LightBuffer12::UpdateLightProperty()
	{
		UINT dirLightOffset = 0;
		UINT len = sizeof(LightProperty);
		UINT nLights = m_pDirLights.size();
		for (int i = 0; i < m_pDirLights.size(); ++i)
		{
			shared_ptr<LightProperty> pProperty = m_pDirLights[i]->GetProperty();
			g_pHeapManager->CopyDataToCbWithOffset(m_lightPropertyKey, len, dirLightOffset, pProperty.get());
			dirLightOffset += len;
		}
		dirLightOffset = len * _MAX_DIR_LIGHTS;
		g_pHeapManager->CopyDataToCbWithOffset(m_lightPropertyKey, sizeof(UINT), dirLightOffset, &nLights);
		dirLightOffset += 16;//hlsl 16bytes align;


		UINT cubeLightOffset = dirLightOffset;
		len = sizeof(LightProperty);
		nLights = m_pCubeLights.size();
		for (int i = 0; i < m_pCubeLights.size(); ++i)
		{
			shared_ptr<LightProperty> pProperty = m_pCubeLights[i]->GetProperty();
			g_pHeapManager->CopyDataToCbWithOffset(m_lightPropertyKey, len, cubeLightOffset, pProperty.get());
			cubeLightOffset += len;
		}
		cubeLightOffset = dirLightOffset+len * _MAX_PNT_LIGHTS;
		g_pHeapManager->CopyDataToCbWithOffset(m_lightPropertyKey, sizeof(UINT), cubeLightOffset, &nLights);
		cubeLightOffset += 16;
		
		
		UINT sptLightOffset = cubeLightOffset;
		len = sizeof(LightProperty);
		nLights = m_pSpotLights.size();
		for (int i = 0; i < m_pSpotLights.size(); ++i)
		{
			shared_ptr<LightProperty> pProperty = m_pSpotLights[i]->GetProperty();
			g_pHeapManager->CopyDataToCbWithOffset(m_lightPropertyKey, len, sptLightOffset, pProperty.get());
			sptLightOffset += len;
		}

		sptLightOffset = cubeLightOffset+len * _MAX_SPT_LIGHTS;
		g_pHeapManager->CopyDataToCbWithOffset(m_lightPropertyKey, sizeof(UINT), sptLightOffset, &nLights);
	}
	void LightBuffer12::SetDirty(BYTE bits)
	{
		m_dirty |= bits;
	}
	void LightBuffer12::SetDirPass(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx)
	{
		m_pDirLights[idx]->UploadDirLightMatrices(pCmdList);
	}
	void LightBuffer12::SetCubePass(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx)
	{
		m_pCubeLights[idx]->UploadCubeLightMatrices(pCmdList);
		m_pCubeLights[idx]->UploadLightPos(pCmdList);
	}
	void LightBuffer12::SetSpotPass(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT idx)
	{
		m_pSpotLights[idx]->UploadSpotLightMatrix(pCmdList);
	}
	void LightBuffer12::SetLightingPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		g_pHeapManager->UploadGraphicsCb(m_dirLitMatricesKey, E_TO_UINT(ePbrLightRP::psDirLitMat), pCmdList);
		g_pHeapManager->UploadGraphicsCb(m_spotLitMatricesKey, E_TO_UINT(ePbrLightRP::psSpotLitMat), pCmdList);
		g_pHeapManager->UploadGraphicsCb(m_lightPropertyKey, E_TO_UINT(ePbrLightRP::psLight), pCmdList);
	}

}