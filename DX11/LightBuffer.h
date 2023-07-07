#pragma once
#include<vector>
#include"DirectionalLight.h"
#include"PointLight.h"
#include"SpotLight.h"
namespace wilson
{	
	constexpr UINT MAX_DIR_LIGHTS = 10;
	constexpr UINT MAX_PNT_LIGHTS = 48;
	constexpr UINT MAX_SPT_LIGHTS = 20;

	struct LightBufferProperty
	{	
		DirLightProperty dirLights[MAX_DIR_LIGHTS];
		UINT dirCnt;
		PointLightProperty pntLights[MAX_PNT_LIGHTS];
		UINT pntCnt;
		SpotLightProperty  sptLights[MAX_SPT_LIGHTS];
		UINT sptCnt;
		UINT padding;
	};

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

		void UpdateLightBuffer(ID3D11DeviceContext* pContext);
		LightBuffer(ID3D11Device*);
		~LightBuffer();
	private:
		ID3D11Buffer* m_pLightBuffer;
		std::vector<DirectionalLight*> m_pDirLights;
		std::vector<PointLight*>m_pPointLights;
		std::vector<SpotLight*>m_pSpotLights;
	};
}