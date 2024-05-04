#pragma once
#include"DirectionalLight11.h"
#include"CubeLight11.h"
#include"SpotLight11.h"
#include"typedef.h"
namespace wilson
{	

	class LightBuffer11
	{
	public:
		inline std::vector<CubeLight11*>& GetCubeLights()
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
		inline std::vector<DirectionalLight11*>& GetDirLights()
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
		inline ID3D11ShaderResourceView** GetNullSRVs()
		{
			return m_pNullSrvs[0].GetAddressOf();
		}
		inline std::vector<SpotLight11*>& GetSpotLights() 
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
		inline void PushCubeLight(CubeLight11* const pCubeLight)
		{
			m_pCubeLights.push_back(pCubeLight);
		}
		inline void PushDirLight(DirectionalLight11* const pDirLight)
		{
			m_pDirLights.push_back(pDirLight);
		}
		inline void PushSpotLight(SpotLight11* const pSpotLight)
		{
			m_pSpotLights.push_back(pSpotLight);
		}
		void UpdateDirLightMatrices(ID3D11DeviceContext* const pContext);
		void UpdateLightBuffer(ID3D11DeviceContext* const pContext);
		void UpdateSpotLightMatrices(ID3D11DeviceContext* const pContext);
		
		LightBuffer11(ID3D11Device* const);
		~LightBuffer11();
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pDirLitMatricesCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pLightPropertyCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pSpotLitMatricesCb;

		std::vector<CubeLight11*>m_pCubeLights;
		std::vector<DirectionalLight11*> m_pDirLights;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pNullSrvs;
		std::vector<SpotLight11*>m_pSpotLights;
	};
}