#ifndef _D3D11_H_
#define _D3D11_H_

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

//D:\DirectxSDK\Include;$(IncludePath)  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include;$(IncludePath)
//D:\DirectxSDK\Lib\x64;$(LibraryPath)  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x64;$(LibraryPath)

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
	constexpr UINT SHADOWMAP_SIZE = 1024;
	constexpr float CUBE_SIZE = 0.25;
	constexpr UINT _GBUF_CNT = 6;
	class D3D11
	{
	private:
		bool CreateDepthBuffer(int, int,
			ID3D11Texture2D**,
			ID3D11DepthStencilView**);
		bool CreateDSS();

		void DrawENTT(bool bGeoPass);
		void DestroyDSS();
		void DestroyRTT();
		void DestroyDSBforRTT();
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
		inline int GetClientWidth() const
		{
			return m_clientWidth;
		};
		inline int GetClientHeight() const
		{
			return m_clientHeight;
		};
		inline UINT GetModelGroupSize()
		{
			return m_pModelGroups.size();
		}
		inline void PickModel(int i, int j)
		{
			m_selectedModelGroup = i;
			m_selectedModel = j;
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

		bool m_bVsyncOn;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pContext;

		ID3D11Buffer* m_pCubeVertices, *m_pQuadVB;
		ID3D11Buffer* m_pCubeIndices, *m_pQuadIB;
		ID3D11Buffer* m_pBoolBuffer, *m_pColorBuffer, *m_pSSAOKernelBuffer;

		ID3D11RenderTargetView* m_pScreenRTTV, * m_pSceneRTTV, *m_pSSAORTTV, * m_pSSAOBlurRTTV, * m_pBrightRTTV, *m_pViewportRTTV;
		ID3D11RenderTargetView* m_pPingPongRTTV[2];
		ID3D11RenderTargetView* m_pGbufferRTTV[_GBUF_CNT];//0:pos, 1:normal, 2:albedo, 3:specular, 4:vPos, 5:vNormal
		ID3D11Texture2D* m_pDSBuffer, * m_pDSBufferForRTT, * m_pSceneRTT, *m_pSSAORTT, * m_pSSAOBlurRTT, *m_pBrightRTT, *m_pViewportRTT;
		ID3D11Texture2D* m_pPingPongRTT[2];
		ID3D11Texture2D* m_pGbufferRTT[_GBUF_CNT];
		ID3D11Texture2D* m_pNoiseRTT;
		ID3D11ShaderResourceView* m_pNoiseSRV;
		ID3D11ShaderResourceView* m_pSceneSRV, *m_pSSAOSRV, *m_pSSAOBlurSRV, * m_pBrightSRV, *m_pSkyBoxSRV, *m_pViewportSRV;
		ID3D11ShaderResourceView* m_pPingPongSRV[2];
		ID3D11ShaderResourceView* m_pGbufferSRV[_GBUF_CNT];

		ID3D11DepthStencilState* m_pOutlinerSetupDSS, * m_pOutlinerTestDSS, * m_pDrawReflectionDSS, *m_pSkyBoxDSS;
		ID3D11DepthStencilView* m_pScreenDSV, * m_pSceneDSV;

		ID3D11RasterizerState* m_pGeoRS, * m_pRasterStateCC, *m_pSkyBoxRS, *m_pQuadRS;
		ID3D11SamplerState* m_pSampleState, *m_pSSAOPosSS;
		ID3D11BlendState* m_pGBufferWriteBS, * m_pLightingPassBS;

		D3D11_VIEWPORT m_viewport;

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
		int m_clientWidth;
		int m_clientHeight;
		int m_selectedModelGroup;
		int m_selectedModel;
		std::vector<XMFLOAT3> m_rotationVecs;
		
	};
}
#endif // !_D3D11_H_
