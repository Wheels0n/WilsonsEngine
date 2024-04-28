#pragma once

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXTex.h>

#include "Camera11.h"
#include "Frustum11.h"
#include "MatrixBuffer11.h"
#include "LightBuffer11.h"
#include "Shader11.h"
#include "Terrain.h"
#include "Import11.h"
#include "ShadowMap11.h"
#include "typedef.h"

namespace wilson
{   

	class D3D11
	{

	public:

		void AddLight(Light11* const);
		void AddObject(Object11* const);
		bool CreateRtvSrv(const UINT, const UINT);
		void DrawScene();
		inline Camera11* GetCam() const
		{
			return m_pCam;
		};
		inline UINT GetClientHeight() const
		{
			return m_clientHeight;
		};
		inline UINT GetClientWidth() const
		{
			return m_clientWidth;
		};
		inline Frustum11* GetFrustum() const
		{
			return m_pFrustum;
		};
		inline ID3D11Device* GetDevice() const
		{
			return m_pDevice;
		};
		inline ID3D11DeviceContext* GetDeviceContext() const
		{
			return m_pContext;
		};
		inline float* GetExposure()
		{
			return &m_exposure;
		}
		inline ID3D11ShaderResourceView** GetpGbufferSrvs()
		{
			return &m_ppGbufferSrvs[0];
		};
		inline float* GetpHeightScale()
		{
			return &m_heightScale;
		}
		inline BOOL* GetpbHeightOn()
		{
			return &m_bHeightOn;
		}
		UINT GetNumLight(const eLIGHT_TYPE);
		inline UINT GetNumGBuffer() const
		{
			return  static_cast<UINT>(eGbuf::cnt);
		}
		UINT GetNumMesh(const UINT i);
		inline UINT GetNumObject() const
		{
			return m_pObjects.size();
		}
		inline ID3D11ShaderResourceView* GetPostProcessSrv() const
		{
			return m_pViewportSrv;
		};
		inline ShadowMap* GetShadowMap() const
		{
			return m_pShadowMap;
		};
		inline ID3D11ShaderResourceView* GetSsaoBlurredSrv() const
		{
			return m_pSsaoBlurDebugSrv;
		};
		inline IDXGISwapChain* GetSwapChain() const
		{
			return m_pSwapChain;
		}
		inline void PickSubMesh(const int i, const int j)
		{
			m_selectedObject = i;
			m_selectedMesh = j;
		}
		void RemoveLight(const UINT i, Light11* const pLight);
		void RemoveMesh(const UINT objectIdx, const UINT meshlIdx);
		void RemoveObject(const UINT i);
		void ResizeBackBuffer(const UINT, const UINT);
		inline void SetNewFrustum(Frustum11* const pFrustum)
		{
			m_pFrustum = pFrustum;
		}
		inline void ToggleAabbGrid()
		{
			m_bAabbGridOn = !m_bAabbGridOn;
		}
		void UpdateScene();

		
		D3D11() = default;
		D3D11(const UINT, const UINT, const bool, HWND, const bool, const float, const float);
		D3D11(const D3D11&) = delete;
		~D3D11();
	private:
		void ConvertEquirectagular2Cube();
		void CreateBrdfMap();
		void CreateDiffuseIrradianceMap();
		bool CreateDss();
		bool CreateDsv(const UINT, const UINT,
			ID3D11Texture2D** const,
			ID3D11DepthStencilView** const);
		bool CreateEquirentangularMap(const char* pPath);
		void CreatePrefileterMap();
		void DestroyBackBuffer();
		void DestroyDss();
		void DestroyHdr();
		void DestroyRtvSrv();
		void DestroySceneDepthTexDsv();
		void DrawObject(const bool bGeoPass, const bool bSpotShadowPass);
		void D3DMemoryLeakCheck();
	private:

		bool m_bAabbGridOn;
		bool m_bVsyncOn;
	
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pContext;
		IDXGISwapChain* m_pSwapChain;

		ID3D11Buffer* m_pAabbIb;
		ID3D11Buffer* m_pAabbVb;
		ID3D11Buffer* m_pBoolCb;
		ID3D11Buffer* m_pColorCb;
		ID3D11Buffer* m_pCubeIndices;
		ID3D11Buffer* m_pCubeVertices;
		ID3D11Buffer* m_pEquirect2CubeCb;
		ID3D11Buffer* m_pExposureCb;
		ID3D11Buffer* m_pHeightScaleCb;
		ID3D11Buffer* m_pHeightOnOffCb;
		ID3D11Buffer* m_pQuadIb;
		ID3D11Buffer* m_pQuadVb;
		ID3D11Buffer* m_pSsaoKernelCb; 

