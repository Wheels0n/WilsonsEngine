#include "ShadowMap12.h"
#include "HeapManager.h"
#include "D3D12.h"
namespace wilson
{
	ShadowMap12::ShadowMap12(ID3D12Device* const pDevice, ID3D12GraphicsCommandList* const pCommandList,
		HeapManager* const pHeapManager,
		const UINT width, const UINT height, const UINT cascadeLevel, 
		const UINT dirLightCap, const UINT cubeLightCap, const UINT spotLightCap)
	{

		m_dirTexes.resize(dirLightCap);
		m_dirDebugTexes.resize(dirLightCap);
		m_dirDebugRtvs.resize(dirLightCap);

		m_cubeTexes.resize(cubeLightCap);
		m_cubeDebugTexes.resize(cubeLightCap);
		m_cubeDebugRtvs.resize(cubeLightCap);

		m_spotTexes.resize(spotLightCap);
		m_spotDebugTexes.resize(spotLightCap);
		m_spotDebugRtvs.resize(spotLightCap);

		m_dirSrvs.resize(dirLightCap);
		m_dirDebugSrvs.resize(dirLightCap);

		m_cubeSrvs.resize(cubeLightCap);
		m_cubeDebugSrvs.resize(cubeLightCap);

		m_spotSrvs.resize(spotLightCap);
		m_spotDebugSrvs.resize(spotLightCap);
		//CubeMap==6 
		m_debugTexes.resize(max(cascadeLevel,6));
		m_debugSrvs.resize(max(cascadeLevel, 6));

		m_dirDsvs.resize(dirLightCap);
		m_cubeDsvs.resize(cubeLightCap);
		m_spotDsvs.resize(spotLightCap);

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
				D3D12_RESOURCE_DESC	texDesc = {};
				texDesc.Width = width;
				texDesc.Height = height;
				texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				texDesc.Alignment = 0;
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				texDesc.DepthOrArraySize = max(cascadeLevel, 6);
				texDesc.MipLevels = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;

				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
				srvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				//D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
				for (int i = 0; i < m_dirTexes.size(); ++i)
				{
					pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, m_dirTexes[i].GetAddressOf(), pDevice);
					m_dirTexes[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap:::m_dirTexes[i]") - 1, "ShadowMap:::m_dirTexes[i]");
					m_dirDsvs[i] = pHeapManager->GetDsv(dsvDesc, m_dirTexes[i].Get(), pDevice);
					m_dirSrvs[i] = pHeapManager->GetSrv(srvDesc, m_dirTexes[i].Get(), pDevice);

				}

				//Gen CubeShadowMap
				{
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					for (int i = 0; i < m_cubeTexes.size(); ++i)
					{
						pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, m_cubeTexes[i].GetAddressOf(), pDevice);
						m_cubeTexes[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_cubeTexes[i]") - 1, "ShadowMap:::m_cubeTexes[i]");
						m_cubeDsvs[i] = pHeapManager->GetDsv(dsvDesc, m_cubeTexes[i].Get(), pDevice);
						m_cubeSrvs[i] = pHeapManager->GetSrv(srvDesc, m_cubeTexes[i].Get(), pDevice);
					}
				}

				//Gen SpotShadowMap
				{
					texDesc.DepthOrArraySize = 1;
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

					for (int i = 0; i < m_spotTexes.size(); ++i)
					{
						pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, m_spotTexes[i].GetAddressOf(), pDevice);
						m_spotTexes[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_spotTexes[i]") - 1, "ShadowMap:::m_spotTexes[i]");
						m_spotDsvs[i] = pHeapManager->GetDsv(dsvDesc, m_spotTexes[i].Get(), pDevice);
						m_spotSrvs[i] = pHeapManager->GetSrv(srvDesc, m_spotTexes[i].Get(), pDevice);
					}
				}


			}
		}

		{	
			//Gen DebugTex
			{
				D3D12_RESOURCE_DESC	texDesc = {};
				texDesc.Width = width;
				texDesc.Height = height;
				texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				texDesc.Alignment = 0;
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				texDesc.DepthOrArraySize = max(cascadeLevel,6);
				texDesc.MipLevels = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;

				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.Format = texDesc.Format;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				rtvDesc.Texture2DArray.MipSlice = 0;

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Format = texDesc.Format;
				srvDesc.Texture2DArray.ArraySize = texDesc.DepthOrArraySize;
				srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


				for (int i = 0; i < m_dirDebugTexes.size(); ++i)
				{
					pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, m_dirDebugTexes[i].GetAddressOf(), pDevice);
					m_dirDebugTexes[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
						sizeof("ShadowMap:::m_dirDebugTexes[i]") - 1, "ShadowMap:::m_dirDebugTexes[i]");
					m_dirDebugRtvs[i] = pHeapManager->GetRtv(rtvDesc, m_dirDebugTexes[i].Get(), pDevice);
					m_dirDebugSrvs[i] = pHeapManager->GetSrv(srvDesc, m_dirDebugTexes[i].Get(), pDevice);
				}

				//Gen CubeShadowMap
				{
					for (int i = 0; i < m_cubeDebugTexes.size(); ++i)
					{
						pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, m_cubeDebugTexes[i].GetAddressOf(), pDevice);
						m_cubeDebugTexes[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_cubeDebugTexes[i]") - 1, "ShadowMap:::m_cubeDebugTexes[i]");
						m_cubeDebugRtvs[i] = pHeapManager->GetRtv(rtvDesc, m_cubeDebugTexes[i].Get(), pDevice);
						m_cubeDebugSrvs[i] = pHeapManager->GetSrv(srvDesc, m_cubeDebugTexes[i].Get(), pDevice);
					}
				}

				//Gen SpotShadowMap
				{
					texDesc.DepthOrArraySize = 1;
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

					for (int i = 0; i < m_spotDebugTexes.size(); ++i)
					{
						pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, m_spotDebugTexes[i].GetAddressOf(), pDevice);
						m_spotDebugTexes[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_spotDebugTexes[i]") - 1, "ShadowMap:::m_spotDebugTexes[i]");
						m_spotDebugRtvs[i] = pHeapManager->GetRtv(rtvDesc, m_spotDebugTexes[i].Get(), pDevice);
						m_spotDebugSrvs[i] = pHeapManager->GetSrv(srvDesc, m_spotDebugTexes[i].Get(), pDevice);
					}
				}

				//Gen DebugTex
				{
					for (int i = 0; i < m_debugTexes.size(); ++i)
					{
						pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, m_debugTexes[i].GetAddressOf(), pDevice);
						m_debugTexes[i]->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("ShadowMap:::m_debugTexes[i]") - 1, "ShadowMap:::m_debugTexes[i]");
						m_debugSrvs[i] = pHeapManager->GetSrv(srvDesc, m_debugTexes[i].Get(), pDevice);

					}
				}

			}
		}
		//Gen Samplers
		{

			D3D12_SAMPLER_DESC ssDesc = {};
			ssDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			ssDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			ssDesc.BorderColor[0] = 1.0f;
			ssDesc.BorderColor[1] = 1.0f;
			ssDesc.BorderColor[2] = 1.0f;
			ssDesc.BorderColor[3] = 1.0f;
			ssDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;

			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = pHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = pHeapManager->GetCurSamplerGpuHandle();

				pDevice->CreateSampler(&ssDesc, samplerCpuHandle);
				m_dirShadowSsv = samplerGpuHandle;
				pHeapManager->IncreaseSamplerHandleOffset();
			}

			{
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = pHeapManager->GetCurSamplerCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = pHeapManager->GetCurSamplerGpuHandle();
				pDevice->CreateSampler(&ssDesc, samplerCpuHandle);
				m_cubeShadowSsv = samplerGpuHandle;
				pHeapManager->IncreaseSamplerHandleOffset();
			}
			
		}

	}

	void ShadowMap12::BindDirDsv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i)
	{	
		pCommandlist->OMSetRenderTargets(1, &m_dirDebugRtvs[i], TRUE, &m_dirDsvs[i]);
	}

	void ShadowMap12::BindCubeDsv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i)
	{
		pCommandlist->OMSetRenderTargets(1, &m_cubeDebugRtvs[i], TRUE, &m_cubeDsvs[i]);
	}

	void ShadowMap12::BindSpotDsv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i)
	{
		pCommandlist->OMSetRenderTargets(1, &m_spotDebugRtvs[i], TRUE, &m_spotDsvs[i]);
	}

	void ShadowMap12::ClearDsv(ID3D12GraphicsCommandList* const pCommandlist, const UINT nLights[])
	{
		for (int i = 0; i < nLights[0]; ++i)
		{
			pCommandlist->ClearDepthStencilView(m_dirDsvs[i], D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}

		for (int i = 0; i < nLights[1]; ++i)
		{
			pCommandlist->ClearDepthStencilView(m_cubeDsvs[i], D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}

		for (int i = 0; i < nLights[2]; ++i)
		{
			pCommandlist->ClearDepthStencilView(m_spotDsvs[i], D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0.0f, 0, nullptr);
		}
	}

	void ShadowMap12::ClearRtv(ID3D12GraphicsCommandList* const pCommandlist, const UINT nLights[])
	{	

		for (int i = 0; i < nLights[0]; ++i)
		{
			pCommandlist->ClearRenderTargetView(m_dirDebugRtvs[i], m_clear, 0, nullptr);
		}

		for (int i = 0; i < nLights[1]; ++i)
		{
			pCommandlist->ClearRenderTargetView(m_cubeDebugRtvs[i], m_clear, 0, nullptr);
		}

		for (int i = 0; i < nLights[2]; ++i)
		{
			pCommandlist->ClearRenderTargetView(m_spotDebugRtvs[i], m_clear, 0, nullptr);
		}
	}

	void ShadowMap12::SetResourceBarrier(ID3D12GraphicsCommandList* const pCommandlist, const UINT nlights[],
		const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState, const bool bRTV)
	{
		UINT litCntSum = nlights[0] + nlights[1] + nlights[2];
		std::vector<D3D12_RESOURCE_BARRIER> barriers(litCntSum);
		int j = 0;
		if (bRTV)
		{	
			for (int i = 0; i < nlights[0]; ++i)
			{	
				barriers[j++] = D3D12::CreateResourceBarrier(m_dirDebugTexes[i].Get(), beforeState, afterState);
			}

			for (int i = 0; i < nlights[1]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_cubeDebugTexes[i].Get(), beforeState, afterState);
			}

			for (int i = 0; i < nlights[2]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_spotDebugTexes[i].Get(), beforeState, afterState);
			}
		}
		else
		{
			for (int i = 0; i < nlights[0]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_dirTexes[i].Get(), beforeState, afterState);
			}

			for (int i = 0; i < nlights[1]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_cubeTexes[i].Get(), beforeState, afterState);
			}

			for (int i = 0; i < nlights[2]; ++i)
			{
				barriers[j++] = D3D12::CreateResourceBarrier(m_spotTexes[i].Get(), beforeState, afterState);
			}
		}

		pCommandlist->ResourceBarrier(litCntSum, barriers.data());
	}

	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetDirDebugSrv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i, const UINT lod)
	{	
		D3D12_RESOURCE_BARRIER srvToCopySrc = {};
		srvToCopySrc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopySrc.Transition.pResource = m_dirDebugTexes[i].Get();
		srvToCopySrc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopySrc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopySrc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		D3D12_RESOURCE_BARRIER srvToCopyDst = {};
		srvToCopyDst.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopyDst.Transition.pResource = m_debugTexes[lod].Get();
		srvToCopyDst.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopyDst.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopyDst.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_RESOURCE_BARRIER barriers[2];
		barriers[0] = srvToCopyDst;
		barriers[1] = srvToCopySrc;
		pCommandlist->ResourceBarrier(2, barriers);

		D3D12_RESOURCE_BARRIER copySrcToSrv = {};
		copySrcToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copySrcToSrv.Transition.pResource = m_dirDebugTexes[i].Get();
		copySrcToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copySrcToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		copySrcToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_RESOURCE_BARRIER copyDstToSrv = {};
		copyDstToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copyDstToSrv.Transition.pResource = m_debugTexes[lod].Get();
		copyDstToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copyDstToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		copyDstToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_dirDebugTexes[i].Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = lod;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debugTexes[lod].Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCommandlist->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);

		barriers[0] = copyDstToSrv;
		barriers[1] = copySrcToSrv;
		pCommandlist->ResourceBarrier(2, barriers);
		return &m_debugSrvs[lod];
	}

	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetCubeDebugSrv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i, const UINT face)
	{
		D3D12_RESOURCE_BARRIER srvToCopySrc = {};
		srvToCopySrc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopySrc.Transition.pResource = m_cubeDebugTexes[i].Get();
		srvToCopySrc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopySrc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopySrc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		D3D12_RESOURCE_BARRIER srvToCopyDst = {};
		srvToCopyDst.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopyDst.Transition.pResource = m_debugTexes[face].Get();
		srvToCopyDst.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopyDst.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopyDst.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_RESOURCE_BARRIER barriers[2];
		barriers[0] = srvToCopyDst;
		barriers[1] = srvToCopySrc;
		pCommandlist->ResourceBarrier(2, barriers);

		D3D12_RESOURCE_BARRIER copySrcToSrv = {};
		copySrcToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copySrcToSrv.Transition.pResource = m_cubeDebugTexes[i].Get();
		copySrcToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copySrcToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		copySrcToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_RESOURCE_BARRIER copyDstToSrv = {};
		copyDstToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copyDstToSrv.Transition.pResource = m_debugTexes[face].Get();
		copyDstToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copyDstToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		copyDstToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_cubeDebugTexes[i].Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = face;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debugTexes[face].Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCommandlist->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);
		barriers[0] = copyDstToSrv;
		barriers[1] = copySrcToSrv;
		pCommandlist->ResourceBarrier(2, barriers);

		return &m_debugSrvs[face];
	}

	D3D12_GPU_DESCRIPTOR_HANDLE* ShadowMap12::GetSpotDebugSrv(ID3D12GraphicsCommandList* const pCommandlist, const UINT i)
	{
		D3D12_RESOURCE_BARRIER srvToCopySrc = {};
		srvToCopySrc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopySrc.Transition.pResource = m_spotDebugTexes[i].Get();
		srvToCopySrc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopySrc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopySrc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		D3D12_RESOURCE_BARRIER srvToCopyDst = {};
		srvToCopyDst.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		srvToCopyDst.Transition.pResource = m_debugTexes[0].Get();
		srvToCopyDst.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		srvToCopyDst.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		srvToCopyDst.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_RESOURCE_BARRIER barriers[2];
		barriers[0] = srvToCopyDst;
		barriers[1] = srvToCopySrc;
		pCommandlist->ResourceBarrier(2, barriers);

		D3D12_RESOURCE_BARRIER copySrcToSrv = {};
		copySrcToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copySrcToSrv.Transition.pResource = m_spotDebugTexes[i].Get();
		copySrcToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copySrcToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		copySrcToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_RESOURCE_BARRIER copyDstToSrv = {};
		copyDstToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		copyDstToSrv.Transition.pResource = m_debugTexes[0].Get();
		copyDstToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		copyDstToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		copyDstToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_spotDebugTexes[i].Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_debugTexes[0].Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		pCommandlist->CopyTextureRegion(&dst, 0, 0, 0,
			&src, NULL);

		barriers[0] = copyDstToSrv;
		barriers[1] = copySrcToSrv;
		pCommandlist->ResourceBarrier(2, barriers);

		return &m_debugSrvs[0];
	}


	ShadowMap12::~ShadowMap12()
	{
	}

}