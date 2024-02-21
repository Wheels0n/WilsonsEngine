#pragma once

#include<vector>

#include"DirectionalLight.h"
#include"PointLight.h"
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
		inline void PushPointLight(PointLight* pPointLight)
		{
			m_pPointLights.push_back(pPointLight);
		}
		inline void PushSpotLight(SpotLight* pSpotLight)
		{
			m_pSpotLights.push_back(pSpotLight);
		}
		inline std::vector<DirectionalLight*>& GetDirLights()
		{
			return m_pDirLights;
		}
		inline std::vector<PointLight*>& GetPointLights()
		{
			return m_pPointLights;
		}
		inline std::vector<SpotLight*>& GetSpotLights() 
		{
			return m_pSpotLights;
		}
		inline UINT GetDirLightSize() const
		{
			return m_pDirLights.size();
		}
		inline UINT GetPointLightSize() const
		{
			return m_pPointLights.size();
		}
		inline UINT GetSpotLightSize() const
		{
			return m_pSpotLights.size();
		}
		inline UINT GetDirLightCapacity() const
		{
			return _MAX_DIR_LIGHTS;
		}
		inline UINT GetPointLightCapacity() const
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
		std::vector<PointLight*>m_pPointLights;
		std::vector<SpotLight*>m_pSpotLights;
	};
}