#include "BackBuffer.h"
#include "D3D12.h"
#include "TextureManager.h"
#include "ResourceBarrierHandler.h"
#include "../ImGui/imgui.h"
namespace wilson
{
	void BackBuffer::Init(ComPtr<IDXGISwapChain> pSwapChain, UINT width, UINT height)
	{
		m_curFrame = 0;
		//Gen Rtvs;
		{
			ComPtr<ID3D12Device>  pDevice = D3D12::GetDevice();
			for (UINT i = 0; i < _BUFFER_COUNT; ++i)
			{
				m_pScreenTexs[i] = g_pTextureManager->CreateTexture2D(pSwapChain,i);
				SET_PRIVATE_DATA(m_pScreenTexs[i]->tex);
			}
		}
		{
			m_screenViewport.Width = static_cast<float>(width);
			m_screenViewport.Height = static_cast<float>(height);
			m_screenViewport.MinDepth = 0.0f;
			m_screenViewport.MaxDepth = 1.0f;
			m_screenViewport.TopLeftX = 0.0f;
			m_screenViewport.TopLeftY = 0.0f;

			m_screenRect.left = 0;
			m_screenRect.top = 0;
			m_screenRect.right = m_screenViewport.Width;
			m_screenRect.bottom = m_screenViewport.Height;
		}
		m_pSceneTex = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b1010,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		SET_PRIVATE_DATA(m_pSceneTex->tex);

		m_pViewportTex = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b0011,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		SET_PRIVATE_DATA(m_pViewportTex->tex);

		for (int i = 0; i < E_TO_UINT(eGbuf::cnt); ++i)
		{
			if (i <E_TO_UINT(eGbuf::albedo) || i >E_TO_UINT(eGbuf::emissive))
			{
				m_pGBufTexs[i] = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b1010,
					DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
			else
			{
				m_pGBufTexs[i] = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b1010,
					DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			}
			SET_PRIVATE_DATA(m_pGBufTexs[i]->tex);

			m_GbufRtvs[i] = m_pGBufTexs[i]->rtv;
			m_GbufSrvs[i] = m_pGBufTexs[i]->srv;
		}


		//Create DepthDebugTex
		m_pDepthDebugTex = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b1000,
			DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		SET_PRIVATE_DATA(m_pDepthDebugTex->tex);

		m_pScreenDepthTex = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b0100,
			DXGI_FORMAT_R32_TYPELESS, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		SET_PRIVATE_DATA(m_pScreenDepthTex->tex);

		m_pSceneDepthTex = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b0110,
			DXGI_FORMAT_R32_TYPELESS, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		SET_PRIVATE_DATA(m_pSceneDepthTex->tex);
	}
	void BackBuffer::Shutdown()
	{
		if (m_pViewportTex != nullptr)
		{
			m_pViewportTex = nullptr;
		}

		if (m_pSceneTex != nullptr)
		{
			m_pSceneTex = nullptr;
		}

		if (m_pSceneDepthTex != nullptr)
		{
			m_pSceneDepthTex = nullptr;
		}

		for (int i = 0; i < E_TO_UINT(eGbuf::cnt); ++i)
		{
			if (m_pGBufTexs[i] != nullptr)
			{
				m_pGBufTexs[i] = nullptr;
			}

		}

		for (int i = 0; i < _BUFFER_COUNT; ++i)
		{
			if (m_pScreenTexs[i] != nullptr)
			{
				m_pScreenTexs[i] = nullptr;
			}
		}

		if (m_pScreenDepthTex != nullptr)
		{
			m_pScreenDepthTex = nullptr;
		}
	}
	void BackBuffer::IncreaseFrameCount()
	{
		m_curFrame = ++m_curFrame % _BUFFER_COUNT;
	}
	void BackBuffer::ClearBackBufferView(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		FLOAT color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		pCmdList->ClearRenderTargetView(m_pScreenTexs[m_curFrame]->rtv, color, 0, nullptr);
		pCmdList->ClearRenderTargetView(m_pSceneTex->rtv, color, 0, nullptr);

		for (int i = 0; i < E_TO_UINT(eGbuf::cnt); ++i)
		{
			pCmdList->ClearRenderTargetView(m_pGBufTexs[i]->rtv, color, 0, nullptr);
		}

		pCmdList->ClearDepthStencilView(m_pScreenDepthTex->dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		pCmdList->ClearDepthStencilView(m_pSceneDepthTex->dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
	}
	void BackBuffer::SetScreenViewport(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->RSSetViewports(1, &m_screenViewport);
	}

	void BackBuffer::SetScreenRect(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->RSSetScissorRects(1, &m_screenRect);
	}


	void BackBuffer::SetScreenRenderTargets(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->OMSetRenderTargets(1, &m_pScreenTexs[m_curFrame]->rtv, FALSE, &m_pScreenDepthTex->dsv);
	}

	void BackBuffer::SetZPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		SetScreenViewport(pCmdList);
		SetScreenRect(pCmdList);

		pCmdList->OMSetRenderTargets(1, &m_pDepthDebugTex->rtv, FALSE, &m_pSceneDepthTex->dsv);
	}

	void BackBuffer::SetGeoPass(ComPtr<ID3D12GraphicsCommandList>  pCmdList)
	{
		SetScreenViewport(pCmdList);
		SetScreenRect(pCmdList);

		pCmdList->OMSetRenderTargets(E_TO_UINT(eGbuf::cnt), m_GbufRtvs, FALSE, &m_pSceneDepthTex->dsv);
	}

	void BackBuffer::SetLightingPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		for (int i = 0; i < E_TO_UINT(eGbuf::vPos); ++i)
		{
			g_pTextureManager->UploadGraphicsSrv(pCmdList, m_pGBufTexs[i], i);
		}

		SetScreenViewport(pCmdList);
		SetScreenRect(pCmdList);

		pCmdList->OMSetRenderTargets(1, &m_pSceneTex->rtv, FALSE, &m_pSceneDepthTex->dsv);
	}

	void BackBuffer::SetSkyboxPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->OMSetRenderTargets(1, &m_pSceneTex->rtv, FALSE, &m_pSceneDepthTex->dsv);
	}

