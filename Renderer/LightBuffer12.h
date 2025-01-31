#pragma once

#include"DirectionalLight12.h"
#include"CubeLight12.h"
#include"SpotLight12.h"
#include"typedef.h"
namespace wilson
{
	using namespace std;
	using namespace DirectX;

	class HeapManager;
	class LightBuffer12
	{
	private:
		void											UpdateDirLightMatrices();
		void											UpdateSpotLightMatrices();
		void											UpdateLightProperty();

	public:
		vector<shared_ptr<DirectionalLight12>>&			GetDirLights();
		vector<shared_ptr<CubeLight12>>&				GetCubeLights();
		vector<shared_ptr<SpotLight12>>&				GetSpotLights();

		UINT											GetCubeLightsSize();
		UINT											GetDirLightsSize();
		UINT											GetSpotLightsSize();

		void											PushCubeLight(shared_ptr<CubeLight12> pCubeLight);
		void											PushDirLight(shared_ptr<DirectionalLight12> pDirLight);
		void											PushSpotLight(shared_ptr<SpotLight12> pSpotLight);

		void											Update();

		BYTE											GetDirty() { return m_dirty; };
		void											SetDirty(BYTE bits);

		void											SetDirPass(ComPtr<ID3D12GraphicsCommandList>, UINT idx);
		void											SetCubePass(ComPtr<ID3D12GraphicsCommandList>, UINT idx);
		void											SetSpotPass(ComPtr<ID3D12GraphicsCommandList>, UINT idx);
		void											SetLightingPass(ComPtr<ID3D12GraphicsCommandList>);

														LightBuffer12(ComPtr<ID3D12Device>);
														~LightBuffer12()=default;
	private:

		UINT											m_lightPropertyKey;
		UINT											m_dirLitMatricesKey;
		UINT											m_spotLitMatricesKey;

		vector<shared_ptr<DirectionalLight12>>			m_pDirLights;
		vector<shared_ptr<CubeLight12>>					m_pCubeLights;
		vector<shared_ptr<SpotLight12>>					m_pSpotLights;

		BYTE											m_dirty;
	};
}