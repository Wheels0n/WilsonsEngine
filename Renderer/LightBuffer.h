#pragma once

#include<vector>

#include"DirectionalLight.h"
#include"CubeLight.h"
#include"SpotLight.h"
#include"typedef.h"
namespace wilson
{	

	class LightBuffer
	{
	public:
		inline void PushDirLight(DirectionalLight* pDirLight)
		{
			m_pDirLights.push_back(pDirLight);
		}
		inline void PushCubeLight(CubeLight* pCubeLight)
		{
			m_pCubeLights.push_back(pCubeLight);
		}
		inline void PushSpotLight(SpotLight* pSpotLight)
		{
			m_pSpotLights.push_back(pSpotLight);
		}
		inline std::vector<DirectionalLight*>& GetDirLights()
		{
			return m_pDirLights;
		}
		inline std::vector<CubeLight*>& GetCubeLights()
		{
			return m_pCubeLights;
		}
		inline std::vector<SpotLight*>& GetSpotLights() 
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
		inline ID3D11ShaderResourceView** GetNullSRVs() 
		{
			return &m_pNullSRVs[0];
		}

		void UpdateDirLightMatrices(ID3D11DeviceContext* pContext);
		void UpdateSpotLightMatrices(ID3D11DeviceContext* pContext);
		void UpdateLightBuffer(ID3D11DeviceContext* pContext);
		
		LightBuffer(ID3D11Device*);
		~LightBuffer();
	private:
		ID3D11Buffer* m_pLightPropertyBuffer;
		ID3D11Buffer* m_pDirLitMatricesBuffer;
		ID3D11Buffer* m_pSpotLitMatricesBuffer;

		std::vector<ID3D11ShaderResourceView*> m_pNullSRVs;
		std::vector<DirectionalLight*> m_pDirLights;
		std::vector<CubeLight*>m_pCubeLights;
		std::vector<SpotLight*>m_pSpotLights;
	};
}