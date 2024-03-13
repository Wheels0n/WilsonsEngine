#pragma once

#include <dxgi.h>
#include <d3dcommon.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <DirectXTex.h>
#include <Windows.h>
#include <vector>

#include "Camera.h"
#include "Frustum.h"
#include "MatrixBuffer.h"
#include "LightBuffer.h"
#include "Shader.h"
#include "Terrain.h"
#include "Import.h"
#include "ShadowMap.h"
#include "typedef.h"

namespace wilson
{   

	class D3D11
	{

	public:

		void ResizeBackBuffer(int, int);
		void UpdateScene();
		void DrawScene();
		void AddLight(Light*);
		void AddModelGroup(ModelGroup*);
		void RemoveModelGroup(int i);
		void RemoveLight(int i, Light* pLight);
		void RemoveModel(int modelGroupIdx, int modelIdx);
		UINT GetModelSize(int i);
		inline Camera* GetCam() const
		{
			return m_pCam;
		};
		inline Frustum* GetFrustum() const
		{
			return m_pFrustum;
		};
		inline void SetNewFrustum(Frustum* pFrustum)
		{
			m_pFrustum = pFrustum;
		}
		inline ShadowMap* GetShadowMap() const
		{
			return m_pShadowMap;
		};

		inline IDXGISwapChain* GetSwapChain() const
		{
			return m_pSwapChain;
		}
		inline ID3D11ShaderResourceView* GetFinalSRV() const
		{
			return m_pViewportSRV;
		};
		inline ID3D11ShaderResourceView* GetSSAOBlurredSRV() const
		{
			return m_pSSAOBlurDebugSRV;
		};
		inline ID3D11ShaderResourceView** GetGbufferSRV()
		{
			return &m_pGbufferSRV[0];
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
		inline float* GetHeightScale()
		{
			return &m_heightScale;
		}
		inline BOOL* GetHeighOnOFF()
		{
			return &m_bHeightOnOff;
		}
		inline int GetClientWidth() const
		{
			return m_clientWidth;
		};
		inline int GetClientHeight() const
		{
			return m_clientHeight;
		};
		inline UINT GetModelGroupSize() const
		{
			return m_pModelGroups.size();
		}
		inline void PickModel(int i, int j)
		{
			m_selectedModelGroup = i;
			m_selectedModel = j;
		}
		inline void ToggleAABBGrid()
		{
			m_bAABBGridOn = !m_bAABBGridOn;
		}
		inline UINT GetGbufferCount() const
		{
			return  eGbuf_cnt;
		}
		UINT GetLightSize(eLIGHT_TYPE);
		
		bool CreateRTVandSRV(int, int);
		
		D3D11() = default;
		D3D11(int, int, bool, HWND, bool, float, float);
		D3D11(const D3D11&) = delete;
		~D3D11();
	private:
		bool CreateEquirentangularMap(const char* pPath);
		void ConvertEquirectagular2Cube();
		void CreateDiffuseIrradianceMap();
		void CreatePrefileterMap();
		void CreateBRDFMap();

		bool CreateDSV(int, int,
			ID3D11Texture2D**,
			ID3D11DepthStencilView**);
		bool CreateDSS();

		void DrawObject(bool bGeoPass, bool bSpotShadowPass);

		void DestroyDSS();
		void DestroyRTVandSRV();
		void DestroyHDR();
		void DestroySceneDepthTexAndDSV();
		void DestroyBackBuffer();

		void D3DMemoryLeakCheck();
	private:

		bool m_bVsyncOn;
		bool m_bAABBGridOn;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pContext;

		ID3D11Buffer* m_pCubeVertices;
		ID3D11Buffer* m_pQuadVB;
		ID3D11Buffer* m_pCubeIndices;
		ID3D11Buffer* m_pQuadIB;
		ID3D11Buffer* m_pBoolCB; 
		ID3D11Buffer* m_pColorCB; 
		ID3D11Buffer* m_pSSAOKernelCB; 
		ID3D11Buffer* m_pExposureCB; 
		ID3D11Buffer* m_pHeightScaleCB;
		ID3D11Buffer* m_pHeightOnOffCB;
		ID3D11Buffer* m_pEquirect2CubeCB;
		ID3D11Buffer* m_pAABBVB;
		ID3D11Buffer* m_pAABBIB;

		ID3D11Texture2D* m_pScreenDepthTex;
		ID3D11Texture2D* m_pSceneDepthTex;
		ID3D11Texture2D* m_pSceneTex;
		ID3D11Texture2D* m_pSSAOTex;
		ID3D11Texture2D* m_pSSAOBlurTex;
		ID3D11Texture2D* m_pSSAOBlurDebugTex;
		ID3D11Texture2D* m_pBrightTex;
		ID3D11Texture2D* m_pViewportTex;
		ID3D11Texture2D* m_pPingPongTex[2];
		ID3D11Texture2D* m_pGbufferTex[eGbuf_cnt];
		ID3D11Texture2D* m_pNoiseTex;
		ID3D11Texture2D* m_pHDRTex;
		ID3D11Texture2D* m_pSkyBoxTex;
		ID3D11Texture2D* m_pDiffIrradianceTex;
		ID3D11Texture2D* m_pPrefilterTex;
		ID3D11Texture2D* m_pBRDFTex;

		ID3D11RenderTargetView* m_pScreenRTTV;
		ID3D11RenderTargetView* m_pSceneRTTV; 
		ID3D11RenderTargetView* m_pSSAORTTV;
		ID3D11RenderTargetView* m_pSSAOBlurRTTV; 
		ID3D11RenderTargetView* m_pSSAOBlurDebugRTTV;
		ID3D11RenderTargetView* m_pBrightRTTV; 
		ID3D11RenderTargetView* m_pViewportRTTV;
		ID3D11RenderTargetView* m_pPingPongRTTV[2];
		ID3D11RenderTargetView* m_pGbufferRTTV[eGbuf_cnt];//0:pos, 1:normal, 2:albedo, 3:specular, 4:emissive, 5: vPos, 6:vNormal, 7:depth;
		ID3D11RenderTargetView* m_pSkyBoxRTV;
		ID3D11RenderTargetView* m_pDiffIrradianceRTTV;
		ID3D11RenderTargetView* m_pPrefilterRTTV;
		ID3D11RenderTargetView* m_pBRDFRTTV;

		ID3D11ShaderResourceView* m_pNoiseSRV;
		ID3D11ShaderResourceView* m_pSceneSRV;
		ID3D11ShaderResourceView* m_pSSAOSRV;
		ID3D11ShaderResourceView* m_pSSAOBlurSRV;
		ID3D11ShaderResourceView* m_pSSAOBlurDebugSRV;
		ID3D11ShaderResourceView* m_pBrightSRV;
		ID3D11ShaderResourceView* m_pViewportSRV;
		ID3D11ShaderResourceView* m_pPingPongSRV[2];
		ID3D11ShaderResourceView* m_pGbufferSRV[eGbuf_cnt];
		ID3D11ShaderResourceView* m_pHDRSRV; 
		ID3D11ShaderResourceView* m_pSkyBoxSRV; 
		ID3D11ShaderResourceView* m_pDiffIrradianceSRV;
		ID3D11ShaderResourceView* m_pPrefilterSRV;
		ID3D11ShaderResourceView* m_pBRDFSRV;

		ID3D11DepthStencilState* m_pOutlinerSetupDSS;
		ID3D11DepthStencilState* m_pOutlinerTestDSS;
		ID3D11DepthStencilState* m_pSkyBoxDSS;

		ID3D11DepthStencilView* m_pScreenDSV;
		ID3D11DepthStencilView* m_SceneDSV;

		ID3D11RasterizerState* m_pGeoRS;
		ID3D11RasterizerState* m_pRasterStateCC;
		ID3D11RasterizerState* m_pSkyBoxRS;
		ID3D11RasterizerState* m_pQuadRS;
		ID3D11RasterizerState* m_pAABBRS;

		ID3D11SamplerState* m_pWrapSS;
		ID3D11SamplerState* m_pClampSS;

		ID3D11BlendState* m_pGBufferWriteBS;
		ID3D11BlendState* m_pLightingPassBS;

		D3D11_VIEWPORT m_viewport;
		D3D11_VIEWPORT m_diffIrradViewport;
		D3D11_VIEWPORT m_prefilterViewport;

		std::vector<ModelGroup*> m_pModelGroups;
		Terrain* m_pTerrain;
		Camera* m_pCam;
		Frustum* m_pFrustum;
		LightBuffer* m_pLightBuffer;
		MatBuffer* m_pMatBuffer;
		Shader* m_pShader;
		ShadowMap* m_pShadowMap;

		BOOL m_bHeightOnOff;
		XMMATRIX m_idMat = XMMatrixIdentity();
		float m_exposure;
		float m_heightScale;
		UINT m_clientWidth;
		UINT m_clientHeight;
		UINT m_selectedModelGroup;
		UINT m_selectedModel;
		std::vector<XMFLOAT3> m_rotationVecs;
	
		
	};
}

