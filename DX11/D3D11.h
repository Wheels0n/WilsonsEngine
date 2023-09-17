#pragma once

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <D3DX11tex.h>
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


namespace wilson
{   
	struct QUAD
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex;
	};
	struct SamplePoints
	{
		XMVECTOR coord[64];
	};
	constexpr UINT _SHADOWMAP_SIZE = 1024;
	constexpr float CUBE_SIZE = 0.25;
	constexpr UINT _GBUF_CNT =5;
	class D3D11
	{

	public:
		bool Init(int, int, bool, HWND, bool, float, float);
		void Shutdown();

		void UpdateScene();
		void DrawScene();
		void AddLight(Light*);
		void AddModelGroup(ModelGroup*, ID3D11Device*);
		void RemoveModelGroup(int i);
		void RemoveLight(int i, Light* pLight);
		void RemoveModel(int i, int j);
		inline Camera* GetCam() const
		{
			return m_pCam;
		};
		inline Frustum* GetFrustum() const
		{
			return m_pFrustum;
		};
		inline IDXGISwapChain* GetSwapChain() const
		{
			return m_pSwapChain;
		}
		inline ID3D11ShaderResourceView* GetRTT() const
		{
			return m_pViewportSRV;
		};
		inline ID3D11Device* GetDevice() const
		{
			return m_pDevice;
		};
		inline float* GetExposure()
		{
			return &m_exposure;
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
		UINT GetLightSize(Light* pLight);

		bool CreateRTT(int, int);
		bool CreateDSBforRTT(int w, int h)
		{
			DestroyDSBforRTT();
			return CreateDepthBuffer(w, h, &m_pDSBufferForRTT, &m_pSceneDSV);
		};

		D3D11();
		D3D11(const D3D11&) = delete;
		~D3D11()=default;
	private:
		bool CreateDepthBuffer(int, int,
			ID3D11Texture2D**,
			ID3D11DepthStencilView**);
		bool CreateDSS();

		void DrawENTT(bool bGeoPass);
		void DestroyDSS();
		void DestroyRTT();
		void DestroyDSBforRTT();
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
		ID3D11Buffer* m_pBoolBuffer; 
		ID3D11Buffer* m_pColorBuffer; 
		ID3D11Buffer* m_pSSAOKernelBuffer; 
		ID3D11Buffer* m_pExposureBuffer; 
		ID3D11Buffer* m_pEquirect2CubeBuffer;
		ID3D11Buffer* m_pAABBVBuffer;
		ID3D11Buffer* m_pAABBIBuffer;

		ID3D11Texture2D* m_pDSBuffer;
		ID3D11Texture2D* m_pDSBufferForRTT;
		ID3D11Texture2D* m_pSceneRTT;
		ID3D11Texture2D* m_pSSAORTT;
		ID3D11Texture2D* m_pSSAOBlurRTT;
		ID3D11Texture2D* m_pBrightRTT;
		ID3D11Texture2D* m_pViewportRTT;
		ID3D11Texture2D* m_pPingPongRTT[2];
		ID3D11Texture2D* m_pGbufferRTT[_GBUF_CNT];
		ID3D11Texture2D* m_pNoiseRTT;
		ID3D11Texture2D* m_pHDRRTT;
		ID3D11Texture2D* m_pSkyBoxRTT;
		ID3D11Texture2D* m_pDiffIrradianceRTT;
		ID3D11Texture2D* m_pPrefilterRTT;
		ID3D11Texture2D* m_pBRDFRTT;

		ID3D11RenderTargetView* m_pScreenRTTV;
		ID3D11RenderTargetView* m_pSceneRTTV; 
		ID3D11RenderTargetView* m_pSSAORTTV;
		ID3D11RenderTargetView* m_pSSAOBlurRTTV; 
		ID3D11RenderTargetView* m_pBrightRTTV; 
		ID3D11RenderTargetView* m_pViewportRTTV;
		ID3D11RenderTargetView* m_pPingPongRTTV[2];
		ID3D11RenderTargetView* m_pGbufferRTTV[_GBUF_CNT];//0:pos, 1:normal, 2:albedo, 3:specular, 4:emissive
		ID3D11RenderTargetView* m_pSkyBoxRTTV;
		ID3D11RenderTargetView* m_pDiffIrradianceRTTV;
		ID3D11RenderTargetView* m_pPrefilterRTTV;
		ID3D11RenderTargetView* m_pBRDFRTTV;

		ID3D11ShaderResourceView* m_pNoiseSRV;
		ID3D11ShaderResourceView* m_pSceneSRV;
		ID3D11ShaderResourceView* m_pSSAOSRV;
		ID3D11ShaderResourceView* m_pSSAOBlurSRV;
		ID3D11ShaderResourceView* m_pBrightSRV;
		ID3D11ShaderResourceView* m_pViewportSRV;
		ID3D11ShaderResourceView* m_pPingPongSRV[2];
		ID3D11ShaderResourceView* m_pGbufferSRV[_GBUF_CNT];
		ID3D11ShaderResourceView* m_pHDRSRV; 
		ID3D11ShaderResourceView* m_pSkyBoxSRV; 
		ID3D11ShaderResourceView* m_pDiffIrradianceSRV;
		ID3D11ShaderResourceView* m_pPrefilterSRV;
		ID3D11ShaderResourceView* m_pBRDFSRV;

		ID3D11DepthStencilState* m_pOutlinerSetupDSS;
		ID3D11DepthStencilState* m_pOutlinerTestDSS;
		ID3D11DepthStencilState* m_pSkyBoxDSS;

		ID3D11DepthStencilView* m_pScreenDSV;
		ID3D11DepthStencilView* m_pSceneDSV;

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

		Importer* m_pImporter;
		std::vector<ModelGroup*> m_pModelGroups;
		Terrain* m_pTerrain;
		Camera* m_pCam;
		Frustum* m_pFrustum;
		LightBuffer* m_pLightBuffer;
		MatBuffer* m_pMatBuffer;
		Shader* m_pShader;
		ShadowMap* m_pShadowMap;

		XMMATRIX m_idMat = XMMatrixIdentity();
		float m_exposure;
		UINT m_clientWidth;
		UINT m_clientHeight;
		int m_selectedModelGroup;
		int m_selectedModel;
		std::vector<XMFLOAT3> m_rotationVecs;
	
		
	};
}