		ID3D11Texture2D* m_pBrdfTex;
		ID3D11Texture2D* m_pBrightTex;
		ID3D11Texture2D* m_pDiffIrradianceTex;
		ID3D11Texture2D* m_pGbufferTex[static_cast<int>(eGbuf::cnt)];
		ID3D11Texture2D* m_pHdrTex;
		ID3D11Texture2D* m_pNoiseTex;
		ID3D11Texture2D* m_pSceneTex;
		ID3D11Texture2D* m_pSceneDepthTex;
		ID3D11Texture2D* m_pScreenDepthTex;
		ID3D11Texture2D* m_pSkyBoxTex;
		ID3D11Texture2D* m_pSsaoTex;
		ID3D11Texture2D* m_pSsaoBlurTex;
		ID3D11Texture2D* m_pSsaoBlurDebugTex;
		ID3D11Texture2D* m_pPingPongTex[2];
		ID3D11Texture2D* m_pPrefilterTex;
		ID3D11Texture2D* m_pViewportTex;

		ID3D11RenderTargetView* m_pBrdfRtv;
		ID3D11RenderTargetView* m_pBrightRtv;
		ID3D11RenderTargetView* m_pDiffIrradianceRtv;
		ID3D11RenderTargetView* m_pGbufferRtvs[static_cast<int>(eGbuf::cnt)];
		ID3D11RenderTargetView* m_pSceneRtv; 
		ID3D11RenderTargetView* m_pScreenRtv;
		ID3D11RenderTargetView* m_pSkyBoxRtv;
		ID3D11RenderTargetView* m_pSsaoRtv;
		ID3D11RenderTargetView* m_pSsaoBlurRtv; 
		ID3D11RenderTargetView* m_pSsaoBlurDebugRtv;
		ID3D11RenderTargetView* m_pPingPongRtvs[2];
		ID3D11RenderTargetView* m_pPrefilterRtv;
		ID3D11RenderTargetView* m_pViewportRtv;

		ID3D11ShaderResourceView* m_pBrdfSrv;
		ID3D11ShaderResourceView* m_pBrightSrv;
		ID3D11ShaderResourceView* m_pDiffIrradianceSrv;
		ID3D11ShaderResourceView* m_ppGbufferSrvs[static_cast<int>(eGbuf::cnt)];
		ID3D11ShaderResourceView* m_pHdrSrv;
		ID3D11ShaderResourceView* m_pNoiseSrv;
		ID3D11ShaderResourceView* m_pSceneSrv;
		ID3D11ShaderResourceView* m_pSkyBoxSrv;
		ID3D11ShaderResourceView* m_pSsaoSrv;
		ID3D11ShaderResourceView* m_pSsaoBlurSrv;
		ID3D11ShaderResourceView* m_pSsaoBlurDebugSrv;
		ID3D11ShaderResourceView* m_pPingPongSrvs[2];
		ID3D11ShaderResourceView* m_pPrefilterSrv;
		ID3D11ShaderResourceView* m_pViewportSrv;

		ID3D11DepthStencilState* m_pOutlinerTestDss;
		ID3D11DepthStencilState* m_pOutlinerSetupDss;
		ID3D11DepthStencilState* m_pSkyBoxDss;

		ID3D11DepthStencilView* m_SceneDsv;
		ID3D11DepthStencilView* m_ScreenDsv;

		ID3D11RasterizerState* m_pAabbRs;
		ID3D11RasterizerState* m_pGeoRs;
		ID3D11RasterizerState* m_pQuadRs;
		ID3D11RasterizerState* m_pRs;
		ID3D11RasterizerState* m_pSkyBoxRs;

		ID3D11SamplerState* m_pWrapSs;
		ID3D11SamplerState* m_pClampSs;

		ID3D11BlendState* m_pGBufferWriteBs;
		ID3D11BlendState* m_pLightingPassBs;

		D3D11_VIEWPORT m_viewport;
		D3D11_VIEWPORT m_diffIrradViewport;
		D3D11_VIEWPORT m_prefilterViewport;

		std::vector<Object11*> m_pObjects;
		Terrain* m_pTerrain;
		Camera11* m_pCam;
		Frustum11* m_pFrustum;
		LightBuffer11* m_pLightCb;
		MatBuffer11* m_pMatricesCb;
		Shader11* m_pShader;
		ShadowMap* m_pShadowMap;

		BOOL m_bHeightOn;
		XMMATRIX m_idMat = XMMatrixIdentity();
		float m_exposure;
		float m_heightScale;
		UINT m_clientWidth;
		UINT m_clientHeight;
		INT m_selectedObject;
		INT m_selectedMesh;
		std::vector<XMFLOAT3> m_rotationVecs;
	
		
	};
}

