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
			return m_pDevice.Get();
		};
		inline ID3D11DeviceContext* GetDeviceContext() const
		{
			return m_pContext.Get();
		};
		inline float* GetExposure()
		{
			return &m_exposure;
		}
		inline ID3D11ShaderResourceView** GetpGbufferSrvs()
		{
			return m_ppGbufferSrvs[0].GetAddressOf();
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
			return m_pViewportSrv.Get();
		};
		inline ShadowMap* GetShadowMap() const
		{
			return m_pShadowMap;
		};
		inline ID3D11ShaderResourceView* GetSsaoBlurredSrv() const
		{
			return m_pSsaoBlurDebugSrv.Get();
		};
		inline IDXGISwapChain* GetSwapChain() const
		{
			return m_pSwapChain.Get();
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
	
		Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pAabbIb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pAabbVb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pBoolCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pColorCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pCubeIndices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pCubeVertices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pEquirect2CubeCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pExposureCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pHeightScaleCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pHeightOnOffCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pQuadIb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pQuadVb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pSsaoKernelCb;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pBrdfTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pBrightTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pDiffIrradianceTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pGbufferTex[static_cast<int>(eGbuf::cnt)];
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pHdrTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pNoiseTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pSceneTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pSceneDepthTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pScreenDepthTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pSkyBoxTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pSsaoTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pSsaoBlurTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pSsaoBlurDebugTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pPingPongTex[2];
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pPrefilterTex;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pViewportTex;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pBrdfRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pBrightRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pDiffIrradianceRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pGbufferRtvs[static_cast<int>(eGbuf::cnt)];
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pSceneRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pScreenRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pSkyBoxRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pSsaoRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pSsaoBlurRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pSsaoBlurDebugRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pPingPongRtvs[2];
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pPrefilterRtv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pViewportRtv;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pBrdfSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pBrightSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pDiffIrradianceSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ppGbufferSrvs[static_cast<int>(eGbuf::cnt)];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pHdrSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pNoiseSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSceneSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSkyBoxSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSsaoSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSsaoBlurSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSsaoBlurDebugSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pPingPongSrvs[2];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pPrefilterSrv;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pViewportSrv;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pOutlinerTestDss;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pOutlinerSetupDss;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pSkyBoxDss;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_SceneDsv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_ScreenDsv;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pAabbRs;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pGeoRs;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pQuadRs;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pRs;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pSkyBoxRs;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pWrapSs;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pClampSs;

		Microsoft::WRL::ComPtr<ID3D11BlendState> m_pGBufferWriteBs;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_pLightingPassBs;

		D3D11_VIEWPORT m_viewport;
		D3D11_VIEWPORT m_diffIrradViewport;
		D3D11_VIEWPORT m_prefilterViewport;

		std::vector<Object11*> m_pObjects;
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