	void BackBuffer::SetHWCullPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		SetScreenViewport(pCmdList);
		SetScreenRect(pCmdList);

		pCmdList->OMSetRenderTargets(1, &m_pSceneTex->rtv, FALSE, &m_pSceneDepthTex->dsv);
	}

	void BackBuffer::SetBeginSceneBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pScreenTexs[m_curFrame], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pViewportTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSceneTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

		for (int i = 0; i < E_TO_UINT(eGbuf::cnt); ++i)
		{
			g_pResourceBarrierHandler->Push(
				g_pTextureManager->CreateBarrier(m_pGBufTexs[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		}

	}

	void BackBuffer::SetBeginDownSampleBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSceneDepthTex, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	}

	void BackBuffer::SetEndDownSampleBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSceneDepthTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	}

	void BackBuffer::SetGeoPassBarriers()
	{
		for (int i = 0; i < E_TO_UINT(eGbuf::cnt); ++i)
		{
			if (i > E_TO_UINT(eGbuf::emissive))
			{
				g_pResourceBarrierHandler->Push(
					g_pTextureManager->CreateBarrier(m_pGBufTexs[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
			}
			else
			{
				g_pResourceBarrierHandler->Push(
					g_pTextureManager->CreateBarrier(m_pGBufTexs[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			}
		}
	}

	void BackBuffer::SetLightPassBarriers()
	{
		g_pResourceBarrierHandler->Push(
				g_pTextureManager->CreateBarrier(m_pGBufTexs[E_TO_UINT(eGbuf::vNormal)], 
					D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pGBufTexs[E_TO_UINT(eGbuf::vPos)],
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		
	}

	void BackBuffer::SetPostProcessBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSceneTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	}

	void BackBuffer::SetUIPassBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pSceneTex, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pViewportTex, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}

	void BackBuffer::SetPresentBarriers()
	{
		g_pResourceBarrierHandler->Push(
			g_pTextureManager->CreateBarrier(m_pScreenTexs[m_curFrame], 
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	}

	void BackBuffer::DrawGBufImage()
	{
		for (int i = 0; i < E_TO_UINT(eGbuf::cnt); ++i)
		{
			g_pTextureManager->SetImGuiImage(m_pGBufTexs[i], m_screenViewport.Width, m_screenViewport.Height);
		}
		g_pTextureManager->SetImGuiImage(m_pSceneDepthTex, m_screenViewport.Width, m_screenViewport.Height);
	}

	void BackBuffer::DrawViewportImage()
	{
		g_pTextureManager->SetImGuiImage(m_pViewportTex, m_screenViewport.Width, m_screenViewport.Height);
	}

	void BackBuffer::UploadDownSampleTexture(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		g_pTextureManager->UploadComputeSrv(pCmdList, m_pSceneDepthTex, E_TO_UINT(eDownSampleRP::depthMap));
	}

	void BackBuffer::SetSSAOPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		for (int i = E_TO_UINT(eGbuf::vPos),j = E_TO_UINT(eSsaoRP::csVpos); 
			i < E_TO_UINT(eGbuf::cnt); ++i,++j)
		{
			g_pTextureManager->UploadComputeSrv(pCmdList, m_pGBufTexs[i], j);
		}
	}

	void BackBuffer::UploadPostProcessTexture(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		g_pTextureManager->UploadComputeSrv(pCmdList, m_pSceneTex, E_TO_UINT(ePostProcessRP::csTex));
		g_pTextureManager->UploadComputeUav(pCmdList, m_pViewportTex, E_TO_UINT(ePostProcessRP::csUav));
	}

	BackBuffer::BackBuffer(ComPtr<IDXGISwapChain> pSwapChain, UINT width, UINT height)
	{
		Init(pSwapChain, width, height);
	}
	BackBuffer::~BackBuffer()
	{
		Shutdown();
	}
}
