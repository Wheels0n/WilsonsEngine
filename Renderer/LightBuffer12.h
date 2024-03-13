#pragma once

#include<vector>

#include"DirectionalLight12.h"
#include"CubeLight12.h"
#include"SpotLight12.h"
#include"typedef.h"
namespace wilson
{
	class DescriptorHeapManager;
	class LightBuffer12
	{
	public:
		inline void PushDirLight(DirectionalLight12* pDirLight)
		{
			m_pDirLights.push_back(pDirLight);
		}
		inline void PushCubeLight(CubeLight12* pCubeLight)
		{
			m_pCubeLights.push_back(pCubeLight);
		}
		inline void PushSpotLight(SpotLight12* pSpotLight)
		{
			m_pSpotLights.push_back(pSpotLight);
		}
		inline std::vector<DirectionalLight12*>& GetDirLights()
		{
			return m_pDirLights;
		}
		inline std::vector<CubeLight12*>& GetCubeLights()
		{
			return m_pCubeLights;
		}
		inline std::vector<SpotLight12*>& GetSpotLights()
		{
			return m_pSpotLights;
		}
		inline UINT GetDirLightSize() const
		{
			return m_pDirLights.size();
		}
		inline UINT GetCubeLightSize() const
		{
			return m_pCubeLights.size();
		}
		inline UINT GetSpotLightSize() const
		{
			return m_pSpotLights.size();
		}
		inline UINT GetDirLightCapacity() const
		{
			return _MAX_DIR_LIGHTS;
		}
		inline UINT GetCubeLightCapacity() const
		{
			return _MAX_PNT_LIGHTS;
		}
		inline UINT GetSpotLightCapacity() const
		{
			return _MAX_SPT_LIGHTS;
		}

		void UpdateDirLightMatrices(ID3D12GraphicsCommandList* pCommandlist);
		void UpdateSpotLightMatrices(ID3D12GraphicsCommandList* pCommandlist);
		void UpdateLightBuffer(ID3D12GraphicsCommandList* pCommandlist);

		LightBuffer12(ID3D12Device*, ID3D12GraphicsCommandList*, DescriptorHeapManager*);
		~LightBuffer12();
	private:

		ID3D12Resource* m_pLightProperty12Buffer;
		ID3D12Resource* m_pDirLitMatrices12Buffer;
		ID3D12Resource* m_pSpotLitMatrices12Buffer;
		D3D12_GPU_DESCRIPTOR_HANDLE m_lightPropertyBufferCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_dirLitMatrices12BufferCBV;
		D3D12_GPU_DESCRIPTOR_HANDLE m_spotLitMatrices12BufferCBV;

		std::vector<DirectionalLight12*> m_pDirLights;
		std::vector<CubeLight12*>m_pCubeLights;
		std::vector<SpotLight12*>m_pSpotLights;

		UINT8* m_pLightPropertyCbBegin;
		UINT8* m_pDirLitMatricesCbBegin;
		UINT8* m_pSpotLitMatricesCbBegin;
	};
}