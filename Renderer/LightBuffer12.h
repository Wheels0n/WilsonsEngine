#pragma once

#include"DirectionalLight12.h"
#include"CubeLight12.h"
#include"SpotLight12.h"
#include"typedef.h"
namespace wilson
{
	class HeapManager;
	class LightBuffer12
	{
	public:
		inline std::vector<std::unique_ptr<CubeLight12>>& GetCubeLights()
		{
			return m_pCubeLights;
		}
		inline UINT GetCubeLightsCapacity() const
		{
			return _MAX_PNT_LIGHTS;
		}
		inline UINT GetCubeLightsSize() const
		{
			return m_pCubeLights.size();
		}
		inline std::vector<std::unique_ptr<DirectionalLight12>>& GetDirLights()
		{
			return m_pDirLights;
		}
		inline UINT GetDirLightsCapacity() const
		{
			return _MAX_DIR_LIGHTS;
		}
		inline UINT GetDirLightsSize() const
		{
			return m_pDirLights.size();
		}
		inline std::vector<std::unique_ptr<SpotLight12>>& GetSpotLights()
		{
			return m_pSpotLights;
		}
		inline UINT GetSpotLightsCapacity() const
		{
			return _MAX_SPT_LIGHTS;
		}
		inline UINT GetSpotLightsSize() const
		{
			return m_pSpotLights.size();
		}
		inline void PushCubeLight(CubeLight12* const pCubeLight)
		{
			std::unique_ptr<CubeLight12> uptr;
			uptr.reset(pCubeLight);
			m_pCubeLights.push_back(std::move(uptr));
		}
		inline void PushDirLight(DirectionalLight12* const pDirLight)
		{
			std::unique_ptr<DirectionalLight12> uptr;
			uptr.reset(pDirLight);
			m_pDirLights.push_back(std::move(uptr));
		}
		inline void PushSpotLight(SpotLight12* const pSpotLight)
		{
			std::unique_ptr<SpotLight12> uptr;
			uptr.reset(pSpotLight);
			m_pSpotLights.push_back(std::move(uptr));
		}
		void UpdateDirLightMatrices(ID3D12GraphicsCommandList* const pCommandlist);
		void UpdateLightBuffer(ID3D12GraphicsCommandList* const pCommandlist);
		void UpdateSpotLightMatrices(ID3D12GraphicsCommandList* const pCommandlist);

		LightBuffer12(ID3D12Device* const, ID3D12GraphicsCommandList* const, HeapManager* const);
		~LightBuffer12();
	private:

		D3D12_GPU_DESCRIPTOR_HANDLE m_lightPropertyCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_dirLitMatricesCbv;
		D3D12_GPU_DESCRIPTOR_HANDLE m_spotLitMatricesCbv;

		std::vector<std::unique_ptr<CubeLight12>>m_pCubeLights;
		std::vector<std::unique_ptr<DirectionalLight12>> m_pDirLights;
		std::vector<std::unique_ptr<SpotLight12>>m_pSpotLights;

		UINT8* m_pDirLitMatricesCbBegin;
		UINT8* m_pLightPropertyCbBegin;
		UINT8* m_pSpotLitMatricesCbBegin;
	};
}