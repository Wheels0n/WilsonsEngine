#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include "typedef.h"
namespace wilson
{
	using namespace Microsoft::WRL;
	using namespace std;
	using namespace DirectX;

	class TextureWrapper;
	class BackBuffer
	{
	private:

		void SetScreenViewport(ComPtr<ID3D12GraphicsCommandList>);
		void SetScreenRect(ComPtr<ID3D12GraphicsCommandList>);

	public:
		inline shared_ptr<TextureWrapper> GetSceneDepthTex()
		{
			return m_pSceneDepthTex;
		};
		inline D3D12_GPU_DESCRIPTOR_HANDLE* GetpGbufferSrvs()
		{
			return m_GbufSrvs;
		};
		inline shared_ptr<TextureWrapper> GetViewportTex()
		{
			return m_pViewportTex;
		};

		void Init(ComPtr<IDXGISwapChain> pSwapChain, UINT width, UINT height);
		void Shutdown();

		void IncreaseFrameCount();
		void ClearBackBufferView(ComPtr<ID3D12GraphicsCommandList>);
		
		void DrawGBufImage();
		void DrawViewportImage();

		void SetScreenRenderTargets(ComPtr<ID3D12GraphicsCommandList>);
		
		void SetZPass(ComPtr<ID3D12GraphicsCommandList>);
		void SetHWCullPass(ComPtr<ID3D12GraphicsCommandList>);
		void SetGeoPass(ComPtr<ID3D12GraphicsCommandList>);
		void SetSSAOPass(ComPtr<ID3D12GraphicsCommandList>);
		void SetLightingPass(ComPtr<ID3D12GraphicsCommandList>);
		void SetSkyboxPass(ComPtr<ID3D12GraphicsCommandList>);

		void SetBeginSceneBarriers();
		void SetBeginDownSampleBarriers();
		void SetEndDownSampleBarriers();
		void SetGeoPassBarriers();
		void SetLightPassBarriers();
		void SetPostProcessBarriers();
		void SetUIPassBarriers();
		void SetPresentBarriers();

		void UploadDownSampleTexture(ComPtr<ID3D12GraphicsCommandList>);
		void UploadPostProcessTexture(ComPtr<ID3D12GraphicsCommandList>);

		BackBuffer(ComPtr<IDXGISwapChain> pSwapChain, UINT width, UINT height);
		~BackBuffer();
	private:
		UINT m_curFrame;

		shared_ptr<TextureWrapper>		m_pScreenTexs[_BUFFER_COUNT];
		shared_ptr<TextureWrapper>		m_pDepthDebugTex;
		shared_ptr<TextureWrapper>		m_pGBufTexs[E_TO_UINT(eGbuf::cnt)];

		shared_ptr<TextureWrapper>		m_pSceneTex;
		shared_ptr<TextureWrapper>		m_pSceneDepthTex;
		shared_ptr<TextureWrapper>		m_pScreenDepthTex;
		shared_ptr<TextureWrapper>		m_pViewportTex;

		D3D12_RECT				m_screenRect;
		D3D12_VIEWPORT			m_screenViewport;

		D3D12_CPU_DESCRIPTOR_HANDLE m_GbufRtvs[E_TO_UINT(eGbuf::cnt)];
		D3D12_GPU_DESCRIPTOR_HANDLE m_GbufSrvs[E_TO_UINT(eGbuf::cnt)];
	};
}

