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
#include "camera.h"
#include "Frustum.h"
#include "MatrixBuffer.h"
#include "Light.h"
#include "Shader.h"
#include "Terrain.h"
#include "import.h"


//D:\DirectxSDK\Include;$(IncludePath)  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include;$(IncludePath)
//D:\DirectxSDK\Lib\x64;$(LibraryPath)  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x64;$(LibraryPath)

class CD3D11
{
	
public:
	CD3D11();
	CD3D11(const CD3D11&) = delete;
	~CD3D11();
	bool Init(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void UpdateScene();
	void DrawScene();
	void AddModel(CModel*, ID3D11Device*);
	void RemoveModel(int i);

	Camera* GetCam() const
	{
		return m_pCCam;
	};

	CLight* GetLight() const
	{
		return m_pCLight;
	}

	IDXGISwapChain* GetSwapChain() const
	{
		return m_pSwapChain;
	}

	ID3D11ShaderResourceView* GetRTT() const
	{
		return m_pSRVForRTT;
	};

	ID3D11Device* GetDevice() const
	{
		return m_pDevice;
	};
	int GetClientWidth() const
	{
		return m_clientWidth;
	};
	int GetClientHeight() const
	{
		return m_clientHeight;
	};

	bool CreateRTT(int, int);
	bool CreateDSBforRTT(int w, int h)
	{   
		DestroyDSBforRTT();
		return CreateDepthBuffer(w, h, &m_pDSBufferForRTT, &m_pDSVforRTT);
	};
private:
	bool CreateDepthBuffer(int, int,
		ID3D11Texture2D**,
		ID3D11DepthStencilView**);
	bool CreateDSS();

	void DestroyDSS();
	void DestroyRTT();
	void DestroyDSBforRTT();

	bool m_bVsync_enabled;
	IDXGISwapChain* m_pSwapChain;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	ID3D11RenderTargetView* m_pRenderTargetView, *m_pRTTV;
	ID3D11Texture2D* m_pDSBuffer, *m_pDSBufferForRTT, *m_pRTT;
	ID3D11ShaderResourceView* m_pSRVForRTT;
	ID3D11DepthStencilState* m_pDefualtDDS, *m_pMirroMarkDDS, *m_pDrawReflectionDDS;
	ID3D11DepthStencilView* m_pDepthStencilView, *m_pDSVforRTT;
	ID3D11RasterizerState* m_pRasterstate, *m_pRasterStateCC;
	ID3D11SamplerState* m_pSampleState;
	ID3D11BlendState* m_pNoRenderTargetWritesBS, * m_pTransparentBS;

	CImporter* m_pCImporter;
	std::vector<CModel*> m_ppCModels;
	CTerrain* m_pCTerrain;
	Camera* m_pCCam;
	Frustum* m_pCFrustum;
	MatBuffer* m_pCMBuffer;
	CLight* m_pCLight;
	CShader* m_pCShader;

	int m_clientWidth;
	int m_clientHeight;
};
#endif // !_D3D11_H_
