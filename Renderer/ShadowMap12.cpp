#include "HeapManager.h"
#include "D3D12.h"
#include "ResourceBarrierHandler.h"
#include "ShadowMap12.h"

namespace wilson
{
	ShadowMap12::ShadowMap12(ComPtr<ID3D12Device> pDevice,
		const UINT width, const UINT height, const UINT cascadeLevel, 
		const UINT dirLightCap, const UINT cubeLightCap, const UINT spotLightCap)
	{

		m_dirTexes.resize(dirLightCap);
		m_dirDebugTexes.resize(dirLightCap);

		m_cubeTexes.resize(cubeLightCap);
		m_cubeDebugTexes.resize(cubeLightCap);

		m_spotTexes.resize(spotLightCap);
		m_spotDebugTexes.resize(spotLightCap);

	
		//CubeMap==6 
		m_debugTexes.resize(max(cascadeLevel, _CUBE_FACES));

		m_rect.right = width;
		m_rect.bottom = height;

		m_width = width;
		m_height = height;

		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = static_cast<float>(width);
		m_viewport.Height = static_cast<float>(height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		{
			//Gen Tex
			{
				//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
				for (int i = 0; i < m_dirTexes.size(); ++i)
				{
					m_dirTexes[i] = g_pTextureManager->CreateTexture2D(width, height, max(cascadeLevel, _CUBE_FACES), 1, (BYTE)0b0110,
						DXGI_FORMAT_R32_TYPELESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

				}

				//Gen CubeShadowMap
				{
					for (int i = 0; i < m_cubeTexes.size(); ++i)
					{
						m_cubeTexes[i] = g_pTextureManager->CreateTexture2D(width, height, max(cascadeLevel, _CUBE_FACES), 1, (BYTE)0b100110,
							DXGI_FORMAT_R32_TYPELESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					}
				}

				//Gen SpotShadowMap
				{
					for (int i = 0; i < m_spotTexes.size(); ++i)
					{
						m_spotTexes[i] = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b0110,
							DXGI_FORMAT_R32_TYPELESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					}
				}


			}
		}

		{	
			//Gen DebugTex
			{
			
				for (int i = 0; i < m_dirDebugTexes.size(); ++i)
				{
					m_dirDebugTexes[i] = g_pTextureManager->CreateTexture2D(width, height, max(cascadeLevel, _CUBE_FACES), 1,(BYTE)0b1010,
						DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				//Gen CubeShadowMap
				{
					for (int i = 0; i < m_cubeDebugTexes.size(); ++i)
					{
						m_cubeDebugTexes[i] = g_pTextureManager->CreateTexture2D(width, height, max(cascadeLevel, _CUBE_FACES), 1, (BYTE)0b1010,
							DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					}
				}

				//Gen SpotShadowMap
				{
					for (int i = 0; i < m_spotDebugTexes.size(); ++i)
					{
						m_spotDebugTexes[i] = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b1010,
							DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					}
				}

				//Gen DebugTex
				{
					for (int i = 0; i < m_debugTexes.size(); ++i)
					{
						m_debugTexes[i] = g_pTextureManager->CreateTexture2D(width, height, 1, 1, (BYTE)0b0010,
							DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					}
				}

			}
		}

	}


	void ShadowMap12::ClearDsv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT nLights[])
	{
		for (int i = 0; i < nLights[E_TO_UINT(eLIGHT_TYPE::DIR)]; ++i)
		{
			pCmdList->ClearDepthStencilView(m_dirTexes[i]->dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}

		for (int i = 0; i < nLights[E_TO_UINT(eLIGHT_TYPE::CUBE)]; ++i)
		{
			pCmdList->ClearDepthStencilView(m_cubeTexes[i]->dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}

		for (int i = 0; i < nLights[E_TO_UINT(eLIGHT_TYPE::SPT)]; ++i)
		{
			pCmdList->ClearDepthStencilView(m_spotTexes[i]->dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}
	}
	void ShadowMap12::ClearRtv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT nLights[])
	{	

		for (int i = 0; i < nLights[E_TO_UINT(eLIGHT_TYPE::DIR)]; ++i)
		{
			pCmdList->ClearRenderTargetView(m_dirDebugTexes[i]->rtv, m_clear, 0, nullptr);
		}

		for (int i = 0; i < nLights[E_TO_UINT(eLIGHT_TYPE::CUBE)]; ++i)
		{
			pCmdList->ClearRenderTargetView(m_cubeDebugTexes[i]->rtv, m_clear, 0, nullptr);
		}

		for (int i = 0; i < nLights[E_TO_UINT(eLIGHT_TYPE::SPT)]; ++i)
		{
			pCmdList->ClearRenderTargetView(m_spotDebugTexes[i]->rtv, m_clear, 0, nullptr);
		}
	}

	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetDirDebugSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT i, const UINT lod)
	{
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_dirDebugTexes[i]->tex,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_debugTexes[lod]->tex,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Flush(pCmdList);



		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_dirDebugTexes[i]->tex.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = lod;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debugTexes[lod]->tex.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCmdList->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);

		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_dirDebugTexes[i]->tex,
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_debugTexes[lod]->tex,
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,  D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Flush(pCmdList);

		return &m_debugTexes[lod]->srv;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetCubeDebugSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT i, const UINT face)
	{
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_cubeDebugTexes[i]->tex,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_debugTexes[face]->tex,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Flush(pCmdList);

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_cubeDebugTexes[i]->tex.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = face;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debugTexes[face]->tex.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCmdList->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);
		
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_cubeDebugTexes[i]->tex,
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_debugTexes[face]->tex,
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Flush(pCmdList);


		return &m_debugTexes[face]->srv;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetSpotDebugSrv(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT i)
	{
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_spotDebugTexes[i]->tex,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_debugTexes[0]->tex,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Flush(pCmdList);


		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_spotDebugTexes[i]->tex.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debugTexes[0]->tex.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCmdList->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);

		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_spotDebugTexes[i]->tex,
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Push(
			ResourceBarrierHandler::CreateResourceBarrier(m_debugTexes[0]->tex,
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
		g_pResourceBarrierHandler->Flush(pCmdList);

		return &m_debugTexes[0]->srv;
	}
	void ShadowMap12::SetClearViewBarrier(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT litCnts[])
	{
		SetResourceBarrier(pCmdList, litCnts,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		SetResourceBarrier(pCmdList, litCnts,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, false);
	}
	void ShadowMap12::SetLightingPassBarrier(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT litCnts[])
	{
		SetResourceBarrier(pCmdList, litCnts,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

		SetResourceBarrier(pCmdList, litCnts,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, false);
	}
	void ShadowMap12::SetDirPass(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT lightIdx)
	{
		SetShadowViewport(pCmdList);
		SetShadowRect(pCmdList);

		pCmdList->OMSetRenderTargets(1, &m_dirDebugTexes[lightIdx]->rtv, TRUE, &m_dirTexes[lightIdx]->dsv);
	}
	void ShadowMap12::SetCubePass(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT lightIdx)
	{
		SetShadowViewport(pCmdList);
		SetShadowRect(pCmdList);

		pCmdList->OMSetRenderTargets(1, &m_cubeDebugTexes[lightIdx]->rtv, TRUE, &m_cubeTexes[lightIdx]->dsv);
	}
	void ShadowMap12::SetSpotPass(ComPtr<ID3D12GraphicsCommandList> pCmdList, UINT lightIdx)
	{
		SetShadowViewport(pCmdList);
		SetShadowRect(pCmdList);

		pCmdList->OMSetRenderTargets(1, &m_spotDebugTexes[lightIdx]->rtv, TRUE, &m_spotTexes[lightIdx]->dsv);
	}
	void ShadowMap12::SetLightingPass(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->SetGraphicsRootDescriptorTable(E_TO_UINT(ePbrLightRP::psDirShadow), m_dirTexes[0]->srv);
		pCmdList->SetGraphicsRootDescriptorTable(E_TO_UINT(ePbrLightRP::psCubeShadow), m_cubeTexes[0]->srv);
		pCmdList->SetGraphicsRootDescriptorTable(E_TO_UINT(ePbrLightRP::psSpotShadow), m_spotTexes[0]->srv);
	}
	void ShadowMap12::SetShadowViewport(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->RSSetViewports(1, &m_viewport);
	}
	void ShadowMap12::SetShadowRect(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		pCmdList->RSSetScissorRects(1, &m_rect);
	}

	void ShadowMap12::SetResourceBarrier(ComPtr<ID3D12GraphicsCommandList> pCmdList, const UINT nlights[],
		const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState, const bool bRTV)
	{
		//DIR, CUBE, SPOT
		int j = 0;
		if (bRTV)
		{	
			for (int i = 0; i < nlights[0]; ++i)
			{	
				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_dirDebugTexes[i]->tex, beforeState, afterState));
			}

			for (int i = 0; i < nlights[1]; ++i)
			{
				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_cubeDebugTexes[i]->tex, beforeState, afterState));
			}

			for (int i = 0; i < nlights[2]; ++i)
			{
				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_spotDebugTexes[i]->tex, beforeState, afterState));
			}
		}
		else
		{
			for (int i = 0; i < nlights[0]; ++i)
			{
				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_dirTexes[i]->tex, beforeState, afterState));
			}

			for (int i = 0; i < nlights[1]; ++i)
			{
				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_cubeTexes[i]->tex, beforeState, afterState));
			}

			for (int i = 0; i < nlights[2]; ++i)
			{
				g_pResourceBarrierHandler->Push(
					ResourceBarrierHandler::CreateResourceBarrier(m_spotTexes[i]->tex, beforeState, afterState));
			}
		}
		g_pResourceBarrierHandler->Flush(pCmdList);
	}

}