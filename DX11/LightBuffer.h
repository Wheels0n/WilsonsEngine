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

	struct DirLightMatrices
	{
		DirectX::XMMATRIX matrices[MAX_DIR_LIGHTS];
		UINT dirCnt;
	};

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
		inline UINT GetDirLightSize()
		{
			return m_pDirLights.size();
		}
		inline UINT GetPointLightSize()
		{
			return m_pPointLights.size();
		}
		inline UINT GetSpotLightSize()
		{
			return m_pSpotLights.size();
		}
		inline UINT GetDirLightCapacity()
		{
			return MAX_DIR_LIGHTS;
		}
		inline UINT GetPointLightCapacity()
		{
			return MAX_PNT_LIGHTS;
	    }
		inline ID3D11ShaderResourceView** GetNullSRVs()
		{
			return &m_pNullSRVs[0];
		}
		void UpdateDirLightByIndex(ID3D11DeviceContext* pContext, UINT i);
		void UpdateDirLightMatrices(ID3D11DeviceContext* pContext);
		void UpdateLightBuffer(ID3D11DeviceContext* pContext);
		LightBuffer(ID3D11Device*);
		~LightBuffer();
	private:
		ID3D11Buffer* m_pLightPropertyBuffer, *m_pLitMatricesBuffer;
		std::vector< ID3D11ShaderResourceView*> m_pNullSRVs;
		std::vector<DirectionalLight*> m_pDirLights;
		std::vector<PointLight*>m_pPointLights;
		std::vector<SpotLight*>m_pSpotLights;
	};
}