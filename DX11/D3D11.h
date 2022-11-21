#ifndef _D3D11_H_
#define _D3D11_H_

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx10math.h>
#include <Windows.h>
//D:\DirectxSDK\Include;$(IncludePath)  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include;$(IncludePath)
//D:\DirectxSDK\Lib\x64;$(LibraryPath)  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x64;$(LibraryPath)
class CD3D11
{
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 color;
	};

	struct ConstantBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	float mTheta, mPhi, mRadius;
public:
	CD3D11();
	CD3D11(const CD3D11&) = delete;
	~CD3D11();

	bool Init(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void UpdateScene();
	void DrawScene();
	float dx, dy, dz = 0;
private:
	long long currtime, frequency;
	D3DXVECTOR3 m_vScale = { 1.0f, 1.0f,1.0f };
	float ds = 0.001f;

	bool m_bVsync_enabled;
	IDXGISwapChain* m_pSwapChain;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilState* m_pDepthStencilState;
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11RasterizerState* m_pRasterstate;
	D3DXMATRIX m_projectionMatrix;
	D3DXMATRIX m_worldMatrix, m_worldMatrix2;
	D3DXMATRIX m_orthoMatrix;
	D3DXMATRIX m_viewMatrix;
	D3DXMATRIX m_rotationMatrix;

	ID3D11Buffer* m_pVertexBuffer, * m_pIndexBuffer;
	int m_vertexCount, m_indexCount;

	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11InputLayout* m_pInputLayout;
	ID3D11Buffer* m_pConstantBuffer, * m_pConstantBuffer2;
};
#endif // !_D3D11_H_
